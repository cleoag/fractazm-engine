"""Record ordinary volume sampler evidence and its shared-caller boundaries."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

ROOT=Path(__file__).resolve().parents[2]


def main():
    ap=argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--candidate',type=Path,required=True)
    ap.add_argument('--images',type=Path,required=True)
    ap.add_argument('--test-log',type=Path,help='Focused log or completed full renderer log')
    ap.add_argument('--output',type=Path,required=True)
    args=ap.parse_args();identities={}
    def read(path):
        path=path.resolve();raw=path.read_bytes()
        identities[str(path)]=hashlib.sha256(raw).hexdigest();return raw
    def anchor(path,text):
        lines=read(ROOT/path).decode('utf-8',errors='replace').splitlines()
        matches=[i+1 for i,line in enumerate(lines) if text in line]
        if not matches:raise ValueError('Missing source anchor '+path+': '+text)
        return dict(path=path,lines=matches,anchor=text)
    oracle_path=ROOT/'comparison/results/stage-a-20261018/volume-sampling-oracle-v2/identity.json'
    oracle=json.loads(read(oracle_path))
    log_path=args.test_log or args.candidate/'test_volume_sampling_original.log'
    log=read(log_path).decode()
    matches=re.findall(r'Results: (\d+) passed, (\d+) failed',log)
    if len(matches)!=1:raise ValueError('Test log must contain one completed result')
    aggregate_passed,aggregate_failed=map(int,matches[0])
    markers=re.findall(r'\[ (PASS|FAIL) \] test_volume_sampling_original\b',log)
    if markers:
        if len(markers)!=1:raise ValueError('Ambiguous full-suite volume test result')
        # The runner prints this marker only after the entire test returns and
        # compares the failure counter before/after this named test. The scoped
        # denominator comes from 2 checks per fixture plus 2 rejection checks;
        # the full-suite aggregate must not be attributed to this single test.
        scoped_status=markers[0].lower()
        passed=6068 if scoped_status=='pass' else None
        failed=0 if scoped_status=='pass' else None
        count_basis='completed named test marker; 2 checks per 3033 rows plus 2 rejection checks'
    else:
        if aggregate_passed+aggregate_failed!=6068:raise ValueError('Changed finite fixture denominator')
        passed,failed=aggregate_passed,aggregate_failed
        scoped_status='pass' if failed==0 else 'fail'
        count_basis='focused test aggregate'
    if oracle['rows']!=3033:raise ValueError('Changed oracle row count')
    read(ROOT/'tests/fixtures/volume_sampling.h')
    read(ROOT/'tests/test_renderer.cpp')
    build_identity_path=args.candidate/'build-identity.json'
    build_identity=json.loads(read(build_identity_path))
    build_identity['sources']=[dict(Path=str((ROOT/row.get('Path',row.get('path'))).resolve()),
        Hash=row.get('Hash',row.get('sha256'))) for row in build_identity['sources']]
    for filename in ('reflection.cpp','reflection.h','test_renderer.cpp'):
        records=[row for row in build_identity['sources'] if Path(row['Path']).name==filename]
        if len(records)!=1:raise ValueError('Missing frozen source identity: '+filename)
        row=records[0]
        if hashlib.sha256(read(Path(row['Path']))).hexdigest()!=row['Hash'].lower():
            raise ValueError('Source changed since candidate freeze: '+filename)
    images=json.loads(read(args.images))
    if not images.get('finished') or len(images['results'])!=8:raise ValueError('Incomplete image set')
    binary=read(args.candidate/'mb3d_render.exe')
    if images['cpp_sha256']!=hashlib.sha256(binary).hexdigest():raise ValueError('Image candidate identity mismatch')
    read(args.candidate/'mb3d_tests_renderer.exe')
    result=dict(schema_version=1,scope='Finite ordinary directional/positional volume lookup and shared fog integration',
        excluded=['Monte Carlo','JIT'],oracle=dict(path=str(oracle_path),identity=oracle),
        build_identity=str(build_identity_path.resolve()),
        numerical=dict(rows=3033,lookup_assertions=3033,length_assertions=3033,
            explicit_original_undefined_rejections=2,status=scoped_status,
            passed=passed,failed=failed,count_basis=count_basis,
            test_log=str(log_path.resolve()),
            whole_log_aggregate=dict(passed=aggregate_passed,failed=aggregate_failed)),
        images=dict(report=str(args.images.resolve()),measured=8,
            counts=dict(Counter(row['status'] for row in images['results'])),
            exact_rgb=sum(row.get('exact_rgb',False) for row in images['results']),
            thresholds=images['thresholds'],binary_sha256=images['cpp_sha256']),
        source=[anchor('mb3d/maps/MB3DMaps.pas','function VolLightMapPosSSE'),
            anchor('mb3d/maps/MB3DMaps.pas','function GetVolLightMapVecSSE'),
            anchor('mb3d/Math3D.pas','function SqrLengthOfSVec'),
            anchor('mb3d/CalcThread.pas','vs := SubtractVectors2s(v, VolumeLightMap.LightPos);'),
            anchor('mb3d/CalcSR.pas','vs := SubtractVectors2s(v, VolumeLightMap.LightPos);'),
            anchor('src/core/reflection.cpp','bool DirectionalVolumeLightMapContains'),
            anchor('src/core/reflection.cpp','float SamplePositionalVolumeLightMap'),
            anchor('src/core/reflection.cpp','void VolLightDoDynFogSubstep'),
            anchor('src/core/ray_marcher.cpp','VolLightDoDynFogSubstep(')],
        semantics=[
            'Six signed faces share one dominant-axis lookup; tie priority x, then y, then z is explicit.',
            'Negative faces divide by signed dominant component; Single conversion precedes comparisons, division and products.',
            'Positional texels round nearest-even before adding HalfSize; directional SSE adds HSizeS and clamps before rounding.',
            'Directional fixtures cover identity/mixed rotation, odd/even HalfSize, clamped extremes and comparison depth.',
            'SqrLengthOfSVec receives Single components but returns x87 to a Double caller store, separately checked against extracted assembly.',
            'Primary and secondary ray callers invoke the same C++ fog substep and lookup helpers; source sharing justifies no duplicate sampler matrix.'],
        limitations=[
            'Finite sampled coordinates and 3 cube sizes do not prove all continuous inputs or non-SSE numerical profiles.',
            'Undefined positional zero direction is rejected explicitly; original cvtss2si of NaN produces an invalid memory index, not a parity PASS.',
            'Eight existing images cover directional source-slot permutations and two positional source slots; numerical fixtures establish signed face selection independently.',
            'Nonfinite directional projection behavior is not certified by this finite witness set.',
            'Final stage A acceptance still requires replay on the final frozen binary.'],identities=identities)
    args.output.parent.mkdir(parents=True,exist_ok=True);args.output.write_text(json.dumps(result,indent=2)+'\n')
    print(json.dumps(dict(numerical=result['numerical'],images=result['images']['counts'])))


if __name__=='__main__':main()
