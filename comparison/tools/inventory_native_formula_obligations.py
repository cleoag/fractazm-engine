#!/usr/bin/env python3
"""Inventory original defaults and original-settings host contexts (no rendering).

The output is INVENTORIED, never AUDITED or numerical evidence. Prepared Native
bytes describe a candidate host, not an original numerical oracle. Their ABI
requires an explicit adapter before it can be mapped to point/orbit probe cases.
"""
import argparse
import hashlib
import importlib.util
import json
from pathlib import Path
import re
import shutil
import shlex
import struct
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[2]
CONTRACT = Path('comparison/contracts/native-gpu-acceptance-v4/acceptance-contract.json')
QUEUE = Path('comparison/results/formula-restart-20261104/queue.json')
CATEGORIES = ('source_semantics', 'parameters_preparation', 'authored_defaults',
              'scene_contexts', 'branches_boundaries', 'state', 'callers_hybrids',
              'rounded_finite', 'resources', 'range_faults')
PREPARATION_SOURCES = ('mb3d/CustomFormulas.pas', 'mb3d/HeaderTrafos.pas',
                       'mb3d/Math3D.pas', 'mb3d/TypeDefinitions.pas', 'mb3d/formulas.pas')


def require(ok, message):
    if not ok:
        raise ValueError(message)


def digest(data):
    return hashlib.sha256(data).hexdigest()


def canonical(value):
    return json.dumps(value, sort_keys=True, separators=(',', ':'), ensure_ascii=False).encode()


def ref(path, root=ROOT):
    path = Path(path).resolve()
    return {'path': str(path.relative_to(root.resolve())), 'sha256': digest(path.read_bytes())}


def read(path):
    return json.loads(Path(path).read_text())


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, indent=2, ensure_ascii=False, allow_nan=False)+'\n')
    return ref(path)


def words64(values):
    return [f'0x{struct.unpack("<Q", struct.pack("<d", v))[0]:016x}' for v in values]


def pascal_code(text):
    """Remove Pascal comments without mistaking strings for comment delimiters."""
    token = re.compile(r"'(?:''|[^'])*'|\{[^}]*\}|\(\*.*?\*\)|//[^\n]*", re.S)
    return token.sub(lambda m: m[0] if m[0].startswith("'") else
                     ''.join('\n' if c == '\n' else ' ' for c in m[0]), text)


def sections(raw):
    """Exact raw section bodies and ranges, never normalized CODE bytes."""
    markers = list(re.finditer(rb'^\[([^]\r\n]+)\][ \t]*\r?$', raw, re.M))
    result = {}
    for i, marker in enumerate(markers):
        end = markers[i+1].start() if i+1 < len(markers) else len(raw)
        name = marker[1].decode('ascii')
        start = marker.end()
        require(name not in result, 'Duplicate M3F section '+name)
        result[name] = {'byte_start': start, 'byte_end': end,
                        'line_start': raw[:start].count(b'\n')+1,
                        'sha256': digest(raw[start:end]),
                        'text': raw[start:end].decode('latin1')}
    return result


def external_defaults(identity, root=ROOT):
    source = root/'mb3d/M3Formulas'/(identity['name']+'.m3f')
    raw = source.read_bytes()
    parts = sections(raw)
    require('CODE' in parts and 'OPTIONS' in parts, 'Missing original CODE/OPTIONS')
    parser_path = root/'scripts/gen_bulk_formula_table.py'
    module_spec = importlib.util.spec_from_file_location('original_options_extractor', parser_path)
    module = importlib.util.module_from_spec(module_spec)
    module_spec.loader.exec_module(module)
    parsed = module.parse_m3f_options(str(source))
    options = parts['OPTIONS']['text']
    # The existing generator's absent RStop=100 is not the original loader's
    # fallback. Preserve original CustomFormulas.pas initialization (16).
    if not re.search(r'^\s*\.rstop\s*=', options, re.I | re.M):
        parsed['rstop'] = 16.0
    types = []
    for kind in parsed['option_types']:
        types.extend([kind]*{5: 3, 6: 3, 12: 6}.get(kind, 1))
        if kind == 18:
            types.extend([6]*3)
        elif kind == 19:
            types.append(6)
    require(len(types) == len(parsed['option_defaults']) <= 16, 'Expanded option arity')
    declarations = [{'line': parts['OPTIONS']['line_start']+n, 'literal': line}
                    for n, line in enumerate(options.splitlines()) if line.strip()]
    return {'schema': 'mb3d.original-formula-defaults.v1', 'identity': identity,
            'status': 'INVENTORIED', 'source': ref(source, root), 'sections': parts,
            'raw_option_declarations': declarations,
            'authored_option_types': types,
            'authored_values_binary64_hex': words64(parsed.pop('option_defaults')),
            'parsed_metadata': parsed,
            'extraction_helper': ref(parser_path, root),
            'preparation_sources': [ref(root/p, root) for p in PREPARATION_SOURCES],
            'limitations': ['Authored binary64 defaults are not prepared PVar values.',
                           'Decimal-to-binary64 extraction and typed preparation require per-formula review.',
                           'Raw original sections are authoritative; no semantic completeness or N/A approval.']}


def internal_defaults(identity, root=ROOT):
    header_path, formulas_path = root/'mb3d/HeaderTrafos.pas', root/'mb3d/formulas.pas'
    header = pascal_code(header_path.read_text(encoding='latin1'))
    formulas = pascal_code(formulas_path.read_text(encoding='latin1')).split('implementation')[0]
    match = re.search(r'oa:\s*array\[0\.\.9,\s*0\.\.1\].*?=\s*\((.*?)\);', header, re.S | re.I)
    require(match is not None, 'Missing internal default table')
    pairs = re.findall(r'\(([^()]*)\)', match[1])
    require(len(pairs) == 10, 'Internal default table shape')
    n = identity['id']
    values = [float(v) for v in pairs[n].split(',')]+[0.0]*14
    types = [0]*16
    count = 2
    if n in (0, 6):
        types[0] = 10
    if n in (4, 5):
        types[1] = 7
    if n == 4:
        count, values[2], types[2] = 3, 1., 11
    elif n == 5:
        count = 6
        values[2:6], types[2:6] = [1., 1., 2., 2.], [11, 0, 9, 9]
    elif n == 6:
        count, values[2], types[2] = 3, 2., 8
    elif n in (7, 8):
        prefix = 'testhybrid' if n == 7 else 'testIFS'
        def literal(suffix, is_array):
            pattern = prefix+suffix+r'\s*:[^;=]*=\s*'+(r'\((.*?)\)' if is_array else r'([^;]+)')+r'\s*;'
            matches = re.findall(pattern, formulas, re.S | re.I)
            require(len(matches) == 1, 'Ambiguous active internal constant '+prefix+suffix)
            return matches[0]
        count = int(literal('OptionCount', False))
        active_types = [int(v) for v in literal('OptionTypes', True).split(',')]
        active_values = [float(v) for v in literal('OptionVals', True).split(',')]
        require(len(active_types) == len(active_values) == count, 'Internal active option arity')
        types[:count], values[:count] = active_types, active_values
    elif n == 9:
        count = 8
        values[2:6] = [1., 1., 8., 1.]
        for index in (2, 3, 6, 7):
            types[index] = 2
    return {'schema': 'mb3d.original-formula-defaults.v1', 'identity': identity,
            'status': 'INVENTORIED', 'option_count': count,
            'authored_option_types': types[:count],
            'authored_values_binary64_hex': words64(values[:count]),
            'all16_authored_types': types, 'all16_authored_values_binary64_hex': words64(values),
            'preparation_sources': [ref(root/p, root) for p in PREPARATION_SOURCES],
            'source_locations': ['HeaderTrafos.GetHAddOnFromInternFormula',
                                 'CustomFormulas.SetCFoptionsFromOldF', 'CustomFormulas.ParseCFfromOld',
                                 'formulas active testhybrid/testIFS constants and dispatched procedures'],
            'limitations': ['Source/default and preparation semantic completeness review remains OPEN.',
                           'No original M3F exists for internal formulas; original Pascal procedures are authoritative.']}


def loaded_slots(raw, identities):
    require(len(raw) == 1136, 'Loaded addon ABI size')
    result = []
    external = {f['name'].casefold(): f['id'] for f in identities if f['namespace'] == 'external'}
    for i in range(6):
        offset = 8+188*i
        count, number, arity = struct.unpack_from('<iii', raw, offset)
        name = raw[offset+12:offset+44].split(b'\0')[0].decode('latin1')
        namespace = 'internal' if 0 <= number <= 9 else ('external' if number >= 20 else 'unresolved')
        formula_id = number if namespace == 'internal' else external.get(name.casefold())
        require(count == 0 or formula_id is not None, 'Unresolved active loaded formula '+name)
        result.append({'slot': i, 'iterations': count, 'formula_number': number, 'id': formula_id,
                       'namespace': namespace, 'name': name,
                       'option_count': arity, 'option_types': list(raw[offset+44:offset+60]),
                       'option_values_binary64_hex': [f'0x{v:016x}' for v in struct.unpack_from('<16Q', raw, offset+60)]})
    return result


def verify_original_settings(scene, summary, slots):
    require(summary['host_only'] is True and summary['settings_overrides'] == [] and
            summary['precision'] == 'native', 'Not original-settings host-only preparation')
    require([summary['width'], summary['height']] == scene['settings']['size'], 'Scene dimensions changed')
    require(summary['original_iterations'] == scene['settings']['iterations'], 'Scene iteration limit changed')
    require(summary['programs'][0]['max_iterations'] == scene['settings']['iterations'],
            'Prepared primary iteration limit changed')
    if len(summary['programs']) == 2:
        require(summary['programs'][1]['max_iterations'] == summary['original_secondary_iterations'],
                'Prepared secondary iteration limit changed')
    by_slot = {s['slot']: s for s in slots}
    for declared in scene['slots']:
        actual = by_slot[declared['slot']]
        require(f"{actual['namespace']}:{actual['id']}" == declared['key'], 'Scene formula identity changed')
        for field in ('iterations', 'option_count', 'option_types', 'option_values_binary64_hex'):
            require(actual[field] == declared[field], 'Scene authored '+field+' changed')


def context_identity(parameters, programs, resources, settings):
    # settings binds camera/effects/state-preparation; it is never discarded just
    # because the same formula parameter tuple occurs in two different scenes.
    return digest(canonical({'parameters_sha256': parameters, 'program_sha256': programs,
                             'resources_sha256': resources, 'settings_sha256': settings}))


def prepare_scene(scene, dump, run, resource, build_receipt, identities):
    sid, folder = scene['id'], run/scene['id']
    require(shutil.disk_usage(run).free >= 12*1024**3, 'Disk reserve below 12 GiB')
    require(digest((ROOT/scene['path']).read_bytes()) == scene['sha256'], 'Original scene changed')
    command = [str(dump), str(ROOT/scene['path']), str(resource['root']), str(folder)]
    process = subprocess.run(command, capture_output=True, timeout=180)
    (run/(sid+'.stdout')).write_bytes(process.stdout)
    (run/(sid+'.stderr')).write_bytes(process.stderr)
    require(process.returncode == 0, 'Host preparation failed '+sid+': '+process.stderr.decode(errors='replace'))
    summary = json.loads(process.stdout)
    slots = loaded_slots((folder/'loaded-addon.bin').read_bytes(), identities)
    verify_original_settings(scene, summary, slots)
    artifacts = {p.name: ref(p) for p in sorted(folder.glob('*.bin'))}
    names = ['primary']+(['secondary'] if summary['program_count'] == 2 else [])
    parameter_hash = digest(b''.join((folder/(n+'-parameters.bin')).read_bytes() for n in names))
    program_hash = digest(b''.join((folder/(n+'.bin')).read_bytes() for n in names))
    resource_record = {k: v for k, v in resource.items() if k != 'root'}
    resource_record['decoded_maps'] = [artifacts[n] for n in ('maps.bin', 'map-pixels.bin')]
    # Content-only hashes: file locations must not alter deduplication identity.
    resources_hash = digest(canonical({'profile': resource['profile'], 'files': resource['files'],
        'decoded_maps': {n: artifacts[n]['sha256'] for n in ('maps.bin', 'map-pixels.bin')}}))
    settings_names = [n for n in artifacts if not n.startswith(('primary', 'secondary')) and n not in ('maps.bin', 'map-pixels.bin')]
    settings_hash = digest(canonical({n: artifacts[n]['sha256'] for n in settings_names}))
    prepared_links = []
    for pi, p in enumerate(summary['programs']):
        for s in p['slots']:
            if s['enabled']:
                prepared_links.append({'program': names[pi], 'slot': s['slot'],
                                       'key': ('external' if s['namespace'] else 'internal')+':'+str(s['id']),
                                       'count': s['count']})
    keys = {s['key'] for s in prepared_links}
    capability = 'combined' if len(names) == 2 else ('mixed' if len(prepared_links) > 1 else 'single_slot')
    return {'id': context_identity(parameter_hash, program_hash, resources_hash, settings_hash),
            'representation': 'native-prepared-scene-v1', 'status': 'INVENTORIED',
            'adapter_status': 'REQUIRED_OPEN', 'capability': capability,
            'parameters_sha256': parameter_hash, 'program_sha256': program_hash,
            'resources_sha256': resources_hash, 'settings_sha256': settings_hash,
            'bundle_layout': {'program': 'raw 8192-byte primary then optional secondary',
                              'parameters': 'for each program, six 1280-byte NativeFormulaSlot suffixes at offset48'},
            'scene_ids': [sid], 'scene_links': [{'scene_id': sid,
                'authored_slots': scene['slots'], 'prepared_slots': prepared_links}],
            'formula_keys': sorted(keys | {s['key'] for s in scene['slots'] if s['iterations'] != 0}),
            'loaded_slots': slots, 'prepared_summary': summary,
            'resource_context': resource_record, 'artifacts': list(artifacts.values()),
            'program_artifacts': [artifacts[n+'.bin'] for n in names],
            'parameter_artifacts': [artifacts[n+'-parameters.bin'] for n in names],
            'host': ref(dump), 'build_receipt': ref(build_receipt),
            'command': command, 'stdout': ref(run/(sid+'.stdout')), 'stderr': ref(run/(sid+'.stderr'))}


def deduplicate(contexts):
    indexed = {}
    for row in contexts:
        if row['id'] not in indexed:
            indexed[row['id']] = row
        else:
            previous = indexed[row['id']]
            require(all(previous[k] == row[k] for k in ('parameters_sha256', 'program_sha256',
                    'resources_sha256', 'settings_sha256', 'formula_keys', 'capability')), 'Context collision')
            previous['scene_ids'] += row['scene_ids']
            previous['scene_links'] += row['scene_links']
            previous.setdefault('equivalent_artifact_instances', []).append(row)
    return list(indexed.values())


def validate_host_build(dump, receipt_path):
    """Replay captured command/producer/dependency closure, not receipt claims."""
    receipt_path, dump = Path(receipt_path).resolve(), Path(dump).resolve()
    receipt = read(receipt_path)
    require(receipt.get('schema') == 'mb3d.host-scene-context.build.v1' and
            receipt.get('status') == 'PASS_CAPTURED_BUILD_CLOSURE' and receipt.get('host_only') is True,
            'Captured host-only build receipt required')
    require(receipt.get('targets') == ['mb3d_native_scene_context_dump'], 'Wrong host-only build target')
    base, build = receipt_path.parent, receipt_path.parent/'build'
    cache = {}
    def checked(path, expected, label):
        path = Path(path).resolve()
        if path not in cache:
            require(path.is_file(), 'Missing '+label+': '+str(path))
            cache[path] = digest(path.read_bytes())
        require(cache[path] == expected, 'Changed '+label+': '+str(path))
    checked(base/'source-manifest.json', receipt['source_manifest_sha256'], 'host source manifest')
    manifest = read(base/'source-manifest.json')
    source = Path(receipt['compiled_source_root']).resolve()
    require(source == (base/'source').resolve(), 'Wrong frozen source root')
    for relative, expected in manifest['source_files'].items():
        path = (source/relative).resolve()
        require(path.is_relative_to(source), 'Source manifest path escape')
        checked(path, expected, 'frozen source')
    checked(base/'provenance/capture.py', receipt['capture_sha256'], 'host build capture')
    require(manifest['capture_sha256'] == receipt['capture_sha256'], 'Capture manifest differs')
    context_path = base/'provenance/context.json'
    context = read(context_path)
    record_dir = base/'provenance/records'
    require(Path(context['root']).resolve() == source and Path(context['build']).resolve() == build and
            Path(context['records']).resolve() == record_dir, 'Build capture context differs')
    records = []
    require(receipt.get('records'), 'Missing build command records')
    listed = set()
    process_keys = set()
    for binding in receipt['records']:
        path = (base/binding['path']).resolve()
        require(path.parent == record_dir and path.suffix == '.json' and
                not path.name.endswith('.started.json') and path not in listed, 'Invalid or duplicate build record')
        listed.add(path); checked(path, binding['sha256'], 'host build command')
        command = read(path)
        require(command.get('schema') == 'mb3d.actual-build-command.v1', 'Invalid command schema')
        require(command.get('exit_code') == 0 and command.get('capture_errors') == [], 'Failed command or capture')
        require(type(command.get('pid')) is int and command['pid'] > 0, 'Invalid build process pid')
        require(command.get('kind') in ('compile', 'link', 'custom') and
                type(command.get('start_time_ns')) is int and type(command.get('end_time_ns')) is int and
                command['start_time_ns'] <= command['end_time_ns'], 'Invalid command interval')
        process_key = (command['pid'], command['start_time_ns'])
        require(process_key not in process_keys, 'Duplicate build process identity')
        process_keys.add(process_key)
        checked(context_path, command['context_sha256'], 'command context')
        checked(command['tool']['path'], command['tool']['sha256'], 'compiler/link tool')
        started = path.with_name(path.stem+'.started.json')
        require(started.is_file(), 'Missing started command counterpart')
        before = read(started)
        require(before.get('schema') == 'mb3d.actual-build-command.v1' and
                {'kind', 'argv', 'expanded_argv', 'cwd', 'pid', 'start_time_ns', 'context_sha256',
                 'tool', 'explicit_inputs_before', 'output_existed_before'} <= set(before),
                'Incomplete started command record')
        require(all(k == 'started_monotonic' or command.get(k) == value for k, value in before.items()),
                'Started command differs from completed command')
        cwd = Path(command['cwd']).resolve()
        argv = command['argv']; expanded = []
        require(isinstance(argv, list) and argv and all(isinstance(v, str) for v in argv), 'Invalid command argv')
        for value in argv:
            expanded += shlex.split((cwd/value[1:]).read_text()) if value.startswith('@') else [value]
        require(expanded == command['expanded_argv'], 'Expanded command argv differs')
        require(Path(expanded[0]).resolve() == Path(command['tool']['path']).resolve(), 'Command tool differs')
        tool = Path(expanded[0]).name
        output = ((cwd/expanded[expanded.index('-o')+1]).resolve() if '-o' in expanded else
                  (cwd/expanded[2]).resolve() if tool in ('ar', 'llvm-ar') else
                  (cwd/expanded[-1]).resolve() if tool in ('ranlib', 'llvm-ranlib') else None)
        require(output is not None and output.is_relative_to(build) and
                output == Path(command['output']['path']).resolve(), 'Command output is not actual producer')
        require(output.suffix not in ('.air', '.metallib'), 'Unexpected AIR/Metal product')
        if tool not in ('ranlib', 'llvm-ranlib'):
            require(command['output_existed_before'] is False, 'Nonfresh compile/archive/link product')
        dep_output = (cwd/expanded[expanded.index('-MF')+1]).resolve() if '-MF' in expanded else None
        explicit = {(cwd/value[1:]).resolve() for value in argv if value.startswith('@')}
        explicit |= {(cwd/value).resolve() for value in expanded[1:] if not value.startswith('-') and
                     (cwd/value).is_file() and (cwd/value).resolve() not in (output, dep_output)}
        require(explicit == {Path(v['path']).resolve() for v in command['explicit_inputs_before']},
                'Missing or extra explicit command input')
        if output.suffix == '.o':
            require(command.get('dependencies') and command.get('depfile'), 'Missing object dependency closure')
            depfile = Path(command['depfile']['path'])
            checked(depfile, command['depfile']['sha256'], 'object dependency file')
            logical = depfile.read_text().replace('\\\n', '')
            line = next((v for v in logical.splitlines() if v.strip()), '')
            split = next((i for i, c in enumerate(line) if c == ':' and (i == 0 or line[i-1] != '\\')), None)
            require(split is not None, 'Invalid dependency rule')
            dep_paths = {(cwd/token.replace('$$', '$')).resolve() for token in shlex.split(line[split+1:])}
            require(dep_paths == {Path(v['path']).resolve() for v in command['dependencies']},
                    'Dependency list differs from compiler depfile')
        records.append(command)
    actual = {p.resolve() for p in record_dir.glob('*.json') if not p.name.endswith('.started.json')}
    starts = {p.with_name(p.name[:-len('.started.json')]+'.json').resolve() for p in record_dir.glob('*.started.json')}
    require(listed == actual == starts, 'Unlisted or interrupted build command')
    latest = {}; edges = objects = dependencies = 0
    for command in records:
        if Path(command['expanded_argv'][0]).name in ('ranlib', 'llvm-ranlib'):
            require(command['output_existed_before'] is True and any(
                r['output']['path'] == command['output']['path'] and
                r['end_time_ns'] <= command['start_time_ns'] and
                Path(r['expanded_argv'][0]).name in ('ar', 'llvm-ar') for r in records),
                'Archive index has no preceding archive producer')
        for item in command['explicit_inputs_before']:
            path = Path(item['path']).resolve()
            if path.is_relative_to(build) and path.suffix in ('.o', '.a'):
                producers = [r for r in records if Path(r['output']['path']).resolve() == path and
                             r['end_time_ns'] <= command['start_time_ns']]
                require(producers, 'Missing preceding object/archive producer')
                producer = max(producers, key=lambda r: r['end_time_ns'])
                require(producer['output']['sha256'] == item['sha256'], 'Producer input hash mismatch')
                edges += 1
            else:
                checked(path, item['sha256'], 'explicit link/source input')
        for dependency in command.get('dependencies', []):
            path = Path(dependency['path']).resolve()
            checked(path, dependency['sha256'], 'compiled dependency'); dependencies += 1
            if path.is_relative_to(source):
                require(manifest['source_files'].get(str(path.relative_to(source))) == dependency['sha256'],
                        'Dependency not bound by source manifest')
            else:
                require(not path.is_relative_to(build) and dependency['classification'] == 'sdk_or_system',
                        'Unproven generated dependency')
        output = Path(command['output']['path']).resolve()
        objects += output.suffix == '.o'
        if output not in latest or latest[output]['end_time_ns'] < command['end_time_ns']:
            latest[output] = command
    actual_products = {p.resolve() for p in build.rglob('*') if p.is_file() and
                       (p.suffix in ('.o', '.a', '.air', '.metallib') or p.name == 'mb3d_native_scene_context_dump')}
    claimed_products = {(base/p['path']).resolve(): p['sha256'] for p in receipt['products']}
    require(set(claimed_products) == actual_products and dump in actual_products, 'Missing or unproven build product')
    for path, expected in claimed_products.items():
        require(path in latest and latest[path]['output']['sha256'] == expected, 'Actual product producer differs')
        checked(path, expected, 'host executable/object/archive product')
    require(latest[dump]['kind'] == 'link', 'Executable must have actual link producer')
    require(objects > 0 and dependencies > 0 and edges > 0, 'Empty host build closure')
    for name, actual in {'commands': len(records), 'objects': objects, 'airs': 0,
                         'dependencies': dependencies, 'producer_edges': edges}.items():
        require(type(receipt.get(name)) is int and receipt[name] == actual, 'Build counter differs: '+name)
    return receipt


def inventory(output, run, dump, build_receipt):
    require(not output.exists() and not run.exists(), 'New inventory and run directories required')
    validate_host_build(dump, build_receipt)
    contract_path = ROOT/CONTRACT
    contract = read(contract_path)
    pin = contract['inventory_sha256']
    inventory_path = contract_path.parent/contract['inventory']
    require(digest(inventory_path.read_bytes()) == pin, 'Acceptance inventory changed')
    original = read(inventory_path)
    require(len(original['formulas']) == 467 and len(original['scenes']) == 77, 'Scope denominator changed')
    queue = read(ROOT/QUEUE)
    require(queue['contract_sha256'] == digest(contract_path.read_bytes()) and
            queue['inventory_sha256'] == pin, 'Restart queue binding changed')
    order = [f for group in queue['groups'] for f in group['formulas']]
    groups = {f"external:{f['id']}": group['id'] for group in queue['groups'] for f in group['formulas']}
    order += [f for f in original['formulas'] if f['namespace'] == 'internal']
    require(len({(f['namespace'], f['id']) for f in order}) == 467, 'Formula identity partition')
    # Resources helper verifies all declared source bytes before copying.
    from prepare_native_scene_resources import prepare_profiles
    run.mkdir(parents=True)
    resources = prepare_profiles(run/'resources')
    contexts = []
    for i, scene in enumerate(original['scenes']):
        profile = resources['scene_resource_profiles'][scene['id']]
        resource = dict(resources['profiles'][profile], profile=profile,
                        root=run/'resources'/resources['profiles'][profile]['runtime'])
        contexts.append(prepare_scene(scene, dump, run, resource, build_receipt, original['formulas']))
        print(f'Prepared host context {i+1}/77: {scene["id"]}', flush=True)
    contexts = deduplicate(contexts)
    require({sid for c in contexts for sid in c['scene_ids']} == {s['id'] for s in original['scenes']}, 'Lost scene links')
    scene_index = write(output/'scene-contexts.json', {'schema': 'mb3d.original-settings-scene-contexts.v1',
        'status': 'INVENTORIED', 'qualification': False, 'scene_inventory': ref(inventory_path),
        'resource_selection': ref(ROOT/'comparison/resources/phase1-selected/selection.json'),
        'build_receipt': ref(build_receipt), 'host': ref(dump),
        'scene_count': 77, 'contexts': contexts,
        'limitations': ['Host preparation is not original numerical oracle or renderer qualification.',
                       'No GPU or renderer was executed.', 'Probe case adapter and semantic completeness reviews remain REQUIRED/OPEN.']})
    inventory_refs = []
    for member in order:
        identity = {k: member[k] for k in ('namespace', 'id', 'name')}
        key, stem = f"{identity['namespace']}:{identity['id']}", f"{identity['namespace']}-{identity['id']}"
        defaults = external_defaults(identity) if identity['namespace'] == 'external' else internal_defaults(identity)
        serialized_identity = [identity[k] for k in ('namespace', 'id', 'name')]
        defaults['identity'] = serialized_identity
        default_ref = write(output/'defaults'/(stem+'.json'), defaults)
        source_refs = list(defaults['preparation_sources'])
        if 'source' in defaults:
            source_refs.insert(0, defaults['source'])
        selected = [c for c in contexts if key in c['formula_keys']]
        required_scenes = sorted({s for c in selected for s in c['scene_ids']})
        formula = {'schema': 'mb3d.formula-obligation-inventory.v1', 'identity': serialized_identity,
            'status': 'INVENTORIED', 'qualification': False, 'sources': source_refs, 'defaults': [default_ref],
            'scene_inventory': ref(inventory_path), 'scene_contexts_index': scene_index,
            'required_scene_ids': required_scenes, 'context_ids': [c['id'] for c in selected],
            'contexts': selected, 'group': groups.get(key, '39-internal'),
            'completeness_review': 'REQUIRED_OPEN'}
        formula_ref = write(output/'formulas'/(stem+'.json'), formula)
        skeleton_ref = write(output/'declarations'/(stem+'.json'), {
            'schema': 'mb3d.formula-obligation-skeleton.v1', 'identity': serialized_identity,
            'status': 'INVENTORIED', 'collectable': False, 'inventory': formula_ref,
            'categories': [{'id': category, 'disposition': 'REQUIRED', 'status': 'OPEN',
                            'reason': 'Per-formula source, domain and completeness review pending.'}
                           for category in CATEGORIES],
            'not_applicable': [], 'case_mappings': [], 'completeness_review': 'REQUIRED_OPEN'})
        inventory_refs.append({'identity': serialized_identity, 'inventory': formula_ref, 'skeleton': skeleton_ref})
    result = {'schema': 'mb3d.formula-obligation-index.v1', 'status': 'INVENTORIED', 'qualification': False,
              'contract': ref(contract_path), 'scene_inventory': ref(inventory_path), 'queue': ref(ROOT/QUEUE),
              'scene_contexts_index': scene_index, 'extraction_tool': ref(Path(__file__)),
              'formula_count': 467, 'external_count': 457, 'internal_count': 10,
              'scene_count': 77, 'contexts_count': len(contexts), 'formulas': inventory_refs}
    write(output/'index.json', result)
    write(output/'progress.json', {'schema': 'mb3d.formula004.inventory-progress.v1', 'status': 'INVENTORIED',
        'inventory': ref(output/'index.json'), 'inventoried': 467, 'audited': 0, 'verified': 0, 'corrected': 0,
        'external': {'denominator': 457, 'verified': 0, 'corrected': 0},
        'priority_external': {'denominator': 95, 'verified': 0, 'corrected': 0},
        'internal': {'denominator': 10, 'verified': 0, 'corrected': 0},
        'qualification': False, 'candidate': None, 'historical_counts_inherited': False})
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--dump', required=True, type=Path)
    parser.add_argument('--build-receipt', required=True, type=Path)
    parser.add_argument('--run-dir', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    args = parser.parse_args()
    for p in (args.dump, args.build_receipt):
        require(p.is_file(), 'Missing host/build artifact '+str(p))
    result = inventory(args.output.resolve(), args.run_dir.resolve(), args.dump.resolve(), args.build_receipt.resolve())
    print(json.dumps({k: result[k] for k in ('status', 'formula_count', 'scene_count', 'contexts_count')}))


if __name__ == '__main__':
    main()
