"""Adjudicate retained raw loader differences using original CODE and Pascal math.

This verifies used numeric bytes for the recorded scenes. It does not relabel
raw allocation inequality as equality, nor certify arbitrary option values.
"""
import argparse
from collections import Counter
import json
from pathlib import Path
import struct
import subprocess

from audit_original_integer_domains import source_analysis
from external_validation import ROOT, decode, digest, prepare_external_options, slots, write_json
from gen_bulk_formula_table import compute_pvar_offsets


# Full CODE review of the only stack-spilled PVar acquisition in this set:
# 0010..0019: fourth argument -> [ebp-4c] -> eax -> [eax+30] -> ebx.
# 0028,003e,004a,0056,00ba,00da,00ff..0102 read [PVar+0..39].
# The other eax reloads at ebp-4 refer to the coordinate pointer, not PVar.
MSLTOE_CODE = '0304ba54dc3f96cf596595f3bdb6e01b8eab8b7194ebf30f13450cd1d9684587'


def matrix_specs(metadata, values):
    saved = 0
    types = metadata['option_types']
    widths = {5:3, 6:3, 12:6, 18:4, 19:2}
    result = []
    for k, kind in enumerate(types):
        if kind == 6:
            start = len(compute_pvar_offsets(types[:k]))
            offsets = [o for o,t in compute_pvar_offsets(types)[start:start+9]]
            result.append((offsets, values[saved:saved+3]))
        saved += widths.get(kind, 1)
    return result


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--layouts', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    layouts = json.loads(args.layouts.read_bytes())
    inventory = ROOT / 'comparison/runs/external-complete/manifest.json'
    formulas = {f['name']:f for f in json.loads(inventory.read_bytes())['formulas']}
    probe = ROOT / 'comparison/runs/external-complete/heightmap-family-original-rotation-v1/heightmap_original_rotation_probe.exe'
    source = probe.with_suffix('.pas')
    original = ROOT / 'mb3d/Math3D.pas'
    text, unchanged = source.read_text(), original.read_text(encoding='latin1')
    for begin in ('procedure BuildRotMatrix(', 'procedure SinCosD(', 'procedure ScaleMatrix('):
        block = text[text.index(begin):]
        block = block[:block.index('\nend;')+5]
        if block not in unchanged:
            raise ValueError('Matrix probe no longer contains unchanged original: '+begin)
    evidence = {str(p):digest(p.read_bytes()) for p in (
        args.layouts, inventory, probe, source, original, Path(__file__),
        ROOT / 'comparison/tools/audit_original_integer_domains.py',
        ROOT / 'comparison/tools/external_validation.py',
        ROOT / 'scripts/gen_bulk_formula_table.py')}
    rows = []
    for row in layouts['results']:
        if row['status'] != 'raw_difference':
            continue
        name = row['formula']
        f = formulas[name]
        asset = ROOT / f['asset']
        asm = ROOT / 'external_formulas/asm' / (name+'.asm')
        if not asm.exists():
            asm = asm.with_name(name.replace(' ', '_')+'.asm')
        analysis = source_analysis(asset, asm)
        evidence.update({str(p):digest(p.read_bytes()) for p in (asset, asm)})
        result = dict(formula=name, scene_sha256=row['scene_sha256'], raw_status='raw_difference',
                      status='unresolved', source_analysis=analysis, matrix_checks=[])
        rows.append(result)
        if any(analysis.get(k) for k in ('error','ambiguous','calls','indirect_control_flow')):
            continue
        # Unsized x86 memory operands conservatively cover 16 bytes, the largest
        # operand in these SSE2 CODE blocks. Never discard an unknown width.
        used = {b for a in analysis['accesses'] for b in range(a['offset'], a['offset']+(a['width'] or 16))}
        if name == 'Msltoe_Sym':
            if analysis['original_code_sha256'] != MSLTOE_CODE:
                raise ValueError('Reviewed Msltoe stack-pointer code changed')
            used.update(range(40))
            result['manual_alias_review'] = 'Stack spill at0013/0016, PVar acquisition0019; only numeric constants PVar[0:40] are accessed.'
        result['used_byte_offsets'] = sorted(used)
        differences = set(row['pvar_relative_differences'])
        result['unused_byte_differences'] = sorted(differences-used)
        unresolved = differences & used
        cpp = Path(row['command'][3]).read_bytes()
        if digest(cpp) != row['cpp_allocation_sha256']:
            raise ValueError('C++ allocation changed')
        scene = Path(row['scene'])
        if digest(scene.read_bytes()) != row['scene_sha256']:
            raise ValueError('Scene changed')
        data = decode(scene)
        slot = next(s for s in slots(data) if s['name'] == name)
        prepare_external_options(data, slot, f['metadata'])
        values = next(s for s in slots(data) if s['slot']==slot['slot'])['options']
        for offsets, angles in matrix_specs(f['metadata'], values):
            matrix_bytes = {b for o in offsets for b in range(o,o+4)}
            if not matrix_bytes & unresolved:
                continue
            line = ' '.join(format(v,'.17g') for v in [*angles,1.])+'\n'
            expected = list(map(int,subprocess.check_output([str(probe)],input=line.encode()).split()))
            if len(expected)!=9:
                raise ValueError('Invalid matrix probe output')
            actual = [struct.unpack_from('<I',cpp,512+o)[0] for o in offsets]
            check = dict(angles=angles, offsets=offsets, original_bits=expected, cpp_bits=actual, exact=actual==expected)
            result['matrix_checks'].append(check)
            if check['exact']:
                unresolved -= matrix_bytes
        result['unexplained_used_byte_differences'] = sorted(unresolved)
        result['status'] = 'used_numeric_exact' if not unresolved else 'unresolved'
    report = dict(scope=__doc__, finished=True, cpp_sha256=layouts['cpp_sha256'],
                  results=rows, counts=dict(Counter(r['status'] for r in rows)),
                  source_and_evidence_sha256=evidence,
                  limits=['Numeric loader bytes for the recorded scenes only.',
                          'Raw FAIL records remain unequal.',
                          'Original CODE review excludes uninitialized bytes; callback/resource semantics have separate tests.'])
    write_json(args.output, report)
    print(json.dumps(report['counts']))
    return int(any(r['status']=='unresolved' for r in rows))


if __name__ == '__main__':
    raise SystemExit(main())
