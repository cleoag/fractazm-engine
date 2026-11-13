"""Compare single-slot doHybridPas orbits with original FPC SSE2 caller + CODE.

Captures actual incoming registers; never seeds them. Retains plain/trace runs
to detect instrumentation effects and separate repeat runs for determinism.
Does not certify arbitrary hybrids, Delphi register allocation or portable code.
"""
import argparse
import hashlib
import json
import math
import random
import subprocess
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
PROFILE = 'fpc-i386-sse2-single-slot-doHybridPas-v1'
PROFILE_MARKER = ('caller_profile=' + PROFILE + ' driver=doHybridPas slot=0 '
                  'counts=1,0,0,0,0,0 start=0 repeat=0 end=0 DEoption=0')


def sha(p):
    return hashlib.sha256(p.read_bytes()).hexdigest()


def equal(a, b):
    return math.isnan(a) and math.isnan(b) or a == b


def run(command, stdin, output):
    result = subprocess.run([str(p) for p in command], input=stdin, text=True,
                            capture_output=True, timeout=30)
    output.write_text(result.stdout)
    output.with_suffix('.stderr').write_text(result.stderr)
    result.check_returncode()
    return [[math.nan if 'nan' in v.lower() else float(v) for v in line.split()]
            for line in result.stdout.splitlines()]


def validate_oracle_identity(oracle):
    """Reuse an immutable oracle only while its binary AND source still match."""
    identity = oracle.parent/'identity.json'
    recorded = json.loads(identity.read_bytes())['sha256']
    for path in (oracle, oracle.with_suffix('.pas')):
        matches = [value for name, value in recorded.items()
                   if Path(name).name == path.name]
        if matches != [sha(path)]:
            raise ValueError('Original caller identity mismatch: '+str(path))
    for name in ('formulas.pas', 'TypeDefinitions.pas', 'Math3D.pas'):
        path = ROOT/'mb3d'/name
        matches = [value for key, value in recorded.items()
                   if Path(key).name == name]
        if matches != [sha(path)]:
            raise ValueError('Immutable Pascal source identity mismatch: '+str(path))
    return identity


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    for arg in ('cpp', 'oracle', 'manifest', 'output', 'layouts'):
        ap.add_argument('--'+arg, type=Path, required=True)
    a = ap.parse_args()
    oracle_identity = validate_oracle_identity(a.oracle)
    a.output.mkdir(parents=True, exist_ok=False)
    rng = random.Random(0x4D414E4F)
    xyz = [[.125,-.25,.5],[0,0,0],[1,0,0],[0,1,0],[0,0,-1],[-.5,.25,-.125],
           [1e-12,-1e-12,1e-12],[1e3,-1e3,1e3]]
    xyz += [[rng.uniform(-1,1) for _ in range(3)] for _ in range(16)]
    profiles = [p+[.2,-.3,.4,julia,limit,stop] for p in xyz
                for julia in (0,1) for limit in (1,2,3,8,32) for stop in (.25,4,256)]
    stdin = ''.join(' '.join(format(v,'.17g') for v in p)+'\n' for p in profiles)
    (a.output/'inputs.txt').write_text(stdin)
    asset = ROOT/'mb3d/M3Formulas/ManowarBulb.m3f'
    code = a.output/'ManowarBulb.bin'
    code.write_bytes(bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0]))
    cases = [c for c in json.loads(a.manifest.read_bytes())['cases'] if c['target_formulas']==['ManowarBulb']]
    if not cases or len({c['id'] for c in cases}) != len(cases):
        raise ValueError('Expected a nonempty manifest with unique Manowar case IDs')
    layouts = json.loads(a.layouts.read_bytes())
    if not layouts.get('finished'):
        raise ValueError('Independent loader evidence is incomplete')
    layout_rows = {r['case']:r for r in layouts['results']}
    sources = [ROOT/p for p in ('comparison/tools/formula_orbit_probe.cpp',
        'src/core/formulas.cpp', 'src/core/formula_iteration_guard.h',
        'src/core/header_trafos.cpp', 'src/io/m3p_parser.cpp',
        'src/core/external_formulas.h', 'src/core/win64_de_x87_helpers.asm',
        'external_formulas/Formulas/3D/ManowarBulb.inc')]
    report = dict(scope=__doc__, profile=PROFILE, finished=False,
                  comparison=dict(max_scaled_error=2e-12,
                    counters_and_instrumentation='exact numeric, NaN classification equal',
                    bitwise_signed_zero_and_nan_payloads=False),
                  overrides=dict(CalcSIT=False, VaryScale=1, Deriv1=1,
                    inputs='XYZ, Julia constants/mode, maxIt and RStop from inputs.txt'),
                  rows_per_case=len(profiles), results=[],
                  identities={str(p):sha(p) for p in (a.cpp,a.oracle,oracle_identity,
                    a.manifest,a.layouts,asset,Path(__file__),*sources)})
    (a.output/'summary.json').write_text(json.dumps(report,indent=2))
    for case in cases:
        scene = ROOT/case['scene']
        assert sha(scene)==case['sha256']
        directory = a.output/case['id']; directory.mkdir()
        blob = directory/'params.bin'
        results = {}
        commands = {}
        parameter_hash = None
        for mode in ('plain','trace'):
            commands['cpp_'+mode] = list(map(str,[a.cpp,scene,blob,mode]))
            commands['pas_'+mode] = list(map(str,[a.oracle,code,blob,mode]))
            output = directory/('cpp_'+mode+'.txt')
            results['cpp_'+mode] = run(commands['cpp_'+mode],stdin,output)
            if output.with_suffix('.stderr').read_text().strip() != PROFILE_MARKER:
                raise ValueError('C++ probe did not certify loaded caller prerequisites')
            if parameter_hash is not None and sha(blob) != parameter_hash:
                raise ValueError('Plain/trace parameter preparation differs')
            parameter_hash = sha(blob)
            results['pas_'+mode] = run(commands['pas_'+mode],stdin,directory/('pas_'+mode+'.txt'))
        failures=[]; exact=0; captured=0
        for key, rows in results.items():
            assert len(rows)==len(profiles), (key,len(rows))
            if key.endswith('_plain'):
                assert all(len(row)==14 and row[13]==0 for row in rows), key
        for i,(c,p) in enumerate(zip(results['cpp_trace'],results['pas_trace'])):
            assert len(c)==14+int(c[13])*4 and len(p)==14+int(p[13])*4
            captured+=int(p[13])
            if len(c)!=len(p):
                failures.append(dict(row=i,kind='call_count',cpp=c[13],pascal=p[13]));continue
            if all(equal(x,y) for x,y in zip(c,p)):exact+=1
            for lane,(x,y) in enumerate(zip(c,p)):
                ok=equal(x,y) or (lane<11 or lane>=14) and math.isfinite(x) and math.isfinite(y) and abs(x-y)<=2e-12*max(1,abs(y))
                if not ok: failures.append(dict(row=i,lane=lane,kind='caller',cpp=str(x),pascal=str(y)))
            for lang in ('cpp','pas'):
                plain=results[lang+'_plain'][i]
                traced=results[lang+'_trace'][i]
                if not all(equal(x,y) for x,y in zip(plain[:13],traced[:13])):
                    failures.append(dict(row=i,kind=lang+'_instrumentation'))
        layout = None
        if case['id'] in layout_rows:
            row=layout_rows[case['id']]
            prior=a.layouts.parent/row['artifact_directory']/'cpp.bin'
            layout=dict(status='pass' if row['status']=='pass' and row['scene_sha256']==sha(scene) and blob.read_bytes()==prior.read_bytes() else 'fail', prior_allocation_sha256=sha(prior))
            if layout['status']!='pass':failures.append(dict(kind='independent_loader_transfer'))
        commands['pas_repeat'] = list(map(str,[a.oracle,code,blob,'trace']))
        repeat=run(commands['pas_repeat'],stdin,directory/'pas_repeat.txt')
        if len(repeat)!=len(profiles) or any(len(x)!=len(y) or not all(equal(u,v) for u,v in zip(x,y)) for x,y in zip(repeat,results['pas_trace'])):
            failures.append(dict(kind='pascal_determinism'))
        row=dict(case=case['id'],scene_sha256=sha(scene),params_sha256=sha(blob),
                 profile=PROFILE, loaded_prerequisites='verified by production GetMCTparasFromHeader',
                 commands=commands,
                 status='fail' if failures else 'pass',rows=len(profiles),exact_rows=exact,
                 captured_calls=captured,failed_lanes=len(failures),examples=failures[:20],independent_loader_transfer=layout)
        report['results'].append(row)
        (a.output/'summary.json').write_text(json.dumps(report,indent=2))
        print(case['id'],row['status'],len(failures),'exact',exact,flush=True)
    report['finished']=True
    report['artifact_sha256']={str(p.relative_to(a.output)):sha(p) for p in sorted(a.output.rglob('*')) if p.is_file() and p.name!='summary.json'}
    (a.output/'summary.json').write_text(json.dumps(report,indent=2))
    return any(r['status']!='pass' for r in report['results'])


if __name__=='__main__':
    raise SystemExit(main())
