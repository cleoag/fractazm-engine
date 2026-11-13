"""Read-only preflight and present-day reference snapshot for ordinary final343.

The old sweep summaries recorded scene hashes, but not Pascal PNG hashes. This
tool explicitly distinguishes historical scene verification from PNG hashes
first observed at this preparation checkpoint. It never runs either renderer.
"""
import argparse
from collections import Counter
from datetime import datetime, timezone
import hashlib
import json
from pathlib import Path
import struct
import zlib

from PIL import Image

ROOT = Path(__file__).resolve().parents[2]
REFERENCE = ROOT/'comparison/runs/20261013T132245Z'
DEFAULT_OUT = ROOT/'comparison/results/stage-a-20261018/final343-reference-preflight-v1.json'
DEFERRED = {'jittrigexample', 'jitbenesipinetree', 'jit_iq_bulb'}


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--reference', type=Path, default=REFERENCE)
    ap.add_argument('--output', type=Path, default=DEFAULT_OUT)
    ap.add_argument('--verify-snapshot', type=Path,
                    help='Verify retained inputs against a prior preflight; do not rewrite it')
    args = ap.parse_args()
    if args.verify_snapshot:
        old = json.loads(args.verify_snapshot.read_bytes())
        for path, expected in old['input_sha256'].items():
            if digest(ROOT/path) != expected:
                raise ValueError('Reference preflight input changed: '+path)
        print(json.dumps(dict(verified_inputs=len(old['input_sha256']), cases=len(old['cases']))))
        return
    identities = {}
    def retain(path):
        path = path.resolve()
        label = path.relative_to(ROOT).as_posix()
        identities[label] = digest(path)
        return label
    reference = args.reference.resolve()
    manifest = json.loads((reference/'manifest.json').read_bytes())
    retain(reference/'manifest.json')
    summaries = sorted(reference.rglob('summary.json'))
    expected_summaries = {(ROOT/row['summary']).resolve() for row in manifest['suites']}
    if set(summaries) != expected_summaries or len(summaries) != 15:
        raise ValueError('Unexpected reference summary set')
    cases = []
    suites = []
    for path in summaries:
        label = retain(path)
        data = json.loads(path.read_bytes())
        if not data.get('ended') or data['threads'] != 16:
            raise ValueError('Reference completion/thread contract changed: '+label)
        if data['provenance']['pascal_sha256'] != manifest['binaries']['pascal']['sha256']:
            raise ValueError('Inconsistent original Pascal executable identity')
        suite = path.parent.parent.name
        rows = data['results']
        suites.append(dict(suite=suite, cases=len(rows), original_counts=dict(Counter(row['status'] for row in rows)),
                           summary=label, historical_cpp_seconds=sum(row.get('cpp_ms',0) for row in rows)/1000))
        for row in rows:
            scene = Path(data['provenance']['scene_dir'])/(row['scene']+'.m3p')
            scene_label = retain(scene)
            if identities[scene_label] != row.get('scene_sha256'):
                raise ValueError('Scene differs from original sweep: '+scene_label)
            raw = scene.read_bytes()
            try:
                raw = zlib.decompress(raw)
            except zlib.error:
                pass
            if len(raw) != 1976 or struct.unpack_from('<i', raw)[0] != 44:
                raise ValueError('Preflight decoder needs an explicit legacy-layout extension')
            names = [raw[860+188*i:892+188*i].split(b'\0')[0].decode('latin1') for i in range(6)]
            if DEFERRED.intersection(name.casefold() for name in names):
                raise ValueError('Deferred formula in ordinary final343: '+scene_label)
            image = path.parent/'pascal'/(row['scene']+'.png')
            image_label = retain(image)
            if row.get('pascal_image_sha256') and identities[image_label] != row['pascal_image_sha256']:
                raise ValueError('Pascal image differs from original committed hash')
            with Image.open(image) as picture:
                dimensions = list(picture.size)
                picture.verify()
            cases.append(dict(id=suite+':'+row['scene'], suite=suite, scene=row['scene'],
                scene_path=scene_label, historical_scene_hash_verified=True,
                pascal_image=image_label, dimensions=dimensions,
                image_hash_provenance='historical_summary' if row.get('pascal_image_sha256') else 'first_recorded_by_this_preflight',
                formula_slot_names=names, historical_status=row['status']))
    if len(cases) != 343 or len({row['id'] for row in cases}) != 343:
        raise ValueError('Final343 denominator or unique suite/scene identity changed')
    retain(ROOT/'comparison/tools/replay_parity.py')
    retain(ROOT/'comparison/tools/sweep.py')
    retain(Path(__file__))
    report = dict(schema_version=1, prepared_utc=datetime.now(timezone.utc).isoformat(),
        scope='343 ordinary deterministic image comparisons, 16 logical threads; Monte Carlo/JIT excluded',
        renderer_executed=False, reference=str(reference),
        original_pascal_identity=manifest['binaries']['pascal'],
        historical_pascal_source_manifest_sha256=manifest['pascal_source_manifest_sha256'],
        suites=suites, cases=cases, input_sha256=identities,
        totals=dict(cases=343, suites=15, historical_scene_hash_verified=343,
            original_image_hashes=sum(row['image_hash_provenance']=='historical_summary' for row in cases),
            readable_pngs=343, deferred_formula_names=0,
            historical_cpp_seconds=sum(row['historical_cpp_seconds'] for row in suites)),
        limitations=['Present-day image hash capture cannot retroactively establish an original historical image hash.',
            'No new candidate has run; historical PASS and timings do not certify its result or duration.',
            'This ordinary regression corpus is separate from active5506 and additional Stage A mode/state witnesses.'],
        planned_command=['python','-X','utf8','comparison/tools/replay_parity.py','--reference',str(reference),
            '--cpp-exe','<frozen-final-candidate>/mb3d_render.exe','--output',
            'comparison/runs/stage-a-20261018/final343-candidate-v18','--checkpoint-every','16'])
    for path, expected in identities.items():
        if digest(ROOT/path) != expected:
            raise ValueError('Reference changed during preflight: '+path)
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(report, indent=2)+'\n')
    print(json.dumps(report['totals']))


if __name__ == '__main__':
    main()
