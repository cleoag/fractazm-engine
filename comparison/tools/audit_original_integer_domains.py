"""Classify INTEGER uses in immutable original CODE, without claiming parity.

Disassembly bytes must match the M3F asset. A small CFG analysis follows only
known PVar/iteration pointers and constant pointer offsets; uncertain aliases,
calls and indirect control flow keep the qualification explicit. Numerical
INTEGER uses are not mistaken for finite selectors.
"""
import argparse
from collections import Counter, deque
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
REGS = ('eax', 'ebx', 'ecx', 'edx', 'esi', 'edi', 'ebp', 'esp')
WIDTH = {'byte': 1, 'word': 2, 'dword': 4, 'qword': 8, 'xmmword': 16, 'tbyte': 10}
INS = re.compile(r'^([0-9A-Fa-f]+):\s+((?:[0-9A-Fa-f]{2}\s+)+)\s*(\w+)\s*(.*?)\s*(?:;.*)?$')
MEM = re.compile(r'(?:(byte|word|dword|qword|xmmword|tbyte) ptr )?\[([^]]+)\]')
SIMPLE = re.compile(r'^(e[a-z]{2})(?:\s*([+-])\s*(0x[0-9a-f]+|\d+))?$')
JCC = {'je', 'jz', 'jne', 'jnz', 'jl', 'jnge', 'jle', 'jng', 'jg', 'jnle', 'jge', 'jnl',
       'jb', 'jc', 'jnae', 'jbe', 'jna', 'ja', 'jnbe', 'jae', 'jnb', 'jnc', 'js', 'jns', 'jo', 'jno'}
FLAGS = {'add', 'sub', 'cmp', 'test', 'and', 'or', 'xor', 'inc', 'dec', 'neg', 'mul', 'imul',
         'div', 'idiv', 'shl', 'shr', 'sar', 'sal', 'rol', 'ror', 'rcl', 'rcr', 'sahf', 'popfd',
         'bt', 'bts', 'btr', 'btc', 'comisd', 'ucomisd', 'fcomi', 'fcomip', 'fucomi', 'fucomip'}


def sha(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def number(value):
    try:
        return int(value, 0)
    except ValueError:
        return None


def memory(expression):
    match = SIMPLE.fullmatch(expression)
    if not match:
        return None
    return match[1], (int(match[3], 0) * (-1 if match[2] == '-' else 1) if match[3] else 0)


def parse(path):
    instructions = []
    for line, text in enumerate(path.read_text().splitlines(), 1):
        match = INS.match(text)
        if match:
            instructions.append(dict(address=int(match[1], 16), data=bytes.fromhex(match[2]),
                                     op=match[3], args=match[4].split(';')[0].strip(), line=line))
    return instructions


def source_analysis(asset, asm):
    source = asset.read_text(errors='replace')
    code = bytes.fromhex(source.split('[CODE]')[1].split('[END]')[0])
    ins = parse(asm)
    if not ins or b''.join(i['data'] for i in ins) != code or any(
            a['address'] + len(a['data']) != b['address'] for a, b in zip(ins, ins[1:])):
        return dict(error='Disassembly is incomplete or does not match original asset CODE')
    index = {i['address']: n for n, i in enumerate(ins)}
    edges, indirect = {}, []
    for n, i in enumerate(ins):
        op, args = i['op'], i['args']
        successors = [] if op.startswith('ret') or op == 'jmp' else [n+1] if n+1 < len(ins) else []
        if op.startswith('j') or op.startswith('loop') or op == 'call':
            target = number(args)
            if target in index:
                successors.append(index[target])
            elif op != 'call':
                indirect.append(i['line'])
        edges[n] = successors
    base = tuple(frozenset({('other', 0)}) for _ in REGS)
    de = re.search(r'(?im)^\s*\.?DEoption\s*=\s*(-?\d+)', source)
    if de and int(de[1]) in {20, 21}:
        base = list(base)
        base[REGS.index('edi')] = frozenset({('pvar', 0)})
        base[REGS.index('esi')] = frozenset({('iteration_ifs', 0)})
        base = tuple(base)
    states = {0: base}
    queue = deque([0])
    calls = []
    while queue:
        n = queue.popleft()
        i, state = ins[n], list(states[n])
        op, operands = i['op'], [s.strip() for s in i['args'].split(',')]
        dst, src = operands[0] if operands else '', operands[1] if len(operands) > 1 else ''
        if op == 'call':
            calls.append(i['line'])
        if dst in REGS and op not in {'cmp', 'test', 'push'}:
            result = frozenset({('other', 0)})
            if op == 'mov' and src in REGS:
                result = state[REGS.index(src)]
            elif op in {'add', 'sub'} and number(src) is not None:
                delta = number(src) * (-1 if op == 'sub' else 1)
                result = frozenset((kind, offset+delta if kind not in {'other', 'unknown'} else 0)
                                   for kind, offset in state[REGS.index(dst)])
            elif op in {'mov', 'lea'}:
                m = MEM.fullmatch(src)
                address = memory(m[2]) if m else None
                if address:
                    reg, delta = address
                    if op == 'lea':
                        result = frozenset((kind, offset+delta if kind not in {'other', 'unknown'} else 0)
                                           for kind, offset in state[REGS.index(reg)])
                    elif reg == 'ebp' and delta == 8:
                        # Standard Delphi register-call prologue: fourth arg is Iteration3D.
                        result = frozenset({('iteration', 0)})
                    elif state[REGS.index(reg)] == frozenset({('iteration', 0)}) and delta == 0x30:
                        result = frozenset({('pvar', 0)})
            state[REGS.index(dst)] = result
        for successor in edges[n]:
            if successor not in states:
                joined = tuple(state)
            else:
                joined = tuple(a | b for a, b in zip(states[successor], state))
                # Loops that compute variable addresses are deliberately not solved here.
                joined = tuple(s if len(s) <= 6 else frozenset({('unknown', 0)}) for s in joined)
            if states.get(successor) != joined:
                states[successor] = joined
                queue.append(successor)
    accesses, ambiguous = [], []
    for n, state in states.items():
        i = ins[n]
        for m in MEM.finditer(i['args']):
            simple = memory(m[2])
            if not simple:
                if any(any(k == 'pvar' for k, _ in state[REGS.index(reg)]) for reg in REGS if reg in m[2]):
                    ambiguous.append(dict(line=i['line'], instruction=i['op']+' '+i['args'], reason='indexed PVar access'))
                continue
            reg, delta = simple
            origins = state[REGS.index(reg)]
            pvars = [(k, o) for k, o in origins if k == 'pvar']
            if any(k == 'unknown' for k, _ in origins):
                ambiguous.append(dict(line=i['line'], instruction=i['op']+' '+i['args'], reason='widened pointer origin'))
            if pvars and len(origins) != 1:
                ambiguous.append(dict(line=i['line'], instruction=i['op']+' '+i['args'], reason='ambiguous PVar origin'))
            for _, offset in pvars:
                accesses.append(dict(line=i['line'], instruction=i['op']+' '+i['args'],
                                     op=i['op'], args=i['args'], offset=offset+delta,
                                     width=WIDTH.get(m[1], 0), node=n, memory=m[0]))

    def consumers(start):
        todo, seen, uses = list(edges[start]), set(), set()
        while todo:
            n = todo.pop()
            if n in seen:
                continue
            seen.add(n)
            op = ins[n]['op']
            if op in FLAGS or op == 'call' or op.startswith('ret'):
                continue
            if op.startswith('j') and op != 'jmp':
                if op not in JCC:
                    return None
                uses.add(op)
            elif op.startswith(('cmov', 'set')) or op in {'lahf', 'pushfd', 'adc', 'sbb'}:
                return None
            todo.extend(edges[n])
        return sorted(uses)

    for access in accesses:
        if access['op'] in {'cmp', 'test'}:
            access['flag_consumers'] = consumers(access['node'])
    return dict(accesses=accesses, ambiguous=ambiguous, calls=sorted(set(calls)),
                indirect_control_flow=indirect, original_code_sha256=hashlib.sha256(code).hexdigest(),
                original_code_bytes=len(code), reached_instructions=len(states))


def predicate(value, bound, branch):
    x, y = value & 0xffffffff, bound & 0xffffffff
    d = (x-y) & 0xffffffff
    z, s, c = d == 0, bool(d & 0x80000000), x < y
    o = bool(((x ^ y) & (x ^ d)) & 0x80000000)
    return {'je': z, 'jz': z, 'jne': not z, 'jnz': not z,
            'jl': s != o, 'jnge': s != o, 'jge': s == o, 'jnl': s == o,
            'jle': z or s != o, 'jng': z or s != o, 'jg': not z and s == o, 'jnle': not z and s == o,
            'jb': c, 'jc': c, 'jnae': c, 'jae': not c, 'jnb': not c, 'jnc': not c,
            'jbe': c or z, 'jna': c or z, 'ja': not c and not z, 'jnbe': not c and not z,
            'js': s, 'jns': not s, 'jo': o, 'jno': not o}[branch]


def partition(reads):
    tests = []
    for read in reads:
        operands = [p.strip() for p in read['args'].split(',')]
        if read['op'] != 'cmp' or read['width'] != 4 or operands[0] != read['memory']:
            return None
        bound = number(operands[1])
        branches = read.get('flag_consumers')
        if bound is None or not branches:
            return None
        tests.extend((bound, branch) for branch in branches)
    tests = sorted(set(tests))
    def signature(value):
        return tuple(predicate(value, b, j) for b, j in tests)
    candidates = {0, 1, -1, -(1 << 31), (1 << 31)-1}
    for bound, _ in tests:
        for boundary in (bound, bound+(1 << 31)):
            for delta in (-1, 0, 1):
                v = (boundary+delta) & 0xffffffff
                candidates.add(v if v < 1 << 31 else v-(1 << 32))
    representatives = {}
    for value in sorted(candidates, key=lambda v: (abs(v), -v)):
        representatives.setdefault(signature(value), value)
    return tests, signature, sorted(representatives.values())


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--selectors', type=Path, required=True)
    ap.add_argument('--fixtures', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--include-classified', action='store_true', help='Audit all INTEGER declarations, including prior finite C++ heuristic domains.')
    args = ap.parse_args()
    audit = json.loads(args.selectors.read_bytes())
    fixtures = {r['formula']: r for r in json.loads(args.fixtures.read_bytes())['formulas']}
    cache, rows = {}, []
    for option in audit['options']:
        if not args.include_classified and option['input_presence_status'] != 'unclassified_domain':
            continue
        name = option['formula']
        if name not in cache:
            asm = ROOT / 'external_formulas/asm' / (name+'.asm')
            if not asm.exists():
                asm = asm.with_name(name.replace(' ', '_')+'.asm')
            asset = ROOT / option['asset']['path']
            result = source_analysis(asset, asm) if asm.exists() else dict(error='No original disassembly')
            result['asm'] = dict(path=asm.relative_to(ROOT).as_posix(), sha256=sha(asm)) if asm.exists() else None
            cache[name] = result
        analysis = cache[name]
        row = {k: option[k] for k in ('formula', 'stored_value_index', 'runtime_param_index', 'pvar_offset', 'label')}
        row.update(classification='requires_dataflow_review', domain_complete=False, parity_status='not_assessed')
        row['scene_values'] = option['observed_int32_values']
        row['fixture_values'] = fixtures.get(name, {}).get('effective_integer_values', {}).get(str(option['stored_value_index']), [])
        row['fixture_headers'] = fixtures.get(name, {}).get('fixtures', [])
        if 'error' in analysis:
            row['blockers'] = [analysis['error']]
            rows.append(row)
            continue
        offset = option['pvar_offset']
        reads = [a for a in analysis['accesses'] if a['offset'] < offset+4 and a['offset']+a['width'] > offset]
        row['original_uses'] = [{k:v for k,v in a.items() if k not in {'node', 'memory', 'args'}} for a in reads]
        row['blockers'] = []
        if analysis['ambiguous']: row['blockers'].append('Ambiguous/indexed PVar accesses require review')
        if analysis['calls']: row['blockers'].append('Calls require interprocedural dataflow qualification')
        if analysis['indirect_control_flow']: row['blockers'].append('Indirect control flow requires review')
        exact = bool(reads) and all(a['offset'] == offset for a in reads)
        part = partition(reads) if exact else None
        if part:
            tests, signature, reps = part
            values = row['scene_values'] + row['fixture_values']
            signatures = {signature(v) for v in values}
            row.update(classification='original_direct_comparison_partition', domain_complete=not row['blockers'],
                       predicates=[dict(bound=b, branch=j) for b,j in tests], representatives=reps,
                       missing_input_classes=[v for v in reps if signature(v) not in signatures],
                       equivalence='Same original integer CMP/jcc results at every identified use; numeric/state inputs stay independent.')
        elif reads:
            ops = {a['op'] for a in reads}
            if any(a['width'] > 4 or a['offset'] != offset for a in reads):
                row['classification'] = 'overlapping_packed_numeric_bits'
            elif ops <= {'fild', 'fimul', 'fidiv', 'fidivr', 'fiadd', 'fisub', 'fisubr', 'cmp'}:
                row['classification'] = 'integer_in_numeric_expression'
            elif option.get('resource_map_id'):
                row['classification'] = 'resource_identifier'
            elif all(a['width'] == 1 for a in reads):
                row['classification'] = 'low_byte_algorithmic_input'
            elif len(row['fixture_values']) >= 20:
                row['classification'] = 'algorithmic_input_with_existing_boundary_samples'
        row['closure_obligation'] = (
            'Verify missing source partitions with loaded native parameters; qualify C++/native mapping and coupled state separately.'
            if part else 'Review algorithmic dataflow and finite semantic boundaries; scene/fixture presence alone is not proof.')
        rows.append(row)
    report = dict(scope=__doc__, input_sha256={str(args.selectors):sha(args.selectors), str(args.fixtures):sha(args.fixtures),
                                             str(Path(__file__).relative_to(ROOT)):sha(Path(__file__))},
                  totals=dict(options=len(rows), classifications=dict(Counter(r['classification'] for r in rows)),
                              qualified_original_finite_domains=sum(r['domain_complete'] for r in rows),
                              finite_missing_classes=sum(len(r.get('missing_input_classes', [])) for r in rows), parity_passes_produced=0),
                  options=rows, original_sources={n:{k:v for k,v in a.items() if k!='accesses'} for n,a in cache.items()},
                  limitations=['Only original direct memory predicates are partitioned automatically.',
                               'Original-code partition is not qualification of the C++ implementation or complete interactions.',
                               'Numerical inputs, counters and resource IDs require algorithm-specific boundaries, not blind integer enumeration.'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report['totals']))


if __name__ == '__main__':
    main()
