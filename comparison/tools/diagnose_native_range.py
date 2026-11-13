#!/usr/bin/env python3
"""Reproduce Fuzzy/Sin4Dplus range witnesses; NOT whole-frame qualification."""
import argparse
from fractions import Fraction
import json
import math
from pathlib import Path
import struct
from native_gpu_coverage import sha256
from run_m3parameter_gpu import invoke
ROOT = Path(__file__).resolve().parents[2]
MAX32 = Fraction(2)**128 - Fraction(2)**104


def value(word): return struct.unpack('<f', struct.pack('<I', word))[0]
def word(x): return struct.unpack('<I', struct.pack('<f', x))[0]
def f32(x): return value(word(x))
def write_words(path, rows): path.write_bytes(b''.join(struct.pack('<'+'I'*len(row), *row) for row in rows))


def witness(name, row):
    a = list(map(value, row['input_words'])); b = list(map(value, row['result_words']))
    if name == 'Fuzzy':
        assert not row['fault'] and all(map(math.isfinite, b[:3]))
        exact = sum(Fraction(x)**2 for x in b[:3])
        assert exact > MAX32
        return {'kind': 'squared_radius_overflow', 'gpu_output_binary32': row['result_words'][:3],
                'square_sum': [str(exact.numerator), str(exact.denominator)], 'multiple_of_FLT_MAX': float(exact/MAX32)}
    r = math.sqrt(a[3]**2 + a[1]**2 + a[2]**2)
    assert 90 < r < 700 and row['fault'] == 4 and abs(a[0]) < 2**63
    e = math.exp(r); factor = math.cos(a[0]) * (e-1/e)*.5/r
    result = [math.sin(a[0])*(e+1/e)*.5*1.5, a[1]*factor*1.5, a[2]*factor*1.5, a[3]*factor*1.5]
    assert all(map(math.isfinite, result)) and max(map(abs, result)) > float(MAX32)
    return {'kind': 'exponential_coordinate_overflow', 'gpu_input_binary32': row['input_words'][:4],
            'radius': r, 'fp64_formula_result': result, 'gpu_fault': row['fault']}


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument('--build', type=Path, required=True); p.add_argument('--output', type=Path, required=True)
    a = p.parse_args(); out = a.output.resolve(); build = a.build.resolve()
    assert not out.exists(); out.mkdir(parents=True)
    identities = {str(x): sha256(x) for x in (build/'mb3d_native_formula_probe', build/'metal_native_scene.metallib', Path(__file__).resolve())}
    records = []
    for name, family in [('Fuzzy', '3D'), ('_Sin4Dplus', '4D')]:
        def run(label, mode, inputs, expected_exit=0):
            folder = out / (name+'-'+label)
            cmd = [build/'mb3d_native_formula_probe', *inputs, '--mode', mode, '--repeats', '3',
                   '--library', build/'metal_native_scene.metallib', '--output', folder/'probe.json']
            event = invoke(cmd, folder, ROOT, timeout=90, evidence_root=ROOT)
            assert event['exit_code'] == expected_exit and not event['guard']
            d = json.loads((folder/'probe.json').read_text())
            assert d['formula_abi'] == [1,8192,188,236,32] and d['library_profile'] == [2,4,32]
            assert d['exact_state_repeats'] and d['repeats'] == 3 and d['input_immutable'] and d['allocation_canaries']
            assert d['invalid_descriptor_controls'] == 7
            return d
        scene = ROOT/'comparison/scenes'/('extall_'+family+'_'+name+'.m3p')
        base = run('distance', 'distance', ['--scene', scene])
        program = base['program_words']; program_file = out/(name+'-program.bin')
        write_words(program_file, [program])
        matrix, labels = [], []
        # ABI state: c[0:4] words4:8; rstop26; max_iterations36.
        for i, row in enumerate(base['rows']):
            for axis in (-1,0,1,2):
                for limit in range(1,7):
                    state = row['input_words'].copy(); state[26] = program[21]; state[36] = limit
                    if axis >= 0:
                        x = value(state[4+axis]); upward = value(word(x)+(1 if x >= 0 else -1))
                        spacing = f32(upward-x); requested = value(program[19])
                        state[4+axis] = word(f32(x+max(requested, f32(spacing*8))))
                    matrix.append(state); labels.append({'point': i, 'axis': axis, 'limit': limit})
        matrix_file = out/(name+'-matrix.bin'); write_words(matrix_file, matrix)
        orbits = run('orbits', 'orbit', ['--program', program_file, '--states', matrix_file])
        before, selected = [], []
        for row, label in zip(orbits['rows'], labels):
            result = row['result_words']
            if result[33] < label['limit'] and value(result[24]) <= value(result[26]):
                before.append(result[:47]); selected.append(label)
        assert before
        state_file = out/(name+'-before-exit.bin'); write_words(state_file, before)
        steps = run('next-step', 'step', ['--program', program_file, '--states', state_file], 0 if name=='Fuzzy' else 2)
        cases = [dict(label, **witness(name,row)) for label,row in zip(selected,steps['rows'])]
        records.append({'formula': name, 'scene_sha256': sha256(scene), 'matrix_cases': len(matrix),
                        'three_exact_device_repeats': True, 'witnesses': cases})
    assert all(sha256(Path(x)) == h for x,h in identities.items())
    report = {'status':'PARTIAL_DIAGNOSIS', 'qualification':False, 'identities':identities,
              'limit':'Controlled orbit/gradient samples only; no complete-frame range attribution.', 'records':records}
    (out/'report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps({'status':report['status'], 'witness_counts':{r['formula']:len(r['witnesses']) for r in records}}))


if __name__ == '__main__': main()
