#!/usr/bin/env python3
"""
gen_formula_offsets.py — Parse external formula .inc files and generate
C++ FormulaParamMap offset tables for the mb3d external formula system.

Usage:
    python3 scripts/gen_formula_offsets.py path/to/formula.inc
    python3 scripts/gen_formula_offsets.py path/to/Formulas/          # all .inc in tree

For each .inc file, extracts:
  1. Formula ID and name from: constexpr unsigned int FORMULA_* = ID;
  2. PVar byte offsets from: [esi-0xNN] or [PVar-0xNN] comments
  3. DEoption from: DEoption=N in comments or .DEoption field in metadata
  4. param_count from: constant FormulaInfo .param_count field

Output: C++ FormulaParamMap declarations to stdout.
"""

import re
import sys
import os
from pathlib import Path


def parse_inc_file(filepath):
    """Parse a single .inc file and return extracted formula info."""
    with open(filepath, 'r', encoding='utf-8', errors='replace') as f:
        content = f.read()
    lines = content.splitlines()

    result = {
        'filepath': str(filepath),
        'formula_id': None,
        'formula_name': None,
        'formula_const_name': None,
        'param_count': None,
        'de_option': None,
        'pvar_offsets': {},  # param_index -> (byte_offset, value_type)
        'warnings': [],
    }

    # 1. Extract formula ID and const name
    #    Pattern: constexpr unsigned int FORMULA_NAME = 12345;
    m = re.search(r'constexpr\s+unsigned\s+int\s+(FORMULA_\w+)\s*=\s*(\d+)\s*;', content)
    if m:
        result['formula_const_name'] = m.group(1)
        result['formula_id'] = int(m.group(2))
    else:
        result['warnings'].append('No FORMULA_* constant found')
        return result

    # 2. Extract formula display name from FormulaInfo .name field
    #    Pattern: .name = "SomeName",
    m_name = re.search(r'\.name\s*=\s*"([^"]+)"', content)
    if m_name:
        result['formula_name'] = m_name.group(1)
    else:
        # Derive from const name: FORMULA_AMAZINGBOXSSE2 -> AmazingBoxSSE2
        raw = result['formula_const_name'].replace('FORMULA_', '').replace('FORMULA__', '_')
        result['formula_name'] = raw

    # 3. Extract param_count from FormulaInfo
    #    Pattern: .param_count = N
    m_pc = re.search(r'\.param_count\s*=\s*(\d+)', content)
    if m_pc:
        result['param_count'] = int(m_pc.group(1))

    # 4. Extract DEoption
    #    From comment: DEoption=N or DEoption: N or .DEoption = N
    for pattern in [r'DEoption\s*[=:]\s*(-?\d+)', r'\.DEoption\s*=\s*(-?\d+)']:
        m_de = re.search(pattern, content)
        if m_de:
            result['de_option'] = int(m_de.group(1))
            break

    # 5. Extract PVar offsets from comments
    #    Patterns:
    #      [esi-0xNN]  or  [PVar-0xNN]  or  [esi+0xNN]  or  [PVar+0xNN]
    #      Associated with params[N] on same or nearby line
    #
    #    Also look for PVar layout blocks:
    #      [PVar-0x40] = ...
    #      [PVar+0x00] = ...

    # Strategy A: Find params[N] with [esi/PVar offset] on the same line
    param_offset_pattern = re.compile(
        r'params\[(\d+)\].*?\[(esi|PVar|edi)\s*([+-])\s*0x([0-9a-fA-F]+)\]'
    )
    for line in lines:
        for m in param_offset_pattern.finditer(line):
            idx = int(m.group(1))
            sign = 1 if m.group(3) == '+' else -1
            offset = sign * int(m.group(4), 16)
            # Determine type from context
            vtype = 0  # default double
            if 'dword' in line.lower() or 'single' in line.lower() or 'float' in line.lower():
                vtype = 1  # float/single
            if 'integer' in line.lower() or 'int(' in line.lower():
                vtype = 2  # int32
            result['pvar_offsets'][idx] = (offset, vtype)

    # Strategy B: PVar layout block (typically in comments at end of file)
    #   [PVar-0x40] = ... (.Folding16 ...)
    #   We map these sequentially if params[N] annotations weren't found
    pvar_layout_pattern = re.compile(
        r'\[(esi|PVar|edi)\s*([+-])\s*0x([0-9a-fA-F]+)\]\s*='
    )
    pvar_layout_entries = []
    skip_keywords = ['mask', 'align', 'padding', 'reserved', 'abs mask', '$7fff',
                     'movsd', 'movupd', 'movapd', 'mulpd', 'mulsd', 'addpd', 'addsd',
                     'subpd', 'subsd', 'maxpd', 'maxsd', 'minpd', 'minsd', 'ucomisd',
                     'divsd', 'shufpd', 'pshufd', 'movhpd', 'fld', 'fstp', 'fmul',
                     'CONSTANTS']
    for line in lines:
        m = pvar_layout_pattern.search(line)
        if m:
            line_lower = line.lower()
            # Skip non-parameter entries (masks, alignment, padding)
            if any(kw in line_lower for kw in skip_keywords):
                continue
            sign = 1 if m.group(2) == '+' else -1
            offset = sign * int(m.group(3), 16)
            vtype = 0
            if 'xmmword' in line_lower:
                vtype = 0  # packed doubles, treat as double
            if 'single' in line_lower or 'dword' in line_lower:
                vtype = 1
            if 'integer' in line_lower:
                vtype = 2
            pvar_layout_entries.append((offset, vtype))

    # If Strategy A found nothing but we have PVar layout, map sequentially
    if not result['pvar_offsets'] and pvar_layout_entries:
        for idx, (offset, vtype) in enumerate(pvar_layout_entries):
            result['pvar_offsets'][idx] = (offset, vtype)

    # Strategy C: Look for matrix layout patterns
    #   m[N]=[PVar-0xNN] — single-precision matrix elements
    matrix_pattern = re.compile(
        r'm\[(\d+)\]\s*=\s*\[PVar\s*([+-])\s*0x([0-9a-fA-F]+)\]'
    )
    matrix_entries = []
    for line in lines:
        for m in matrix_pattern.finditer(line):
            idx = int(m.group(1))
            sign = 1 if m.group(2) == '+' else -1
            offset = sign * int(m.group(3), 16)
            matrix_entries.append((idx, offset))
    # For matrix-style formulas, map m[0..N] as params[0..N] with float type
    if matrix_entries and not result['pvar_offsets']:
        for idx, offset in sorted(matrix_entries):
            result['pvar_offsets'][idx] = (offset, 1)  # single-precision

    # If no param_count from metadata, infer from offsets or template args
    if result['param_count'] is None and result['pvar_offsets']:
        result['param_count'] = max(result['pvar_offsets'].keys()) + 1

    # Count params from template function signature
    if result['param_count'] is None:
        # Count params[N] usages in code
        param_indices = set()
        for m in re.finditer(r'params\[(\d+)\]', content):
            param_indices.add(int(m.group(1)))
        if param_indices:
            result['param_count'] = max(param_indices) + 1

    return result


def generate_cpp_map(info):
    """Generate C++ FormulaParamMap declaration from parsed info."""
    lines = []
    name = info['formula_const_name']
    clean_name = name.replace('FORMULA_', '').replace('FORMULA__', '_')

    if info['param_count'] is None or info['param_count'] == 0:
        lines.append(f'// {info["filepath"]}')
        lines.append(f'// WARNING: No parameters detected for {name}')
        lines.append(f'static const mb3d::FormulaParamMap map_{clean_name} = {{')
        lines.append(f'    0, // count')
        lines.append(f'    {{}}')
        lines.append(f'}};')
        return '\n'.join(lines)

    param_count = info['param_count']
    entries = []
    has_annotations = bool(info['pvar_offsets'])
    warned = False

    for i in range(param_count):
        if i in info['pvar_offsets']:
            offset, vtype = info['pvar_offsets'][i]
            entries.append((offset, vtype))
        else:
            # Generate sequential offsets: params[0] -> -0x10, params[1] -> -0x18, etc.
            # This follows the Pascal convention where PVar points past the param block,
            # and doubles are stored at decreasing negative offsets
            offset = -(0x10 + i * 0x08)
            entries.append((offset, 0))
            if not warned:
                warned = True

    lines.append(f'// {info["filepath"]}')
    if info['de_option'] is not None:
        lines.append(f'// DEoption={info["de_option"]}')
    if warned:
        lines.append(f'// WARNING: Some offsets are auto-generated sequential estimates (no [esi-0xNN] annotations)')

    lines.append(f'static const mb3d::FormulaParamMap map_{clean_name} = {{')
    lines.append(f'    {param_count}, // count')
    lines.append(f'    {{')

    for i, (offset, vtype) in enumerate(entries):
        type_names = {0: 'double', 1: 'float', 2: 'int32'}
        type_str = type_names.get(vtype, 'double')
        sign = '+' if offset >= 0 else '-'
        abs_off = abs(offset)
        comment = f'  // params[{i}] [{type_str}]'
        if i in info['pvar_offsets']:
            comment += ' (annotated)'
        else:
            comment += ' (estimated)'
        lines.append(f'        {{{offset}, {vtype}}},{comment}')

    lines.append(f'    }}')
    lines.append(f'}};')

    return '\n'.join(lines)


def process_path(path):
    """Process a file or directory, printing C++ output."""
    p = Path(path)
    if p.is_file():
        files = [p]
    elif p.is_dir():
        files = sorted(p.rglob('*.inc'))
    else:
        print(f'Error: {path} is not a file or directory', file=sys.stderr)
        sys.exit(1)

    if not files:
        print(f'No .inc files found in {path}', file=sys.stderr)
        sys.exit(1)

    print('// Auto-generated by gen_formula_offsets.py')
    print('// Do not edit manually — regenerate from .inc files')
    print()
    print('#pragma once')
    print('#include "core/external_formulas.h"')
    print()

    total = 0
    warnings = 0

    for f in files:
        info = parse_inc_file(f)
        if info['formula_const_name'] is None:
            print(f'// SKIP: {f} — no FORMULA_* constant found', file=sys.stderr)
            continue

        cpp = generate_cpp_map(info)
        print(cpp)
        print()
        total += 1

        if info['warnings']:
            for w in info['warnings']:
                print(f'  WARNING [{f.name}]: {w}', file=sys.stderr)
            warnings += 1

    print(f'// Generated {total} FormulaParamMap entries', file=sys.stderr)
    if warnings:
        print(f'// {warnings} files had warnings', file=sys.stderr)


def main():
    if len(sys.argv) < 2:
        print(f'Usage: {sys.argv[0]} <file.inc|directory>', file=sys.stderr)
        print(f'  Single file:  {sys.argv[0]} path/to/formula.inc', file=sys.stderr)
        print(f'  All formulas: {sys.argv[0]} path/to/Formulas/', file=sys.stderr)
        sys.exit(1)

    process_path(sys.argv[1])


if __name__ == '__main__':
    main()
