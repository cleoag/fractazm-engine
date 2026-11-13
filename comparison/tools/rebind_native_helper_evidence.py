#!/usr/bin/env python3
"""Replay a partial package after explicitly reviewed Python helper changes.

No renderer, shader, scene, resource, process event or frozen package is changed.
Compiler inputs and artifact hashes must still match. A new package receives
new receipts only after ordinary, formula and full-scene validators replay the
original observations. This supports the currently implemented partial proof
types, not arbitrary historical global proofs.
"""
import argparse
import copy
import json
from pathlib import Path
import shutil
import subprocess
import sys

from qualify_native_metal_all import ROOT, artifact, require, sha, source_snapshot
from prepare_gpu_parallel_bundle import ref, refresh, write
from qualify_gpu_parallel_pipeline import evaluate
from verify_native_formula_group import admit, validate_build_products


def read(path):
    return json.loads(Path(path).read_text())


def helper_changes(before, after, allowed):
    changed = {p for p in before.keys() | after.keys() if before.get(p) != after.get(p)}
    require(changed and changed == set(allowed), 'Unreviewed or absent helper changes')
    require(all(Path(p).parts[:2] == ('comparison', 'tools') and
                Path(p).suffix == '.py' and '..' not in Path(p).parts for p in changed),
            'Only explicitly reviewed Python verification/report helpers may be rebound')
    return {p: {'before': before.get(p), 'after': after.get(p)} for p in sorted(changed)}


def distinct_packages(source, target):
    require(not source.is_relative_to(target) and not target.is_relative_to(source),
            'Source and target packages must be separate, without nesting')


def preserve_rebound_manifest(target):
    # Later admission may append scene/formula rows to evidence.json. Keep the
    # exact rebind-stage bytes at an immutable path for this replay receipt.
    snapshot = target/'rebind-stage-evidence.json'
    require(not snapshot.exists(), 'Rebind snapshot already exists')
    shutil.copyfile(target/'evidence.json', snapshot)
    return snapshot


def compiled_inputs_unchanged(build):
    receipt_path = build.parent / 'build-receipt.json'
    validate_build_products(receipt_path, build/'mb3d_native_formula_probe',
                            build/'metal_native_scene.metallib')
    receipt = read(receipt_path)
    require(sha(build.parent/'source-manifest.json') == receipt['source_manifest_sha256'],
            'Captured source manifest changed')
    seen = {}
    for row in receipt['records']:
        record = read(build.parent/row['path'])
        for item in record.get('dependencies', []) + record.get('explicit_inputs_before', []):
            path = Path(item['path']).resolve()
            if path.is_relative_to(ROOT) and not path.is_relative_to(build):
                if path not in seen:
                    seen[path] = sha(path)
                require(seen[path] == item['sha256'], 'Actual compiler input changed: ' + str(path))
    return len(seen)


def rebind(source, target, contract, build, ordinary_capture, allowed):
    source, target, contract, build, ordinary_capture = (
        Path(p).resolve() for p in (source, target, contract, build, ordinary_capture))
    require(all(p.is_relative_to(ROOT) for p in (source, target, contract, build, ordinary_capture)) and
            source != target and not target.exists(), 'Use a new repository package')
    distinct_packages(source, target)
    original = read(source/'evidence.json')
    previous = read(source/'qualification.json')
    before_hash = sha(source/'evidence.json')
    snapshot = source_snapshot(ROOT)
    changes = helper_changes(original['binding']['source'], snapshot, allowed)
    require(original['binding']['contract_sha256'] == sha(contract), 'Changed acceptance contract')
    require(original['binding']['renderer'] == ref(build/'mb3d_render') and
            original['binding']['library'] == ref(build/'metal_native_scene.metallib'),
            'Different compiled artifacts')
    dependency_count = compiled_inputs_unchanged(build)
    require([p['kind'] for p in original['proofs']] == ['cpu_ordinary'],
            'Unsupported global proof type; explicit new verification required')
    groups = {}
    for row in original['formulas']:
        proof = read(artifact(ROOT, row['proof']))
        require(proof['producer'] == 'verify_native_formula_group.py' and
                proof['kind'] == 'formula_gpu_point_orbit', 'Unsupported formula proof producer')
        collection, review = (artifact(ROOT, r) for r in proof['artifacts'][:2])
        require(read(collection)['schema'] == 'mb3d.native-formula-group.v1', 'Wrong formula collection')
        groups[str(collection)] = review
    resources = read(source/'resources.json')
    require(resources['schema'] == 'mb3d.prepared-scene-resources.v1', 'Prepared profiles required')
    target.mkdir()
    # Share immutable resource bytes; resolved cwd remains exactly that captured
    # in the original frame events. The manifests and receipts remain distinct.
    for profile in resources['profiles'].values():
        relative = Path(profile['runtime'])
        require(len(relative.parts) == 1 and relative.name not in ('.', '..'), 'Unsafe runtime alias')
        (target/relative).symlink_to((source/relative).resolve(), target_is_directory=True)
    shutil.copyfile(source/'resources.json', target/'resources.json')
    rebuilt = copy.deepcopy(original)
    rebuilt['formulas'], rebuilt['proofs'] = [], []
    write(target/'evidence.json', rebuilt)
    refresh(target, contract, build)
    command = [sys.executable, '-B', str(ROOT/'comparison/tools/admit_native_ordinary_proof.py'),
               '--repository', str(ROOT), '--package', str(target), '--contract', str(contract),
               '--build', str(build), '--capture', str(ordinary_capture)]
    subprocess.run(command, check=True, timeout=180)
    for collection, review in sorted(groups.items()):
        admit(Path(collection), review, target, contract)
    result = evaluate(contract, target)
    write(target/'qualification.json', result)
    require(sorted(result['accepted']) == sorted(previous['accepted']) and
            result['remaining'] == previous['remaining'], 'Replayed scope differs from sealed package')
    for row in original['scenes']:
        prior = source/'scene-runs'/row['id']/'case.json'
        case = read(prior)
        require(case['status'] == 'QUALIFIED_GPU' and case['frames'] == row['frames'],
                'Scene metadata disagrees with accepted observations')
        case['revalidated_from'] = {'manifest': ref(source/'evidence.json'), 'case': ref(prior)}
        write(target/'scene-runs'/row['id']/'case.json', case)
    require(sha(source/'evidence.json') == before_hash and source_snapshot(ROOT) == snapshot,
            'Frozen source package or current source changed during replay')
    sealed_manifest = preserve_rebound_manifest(target)
    report = {'status': 'PASS_REVALIDATED_PARTIAL', 'qualification': False,
              'full_gate': result['status'], 'remaining': result['remaining'],
              'original_manifest': ref(source/'evidence.json'), 'new_manifest': ref(sealed_manifest),
              'working_manifest_path': str((target/'evidence.json').relative_to(ROOT)),
              'reviewed_helper_changes': changes, 'compiled_inputs_rechecked': dependency_count,
              'gpu_processes_launched': 0, 'scope': 'Same artifacts and raw observations, freshly replayed receipts'}
    write(target/'rebind.json', report)
    return report


def main():
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    for name in ('source', 'target', 'contract', 'build', 'ordinary-capture'):
        parser.add_argument('--'+name, type=Path, required=True)
    parser.add_argument('--allow-helper', action='append', required=True)
    args = parser.parse_args()
    result = rebind(args.source, args.target, args.contract, args.build,
                    args.ordinary_capture, args.allow_helper)
    print(result['status'], result['full_gate'], result['remaining'])


if __name__ == '__main__':
    main()
