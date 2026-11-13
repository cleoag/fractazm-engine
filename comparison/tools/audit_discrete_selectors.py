"""Static INTEGER option obligations, with identity-checked scene observations.

This does not execute a formula and never emits a parity PASS. An input value
present in a scene is not proof that its branch was executed, state was checked,
or the scene passed on the current binary. Finite domains taken from option
labels are distinguished from reviewed C++ selector equivalence classes.
"""
from __future__ import annotations

import argparse
from collections import Counter, defaultdict
import hashlib
import json
import math
import os
from pathlib import Path
import re
import struct
import sys

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / 'scripts'))
from gen_bulk_formula_table import M3F_OPTION_MAP, compute_pvar_offsets
from external_validation import decode, write_json, prepare_external_options
from discrete_selector_domains import classify as infer_domain, fingerprint

EXCLUDED = {'JITTrigExample', 'JITBenesiPineTree', 'JIT_IQ_Bulb'}
SAVED_WIDTH = {5: 3, 6: 3, 12: 6, 18: 4, 19: 2}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def relative(path):
    # These are catalog paths under ROOT. Avoid repeated Windows filesystem
    # resolution for every option of every scene.
    return Path(os.path.abspath(path)).relative_to(ROOT).as_posix()


def local(path):
    path = Path(str(path).replace('\\', '/'))
    return path if path.is_absolute() else ROOT / path


def declarations(path):
    """Line-based parsing avoids attaching a preceding .SSE2 to .Integer."""
    result, in_options = [], False
    for lineno, raw in enumerate(path.read_text(errors='replace').splitlines(), 1):
        line = raw.strip()
        if line.startswith('[') and line.endswith(']'):
            in_options = line == '[OPTIONS]'
            continue
        if not in_options or not line.startswith('.') or '=' not in line:
            continue
        key, _, default = line.partition('=')
        parts = key[1:].split(maxsplit=1)
        kind = M3F_OPTION_MAP.get(parts[0].lower()) if parts else None
        if kind is not None:
            result.append(dict(type=kind, label=parts[1].strip() if len(parts)>1 else '',
                               default_text=default.strip(), line=lineno))
    return result


def integer_value(value):
    """The loader's nearest-even Int64 conversion, then low signed 32 bits.

    Invalid/nonfinite Int64 conversions are retained as unknown rather than
    guessing a platform's invalid-conversion sentinel.
    """
    if not math.isfinite(value):
        return None
    rounded = round(value)
    if not -(1 << 63) <= rounded < (1 << 63):
        return None
    return ((rounded + (1 << 31)) % (1 << 32)) - (1 << 31)


def clean_code(text):
    # Preserve line numbers; comments are not evidence of implemented branches.
    text = re.sub(r'/\*.*?\*/', lambda m: '\n' * m.group().count('\n'), text, flags=re.S)
    return re.sub(r'//[^\n]*', '', text)


def source_uses(path, runtime_index):
    text = path.read_text(errors='replace')
    clean = clean_code(text)
    lines = text.splitlines()
    mention = re.compile(r'\bparams\s*\[\s*' + str(runtime_index) + r'\s*\]')
    evidence, aliases = [], set()
    for lineno, line in enumerate(clean.splitlines(), 1):
        if mention.search(line):
            evidence.append(dict(line=lineno, text=lines[lineno-1].strip()))
            m = re.search(r'\b(?:const\s+)?int\s+(\w+)\s*=.*?' + mention.pattern, line)
            if m:
                aliases.add(m[1])
    branches = []
    for lineno, line in enumerate(clean.splitlines(), 1):
        if any(re.search(r'\b' + re.escape(a) + r'\b', line) for a in aliases):
            if re.search(r'\b(if|switch|for|while)\b|[&|]|==|!=|<=|>=|<<|>>', line):
                branches.append(dict(line=lineno, text=lines[lineno-1].strip()))
    return dict(path=relative(path), sha256=sha(path.read_bytes()),
                runtime_param_mentions=evidence, direct_integer_aliases=sorted(aliases),
                candidate_branch_lines=branches,
                native_dispatch_present=bool(re.search(r'Original_\w+\s*\(', clean)),
                limitation='Lexical evidence; helper/native assembly dataflow is not automatically proved.')


def domain(formula, saved, label):
    """Small reviewed C++ domains, then conservative declared-range candidates.

    Exact/other classes refer to this selector's control use, not whole-formula
    equivalence. Native dispatch and the original binary still need qualification.
    """
    mask_specs = {('SphereIFS', 9): 3, ('boxIFS', 10): 3,
                  ('cylinderIFS', 9): 3, ('RoundedBox', 9): 3,
                  ('Amazing Surf 2', 6): 15, ('koch_surf', 13): 3}
    exact_specs = {
        ('SphereIFS', 8): ([0], 1), ('boxIFS', 9): ([0], 1),
        ('cylinderIFS', 8): ([0], 1), ('boardIFS', 6): ([0], 1),
        ('boardIFS', 10): ([1, 2, 3, 4], 0),
        ('Amazing Surf', 7): ([0], 1),
        ('ATetraVS', 7): ([0, 1, 2], 3), ('ATetraVS', 8): ([1], 0),
        ('TetraFolding Pow2', 9): ([0, 1, 2], 3),
        ('TetraFolding Pow2', 10): ([1], 0),
        ('General', 4): ([1, 2, 3], 0), ('General', 6): ([1, 2, 3], 0),
        ('General', 7): ([0, -1], 1), ('General', 8): ([0, -1], 1),
        ('General', 9): ([0, -1], 1), ('General', 10): ([0, 1], 2),
    }
    key = (formula, saved)
    if key in mask_specs:
        mask = mask_specs[key]
        return dict(kind='reviewed_cpp_mask', mask=mask, representatives=list(range(mask+1)),
                    equivalence=f'All int32 values with equal (value & {mask}) take the same selector decisions.',
                    domain_complete_for_this_cpp_selector=True,
                    original_and_native_domain_qualification='not_established_by_this_audit')
    if key in exact_specs:
        exact, other = exact_specs[key]
        return dict(kind='reviewed_cpp_exact_or_other', exact_values=exact,
                    other_representative=other, representatives=exact+[other],
                    equivalence=f'Equality classes {exact}; all other int32 values share the remaining selector path.',
                    domain_complete_for_this_cpp_selector=True,
                    original_and_native_domain_qualification='not_established_by_this_audit')
    if formula == 'ABoxVSShapes' and saved == 4:
        return dict(kind='reviewed_cpp_cases', representatives=list(range(1, 18)),
                    equivalence='Cases 1..17; numerically distinct min/max subcases are retained.',
                    domain_complete_for_this_cpp_selector=False,
                    outside_domain='Default/original-stack behavior requires separate invalid-input qualification.')
    match = re.search(r'(?<![\d.])(-?\d+)\s*(?:\.\.|\bto\b|-)\s*(-?\d+)', label, re.I)
    if match:
        lo, hi = map(int, match.groups())
        if 0 <= hi-lo <= 63:
            return dict(kind='declared_bounded_candidates', representatives=list(range(lo, hi+1)),
                        equivalence='No equivalence inferred from a user-facing range label.',
                        domain_complete_for_this_cpp_selector=False,
                        outside_domain='Unclassified; a label is not a runtime range check.')
        return dict(kind='large_declared_range', declared_min=lo, declared_max=hi,
                    representatives=[], domain_complete_for_this_cpp_selector=False,
                    limitation='Do not enumerate counters blindly; classify arithmetic and loop boundaries first.')
    return dict(kind='unclassified_integer', representatives=[],
                domain_complete_for_this_cpp_selector=False,
                limitation='Needs source-qualified selector, counter, signed-width, resource-ID or numerical classification.')


def class_of(value, spec):
    if spec['kind'].startswith('automated_cpp_'):
        key = fingerprint(value, spec)
        return next(v for v in spec['representatives'] if fingerprint(v, spec) == key)
    if spec['kind'] == 'reviewed_cpp_mask':
        return value & spec['mask']
    if spec['kind'] == 'reviewed_cpp_exact_or_other':
        return value if value in spec['exact_values'] else spec['other_representative']
    return value


def known_rejection(slots):
    # Read saved doubles through INTEGER conversion; never inspect their raw low byte.
    for slot in slots:
        name, opts = slot['name'], slot['options']
        if name in {'gnarlyIFS', 'gnarly2IFS', 'gnarly3IFS', 'ducksIFS'}:
            flip = integer_value(opts[8])
            if flip is not None and flip & 1:
                return 'caller_dependent_odd_flip'
        if name == 'TrifoxComplexAngles' and any(integer_value(opts[i]) == 0 for i in (0, 3)):
            return 'nonterminating_zero_power'
    return None


def raw_slots(data):
    result = []
    for i in range(6):
        offset = 848 + 188*i
        count, number, option_count = struct.unpack_from('<iii', data, offset)
        if not (count & 0x7fffffff):
            continue
        result.append(dict(slot=i, count=count, number=number, option_count=option_count,
                           name=bytes(data[offset+12:offset+44]).split(b'\0')[0].decode('latin1'),
                           options=list(struct.unpack_from('<16d', data, offset+60))))
    return result


def build(ledger_path, manifest_dir):
    ledger = json.loads(ledger_path.read_text(encoding='utf-8-sig'))
    formulas = [f for f in ledger['formulas'] if f['formula'] not in EXCLUDED]
    names = {f['formula'] for f in formulas}
    metadata = {f['formula']: f['metadata'] for f in formulas}
    integer_columns = {}
    for f in formulas:
        saved, columns = 0, []
        for t in f['metadata']['option_types']:
            if t in (2, 20):
                columns.append(saved)
            saved += SAVED_WIDTH.get(t, 1)
        integer_columns[f['formula']] = columns
    catalog, sources, issues = {}, [], []

    def add_case(case, source, population):
        targets = case.get('target_formulas', [])
        if not targets or not set(targets).issubset(names):
            return
        scene, expected = case.get('scene'), case.get('scene_sha256') or case.get('sha256')
        if not scene or not expected:
            issues.append(dict(source=source, case=case.get('id'), problem='missing_scene_identity'))
            return
        path = local(scene)
        key = (os.path.abspath(path).casefold(), expected)
        if key not in catalog:
            catalog[key] = dict(path=path, expected=expected, targets=set(), populations=set(), references=[])
        row = catalog[key]
        row['targets'].update(targets)
        row['populations'].add(population)
        if len(row['references']) < 3:
            row['references'].append(dict(source=source, case=case.get('case') or case.get('id')))

    for case in ledger['external_cases']:
        add_case(case, relative(ledger_path), 'base_active_5506')
    for path in sorted(manifest_dir.glob('*.json')):
        lower = path.name.lower()
        if not ('manifest' in lower or 'selectors' in lower) or re.search(r'monte|jit|(?:^|[-_])mc(?:[-_]|\.)', lower):
            continue
        content = path.read_bytes()
        data = json.loads(content.decode('utf-8-sig'))
        cases = data.get('cases', []) if isinstance(data, dict) else []
        if not isinstance(cases, list) or not any(isinstance(c, dict) and 'scene' in c for c in cases):
            continue
        sources.append(dict(path=relative(path), sha256=sha(content), listed_cases=len(cases)))
        for case in cases:
            if isinstance(case, dict):
                add_case(case, relative(path), 'additional_manifest')

    observed = defaultdict(lambda: defaultdict(lambda: defaultdict(dict)))
    catalog_counts = Counter()
    migrated_slots, overwritten_selectors = [], []
    print(f'Catalog: {len(catalog)} unique scene identities from {len(sources)} additional manifests', flush=True)
    for row in catalog.values():
        path = row['path']
        if not path.exists():
            issues.append(dict(scene=str(path), problem='missing_scene', references=row['references']))
            catalog_counts['missing_scene'] += 1
            continue
        actual = sha(path.read_bytes())
        if actual != row['expected']:
            issues.append(dict(scene=relative(path), problem='scene_sha256_mismatch', expected=row['expected'], actual=actual))
            catalog_counts['scene_sha256_mismatch'] += 1
            continue
        try:
            data = decode(path)
            active = raw_slots(data)
        except ValueError as exc:
            issues.append(dict(scene=relative(path), problem='unsupported_scene_layout', error=str(exc)))
            catalog_counts['unsupported_scene_layout'] += 1
            continue
        scene_name = relative(path)
        effective = []
        for slot in active:
            name = slot['name']
            if name in metadata and slot['number'] >= 20:
                loaded = prepare_external_options(data, slot, metadata[name])
                if slot['option_count'] < loaded['option_count']:
                    migrated_slots.append(dict(scene=scene_name, scene_sha256=actual,
                        formula=name, slot=slot['slot'], saved_count=slot['option_count'],
                        expanded_count=loaded['option_count']))
                for saved in integer_columns[name]:
                    if struct.pack('<d', slot['options'][saved]) != struct.pack('<d', loaded['options'][saved]):
                        overwritten_selectors.append(dict(scene=scene_name, scene_sha256=actual,
                            formula=name, slot=slot['slot'], stored_value_index=saved,
                            raw_value=slot['options'][saved], effective_value=loaded['options'][saved]))
                effective.append(loaded)
            else:
                effective.append(slot)
        rejection = known_rejection(effective)
        catalog_counts['identity_checked_scenes'] += 1
        if rejection:
            catalog_counts['known_strict_rejected_scenes'] += 1
        for slot in effective:
            name = slot['name']
            if name not in row['targets']:
                continue
            # Record input presence only; loop reachability still needs dynamic evidence.
            for saved in integer_columns[name]:
                value = slot['options'][saved]
                converted = integer_value(value)
                if converted is None:
                    continue
                identity = (scene_name, slot['slot'])
                bucket = observed[name][saved][converted]
                if identity not in bucket:
                    bucket[identity] = dict(scene=scene_name, scene_sha256=actual, slot=slot['slot'],
                                            saved_value=value, populations=sorted(row['populations']),
                                            known_strict_rejection=rejection, references=row['references'])

    print(f'Identity-checked: {catalog_counts["identity_checked_scenes"]}; issues: {len(issues)}', flush=True)
    option_rows, mapping_changes, obligations = [], [], []
    for formula in formulas:
        name = formula['formula']
        asset = local(formula['pascal_asset']['path'])
        decls = declarations(asset)
        types = formula['metadata']['option_types']
        if [d['type'] for d in decls] != types:
            raise ValueError(f'Metadata declaration identity mismatch: {name}')
        stored = 0
        expanded = compute_pvar_offsets(types)
        for index, item in enumerate(decls):
            runtime = len(compute_pvar_offsets(types[:index]))
            if item['type'] not in (2, 20):
                stored += SAVED_WIDTH.get(item['type'], 1)
                continue
            values, known_rejected_values, samples = [], [], {}
            for value, sightings in sorted(observed[name][stored].items()):
                usable = [s for s in sightings.values() if not s['known_strict_rejection']]
                if usable:
                    values.append(value)
                    samples[str(value)] = dict(scene_slot_count=len(usable), examples=usable[:2])
                if len(usable) != len(sightings):
                    known_rejected_values.append(value)
            spec = domain(name, stored, item['label'])
            seen_classes = sorted({class_of(v, spec) for v in values})
            missing = [v for v in spec['representatives'] if v not in seen_classes]
            is_map_id = bool(formula['metadata'].get('map_value_mask', 0) & (1 << stored))
            source = source_uses(local(formula['cpp_implementation']['path']), runtime)
            row = dict(formula=name, downloaded_unique_scenes=formula['downloaded_unique_scenes'],
                       declaration_index=index, stored_value_index=stored, runtime_param_index=runtime,
                       pvar_offset=expanded[runtime][0], loader_type=item['type'], label=item['label'],
                       runtime_index_basis='canonical generated option expansion; custom dispatch still needs source dataflow qualification',
                       asset=dict(path=relative(asset), line=item['line'], sha256=sha(asset.read_bytes())),
                       resource_map_id=is_map_id, source=source, domain=spec,
                       observed_int32_values=values, known_rejected_input_values=known_rejected_values,
                       observations=samples, missing_representatives_in_scene_catalog=missing,
                       input_presence_status=('unclassified_domain' if not spec['representatives'] else
                                              'missing_representatives' if missing else 'listed_representatives_present'),
                       branch_execution_status='not_measured', parity_status='not_assessed',
                       integer_conversion_boundaries='Separate shared-loader fixture; not proved by integer scene samples.',
                       interactions='Unclassified; marginal option presence does not prove combinations or state writeback.')
            option_rows.append(row)
            if stored != index:
                mapping_changes.append(dict(formula=name, label=item['label'], declaration_index=index,
                                            stored_value_index=stored, runtime_param_index=runtime))
            if missing:
                obligations.append(dict(formula=name, stored_value_index=stored, runtime_param_index=runtime,
                                        label=item['label'], downloaded_unique_scenes=formula['downloaded_unique_scenes'],
                                        domain_kind=spec['kind'], missing_representatives=missing,
                                        source=source['path'], asset_line=item['line'],
                                        action='First inspect fixture-only evidence, then add minimum distinguishing state/point inputs and useful image controls.'))
            stored += 1

    obligations.sort(key=lambda r: (-r['downloaded_unique_scenes'], r['formula'], r['stored_value_index']))
    status = Counter(row['input_presence_status'] for row in option_rows)
    return dict(schema_version=1, scope=dict(active_formulas=len(formulas), excluded_formulas=sorted(EXCLUDED),
                    deferred=['Monte Carlo', 'JIT'], option_loader_types=[2, 20]),
                interpretation=__doc__.strip(),
                limitations=[
                    'Only INTEGER/2INTEGER declarations inventoried; DOUBLE-to-int selectors and packed internal mode bits are a separate remaining obligation.',
                    'Values are counted after manifest SHA256 and effective loader defaults/migrations. This Python preparation is not independent native-loader proof; branch execution still needs dynamic checks.',
                    'Initial nonzero slot counts identify input observations; actual loop/caller reachability is not inferred.',
                    'Known odd-Flip/zero-power inputs remain rejected and do not satisfy supported-input observations.',
                    'Additional manifests are inventoried without promoting their historical result status. Fixture-only inputs not represented by scene manifests remain unaudited.',
                    'Reviewed C++ equivalence classes do not certify original x86/native assembly, nonfinite boundaries, or coupled selector interactions.',
                    'Large ranges and unbounded counters require mathematical/algorithmic partitions, not blind enumeration.'
                ],
                totals=dict(integer_formulas=len({r['formula'] for r in option_rows}), integer_options=len(option_rows),
                            option_types=dict(Counter(str(r['loader_type']) for r in option_rows)),
                            declared_and_reviewed_domain_options=sum(bool(r['domain']['representatives']) for r in option_rows),
                            reviewed_cpp_equivalence_options=sum(r['domain']['kind'].startswith('reviewed_cpp') for r in option_rows),
                            input_presence_status=dict(status), shifted_integer_option_indices=len(mapping_changes),
                            unique_catalog_entries=len(catalog), catalog=dict(catalog_counts),
                            source_manifests=len(sources), catalog_identity_issues=len(issues),
                            missing_representatives=sum(len(r['missing_representatives']) for r in obligations),
                            short_saved_option_slots=len(migrated_slots),
                            overwritten_integer_observations=len(overwritten_selectors),
                            scenes_with_overwritten_integer_values=len({r['scene'] for r in overwritten_selectors}),
                            parity_passes_produced=0),
                inputs=dict(ledger=dict(path=relative(ledger_path), sha256=sha(ledger_path.read_bytes())),
                            tool_sha256=sha(Path(__file__).read_bytes()),
                            metadata_generator_sha256=sha((ROOT/'scripts/gen_bulk_formula_table.py').read_bytes()),
                            source_manifests=sources),
                shifted_integer_option_indices=mapping_changes, prioritized_obligations=obligations,
                short_saved_option_slots=migrated_slots,
                overwritten_integer_observations=overwritten_selectors,
                options=option_rows, catalog_identity_issues=issues)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--ledger', type=Path, default=ROOT/'comparison/results/stage-a-20261018/coverage-ledger-v1.json')
    parser.add_argument('--manifest-dir', type=Path, default=ROOT/'comparison/results/external-complete-20261013')
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--observations-from', type=Path,
                        help='Reuse the recorded input-observation snapshot; reclassify current source without reopening scenes.')
    args = parser.parse_args()
    if args.observations_from:
        raw = args.observations_from.read_bytes()
        report = json.loads(raw.decode('utf-8-sig'))
        report['inputs']['observation_snapshot'] = dict(path=relative(args.observations_from), sha256=sha(raw),
            scene_identity_status='Verified at snapshot creation; not reopened during source-only reclassification.')
    else:
        report = build(args.ledger.resolve(), args.manifest_dir.resolve())
    reclassify(report)
    write_json(args.output, report)
    print(json.dumps(report['totals'], indent=2))
    return 0


def reclassify(report):
    """Add only exhaustive direct-alias partitions; leave complex uses explicit."""
    obligations = []
    for row in report['options']:
        if not row['domain']['kind'].startswith('reviewed_cpp'):
            source_path = local(row['source']['path'])
            spec = infer_domain(source_path.read_text(errors='replace'), row['runtime_param_index'])
            if spec:
                row['declared_domain'] = row['domain']
                row['domain'] = spec
        spec = row['domain']
        seen = {class_of(v, spec) for v in row['observed_int32_values']}
        missing = [v for v in spec['representatives'] if v not in seen]
        row['missing_representatives_in_scene_catalog'] = missing
        row['input_presence_status'] = ('unclassified_domain' if not spec['representatives'] else
                                       'missing_representatives' if missing else 'listed_representatives_present')
        if missing:
            obligations.append(dict(formula=row['formula'], stored_value_index=row['stored_value_index'],
                runtime_param_index=row['runtime_param_index'], label=row['label'],
                downloaded_unique_scenes=row['downloaded_unique_scenes'], domain_kind=spec['kind'],
                missing_representatives=missing, source=row['source']['path'], asset_line=row['asset']['line'],
                action='Inspect fixture-only evidence, then add minimal distinguishing state/point inputs and useful image controls.'))
    obligations.sort(key=lambda r: (-r['downloaded_unique_scenes'], r['formula'], r['stored_value_index']))
    report['prioritized_obligations'] = obligations
    rows = report['options']
    report['totals'].update(
        declared_and_reviewed_domain_options=sum(bool(r['domain']['representatives']) for r in rows),
        automated_complete_cpp_domain_options=sum(r['domain']['kind'].startswith('automated_cpp') for r in rows),
        domain_kind_counts=dict(Counter(r['domain']['kind'] for r in rows)),
        input_presence_status=dict(Counter(r['input_presence_status'] for r in rows)),
        missing_representatives=sum(len(r['missing_representatives']) for r in obligations))
    report['inputs']['tool_sha256'] = sha(Path(__file__).read_bytes())
    report['inputs']['domain_classifier_sha256'] = sha((ROOT/'comparison/tools/discrete_selector_domains.py').read_bytes())


if __name__ == '__main__':
    raise SystemExit(main())
