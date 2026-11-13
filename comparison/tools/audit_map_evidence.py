"""Summarize finite ordinary-map evidence without promoting image samples to proof."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
BASE = 'comparison/results/stage-a-20261018/'
RUN = 'comparison/runs/stage-a-20261018/'


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--output', type=Path, required=True)
    ap.add_argument('--candidate', type=Path, default=ROOT/(RUN+'candidate-v15'))
    ap.add_argument('--matrix-root', type=Path,
                    help='New finite-matrix replay root containing maps18/preprocess8/heightmap4')
    ap.add_argument('--renderer-log', type=Path,
                    help='Completed full renderer log; uses named test status with separate finite denominators')
    args = ap.parse_args()
    identities = {}

    def load(path):
        data = (ROOT/path).read_bytes()
        identities[path] = hashlib.sha256(data).hexdigest()
        return data

    def anchor(path, text):
        lines = load(path).decode('utf-8', errors='replace').splitlines()
        found = [i+1 for i, line in enumerate(lines) if text in line]
        if not found:
            raise ValueError(f'Missing source anchor {path}: {text}')
        return dict(path=path, lines=found, anchor=text)

    images = []
    for name, current, count in [('render-maps-bmp-candidate-v15','maps18',18),
                        ('render-map-preprocess-candidate-v15','preprocess8',8),
                        ('formula-render-maps-candidate-v15','heightmap4',4)]:
        path = str(args.matrix_root/current/'summary.json') if args.matrix_root else RUN+name+'/summary.json'
        report = json.loads(load(path))
        if len(report['results']) != count or not report['finished']:
            raise ValueError('Incomplete named evidence '+path)
        images.append(dict(report=path, cpp_sha256=report['cpp_sha256'],
            measured_cases=count, counts=dict(Counter(r['status'] for r in report['results'])),
            exact_rgb_cases=sum(r.get('exact_rgb',False) for r in report['results']),
            reference=report['reference'], thresholds=report['thresholds']))

    candidate = args.candidate.resolve()
    build_path = candidate/'build-identity.json'
    if build_path.exists():
        build = json.loads(load(str(build_path)))
        for group in ('sources','binaries'):
            build[group]=[dict(Path=str((ROOT/row.get('Path',row.get('path'))).resolve()),
                Hash=row.get('Hash',row.get('sha256'))) for row in build[group]]
        binary_identity = {Path(row['Path']).stem: row['Hash'].lower()
                           for row in build['binaries']
                           if Path(row['Path']).name in ('mb3d_render.exe','mb3d_tests_renderer.exe')}
        for row in build['sources']:
            if Path(row['Path']).name in ('lightmap.cpp','lightmap.h','colorizer.cpp','test_renderer.cpp'):
                if hashlib.sha256(load(row['Path'])).hexdigest()!=row['Hash'].lower():
                    raise ValueError('Map source changed since build: '+row['Path'])
    else:
        binary_identity = json.loads(load(str(candidate/'renderer-identity.json')))
    for name, expected_hash in binary_identity.items():
        path=str(candidate/(name+'.exe'))
        load(path)
        if identities[path] != expected_hash:
            raise ValueError('Frozen executable changed '+path)
    if any(row['cpp_sha256']!=binary_identity['mb3d_render'] for row in images):
        raise ValueError('Images do not belong to selected candidate')
    numeric = []
    for name, expected, oracle in [
        ('test_map_preprocessing_original',32,'map-preprocessing-oracle-v1'),
        ('test_map_sampling_original',2520,'map-sampling-oracle-v2'),
        ('test_map_preprocessing_small_widths',12,None)]:
        path=str(args.renderer_log or candidate/(name+'.log'))
        text=load(path).decode()
        matches=re.findall(r'Results: (\d+) passed, (\d+) failed',text)
        if len(matches)!=1:
            raise ValueError('Unrecognized test log '+path)
        aggregate_passed,aggregate_failed=map(int,matches[0])
        if args.renderer_log:
            markers=re.findall(r'\[ (PASS|FAIL) \] '+re.escape(name)+r'\b',text)
            if len(markers)!=1:
                raise ValueError('Missing named map test: '+name)
            passed,failed=(expected,0) if markers[0]=='PASS' else (None,None)
            status=markers[0].lower()
        else:
            passed,failed=aggregate_passed,aggregate_failed
            if passed+failed!=expected:
                raise ValueError('Changed test denominator '+path)
            status='pass' if failed==0 else 'fail'
        item=dict(test=name,log=path,passed=passed,failed=failed,assertions=expected,
                  status=status,whole_log_aggregate=dict(passed=aggregate_passed,failed=aggregate_failed),
                  count_basis='Completed named test marker with fixture denominator' if args.renderer_log else 'Focused aggregate')
        if oracle:
            identity=BASE+oracle+'/identity.json'
            item.update(independent_oracle=identity,oracle=json.loads(load(identity)))
        numeric.append(item)

    report=dict(schema_version=1,
        scope='Ordinary map loading, preprocessing and spline consumers; finite fixtures and controlled images',
        excluded=['Monte Carlo','JIT','P5 binary-PGM extension'],
        source=[anchor('mb3d/heightmapgen/PNMReader.pas',"if Line <> 'P2' then"),
            anchor('mb3d/maps/MB3DMaps.pas','procedure CalcLMavrgCol'),
            anchor('mb3d/maps/MB3DMaps.pas','procedure SmoothLightMap'),
            anchor('mb3d/maps/MB3DMaps.pas','procedure FitLMborder'),
            anchor('mb3d/Math3D.pas','function MakeSplineCoeff'),
            anchor('mb3d/ImageProcess.pas','function GetLightMapPixel('),
            anchor('mb3d/Calc.pas','function SplineIpolMap('),
            anchor('src/core/lightmap.cpp','static bool LoadPGMLightMapFile'),
            anchor('src/core/lightmap.cpp','void PrepareLightMap'),
            anchor('src/core/lightmap.cpp','static void SampleLightMapRow'),
            anchor('src/core/colorizer.cpp','PrepareLightMap(Lvals->BGLightMap')],
        numerical=numeric,images=images,binary_identity=binary_identity,
        finite_classes=[
            dict(id='preprocessing',domain='RGB8/RGB16 × smooth0/1 × spherical0/1 × border0/1',
                 classes=16,measured_dimensions=[[8,8],[32,16]],fixtures=32,gate='exact entire allocated map buffer hash'),
            dict(id='lighting-spline',domain='RGB8/RGB16 × wrap0/1/2 × linear/squared',
                 classes=12,coordinate_pairs_per_class=64,fixtures=768,gate='exact promoted Single output channels',
                 boundaries='below0,0,near0,interior,near1,1,above1; asymmetric extra seam column'),
            dict(id='formula-spline',domain='RGB8/RGB16, direct-coordinate wrapped callback',classes=2,
                 coordinate_pairs_per_class=36,fixtures=72,gate='exact Double output channels'),
            dict(id='border-source-defect',domain='width4/5 with border enabled; width6 valid boundary',
                 status='explicit rejection tested',reason='Original FitLMborder visits x0..5 and reads width-x-1 before the allocated map for width<6.',
                 preserved='width4/5 smoothing/spherical without border remain supported; lighting sampler width<=4 returns zero as original')],
        qualification_limits=[
            'Fixed x86 Pascal SSE2 sampler profile; finite values do not prove every real-valued coordinate or an alternative non-SSE2 numerical profile.',
            'Image checks distinguish exact RGB from existing tolerant PASS; no threshold changes.',
            'The two historical P5 image FAIL rows remain in render-maps-candidate-v13; original PNMReader accepts ASCII P2 only.',
            'Other image-decoder formats, animations and invalid PNM syntax are not certified by this map subset.',
            'A final changed renderer must replay these references; v15 results do not certify subsequent binaries.'],
        identities=identities)
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(dict(image_cases=sum(x['measured_cases'] for x in images),
        image_statuses=dict(Counter({k:sum(x['counts'].get(k,0) for x in images) for k in ('pass','fail')})),
        numerical_assertions=sum(x['assertions'] for x in numeric),
        numerical_failures=sum(x['failed'] for x in numeric))))


if __name__=='__main__':
    main()
