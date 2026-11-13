"""Reuse a captured native CPU comparison, never GPU/fallback qualification.

The CPU reference keeps its own executable identity. The requested scene,
worker topology, effects and complete resource bytes must still match.
"""
from pathlib import Path
from qualify_native_metal_all import artifact, contained, digest, read_json, require, sha
from gpu_pipeline_binding import runtime_files
from gpu_pipeline_proofs import binary_identity
from gpu_pipeline_fp32 import frame as cpu_frame


def previous_slow_reference(base, package, scene):
    """An earlier slow frame blocks a blind repeat even if not reusable evidence."""
    if package is None:
        return None
    path = contained(package, 'scene-runs/'+scene['id']+'/reference-cpu/event.json')
    if not path.is_file():
        case_path = contained(package, 'scene-runs/'+scene['id']+'/case.json')
        if not case_path.is_file():
            return None
        case = read_json(case_path)
        retained = case.get('previous_slow_cpu')
        if not retained:
            return None
        require(case['id'] == scene['id'], 'Wrong retained deadline scene')
        path = artifact(base, retained['event'])
        event = read_json(path)
        require(any(r['sha256'] == scene['sha256'] for r in event.get('inputs', [])),
                'Retained deadline does not bind this scene')
    event = read_json(path)
    if event.get('seconds', 0) <= 180 and not event.get('frame_warning_emitted'):
        return None
    return {'event': {'path': str(path.relative_to(base)), 'sha256': sha(path)},
            'seconds': event.get('seconds'), 'qualification_credit': False}


def reference_binding(base, package):
    """Keep original CPU capture provenance when the aggregate manifest advances."""
    pin=Path(package)/'cpu-reference-binding.json'
    if not pin.exists():return read_json(Path(package)/'evidence.json')['binding']
    record=read_json(pin)
    require(record.get('schema')=='mb3d.native-cpu-reference-binding.v1','Unknown CPU reference binding')
    snapshot=read_json(artifact(base,record['manifest']))
    binding=snapshot['binding']
    require(digest(binding)==record['binding_sha256'],'CPU reference binding digest changed')
    return binding


def saved_reference(base, package, scene, ctx, _seen=frozenset()):
    package = Path(package).resolve()
    require(package.is_relative_to(Path(base).resolve()) and package not in _seen,
            'Foreign or cyclic CPU reference package')
    require(len(_seen) < 8, 'CPU reference chain too deep')
    folder = contained(package, 'scene-runs/'+scene['id']+'/reference-cpu')
    paths = {k: folder/name for k, name in
             (('event', 'event.json'), ('profile', 'profile.json'), ('image', 'image.png'))}
    if not any(p.exists() for p in paths.values()):
        case_path = folder.parent/'case.json'
        if not case_path.is_file():
            return None
        case = read_json(case_path)
        source = case.get('cpu_reference_origin')
        if not source or not case.get('cpu_reference'):
            return None
        require(case['id'] == scene['id'] and source['kind'] == 'REUSED_NATIVE_CPU_REFERENCE',
                'Wrong inherited CPU reference')
        prior = saved_reference(base, contained(base, source['package']), scene, ctx, _seen | {package})
        require(prior is not None and prior[0] == case['cpu_reference'] and
                prior[1]['renderer'] == source['renderer'], 'Inherited reference does not match its sealed source')
        return prior
    require(all(p.is_file() for p in paths.values()), 'Incomplete saved CPU reference')
    binding = reference_binding(base,package)
    event = read_json(paths['event'])
    # Earlier captures did not bind resource bytes. Keep them as historical
    # observations, but do not silently upgrade them to reusable references.
    if any(key not in event for key in ('resource_files_before', 'resource_files_after')):
        return None
    runtime = Path(event['cwd']).resolve()
    allowed = {contained(base, v['runtime']) for v in binding['resource_profiles'].values()}
    require(runtime in allowed, 'CPU reference runtime is not in its pinned source binding')
    require(runtime_files(runtime) == ctx['runtime_files'], 'CPU reference resources differ')
    for key in ('resource_files_before', 'resource_files_after'):
        require(event.get(key) == ctx['runtime_files'], 'CPU reference did not capture matching resources')
    for name, expected in ctx['runtime_files'].items():
        path = runtime/name
        require(any(artifact(base, r) == path and r['sha256'] == expected for r in event['inputs']),
                'CPU reference resource not bound to process')
    reference_ctx = dict(ctx, runtime=runtime, renderer=binary_identity(base, binding['renderer']))
    row = {k: {'path': str(p.relative_to(base)), 'sha256': sha(p)} for k, p in paths.items()}
    cpu_frame(base, row, scene, reference_ctx, fallback=False, reference_only=True)
    origin = {'kind': 'REUSED_NATIVE_CPU_REFERENCE', 'package': str(package.relative_to(base)),
              'renderer': binding['renderer'], 'collection_binding_sha256': digest(binding),
              'runtime': str(runtime.relative_to(base)), 'runtime_files': ctx['runtime_files'],
              'qualification_credit': False}
    return row, origin
