"""Inventory effective INTEGER inputs in active original-code fixture tables.

This reads fixtures actually called by test_renderer.cpp, not merely files in
the fixture folder. It reports input coverage, never numerical PASS or complete
branch coverage. Execution evidence and original-oracle provenance stay separate.
"""
import argparse
from collections import Counter, defaultdict
import json
import math
from pathlib import Path
import re
import struct

from external_validation import ROOT, digest, prepare_external_options, slots, write_json
from gen_bulk_formula_table import parse_m3f_options


def number(value):
    return float(value.strip().replace('INFINITY', 'inf').replace('NAN', 'nan'))


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('--selectors', type=Path, required=True)
    ap.add_argument('--output', type=Path, required=True)
    args = ap.parse_args()
    source = ROOT / 'tests/test_renderer.cpp'
    code = source.read_text()
    selector = json.loads(args.selectors.read_bytes())
    integer_indices = defaultdict(list)
    for row in selector['options']:
        integer_indices[row['formula']].append(row['stored_value_index'])
    active = []
    for match in re.finditer(r'namespace\s+(\w+)\s*\{\s*#include\s+"(fixtures/[^"\n]+)"\s*\}', code):
        if re.search(r'check_external_formula_regressions\(\s*' + re.escape(match[1]) + r'::formulaRegressions\b', code):
            active.append(ROOT / 'tests' / match[2])
    # The original shared table is intentionally in the global namespace.
    if re.search(r'check_external_formula_regressions\(formulaRegressions\)', code):
        for match in re.finditer(r'^#include "(fixtures/[^"\n]+)"', code, re.M):
            path = ROOT / 'tests' / match[1]
            if 'static const FormulaRegression formulaRegressions[]' in path.read_text() and path not in active:
                active.append(path)
    pattern = re.compile(r'^\s*\{"([^"\n]+)",\s*(\d+),\s*\{([^}]*)\},\s*\{([^}]*)\}')
    counts = Counter()
    by_formula = defaultdict(lambda: dict(rows=0, fixtures=set(), unique_options=set(), input_categories=Counter()))
    payloads = {}
    identities = {}
    errors = []
    for path in active:
        identities[path.relative_to(ROOT).as_posix()] = digest(path.read_bytes())
        for line, text in enumerate(path.read_text().splitlines(), 1):
            match = pattern.match(text)
            if not match:
                if text.lstrip().startswith('{"'):
                    errors.append(dict(path=str(path.relative_to(ROOT)), line=line, error='unrecognized row layout'))
                continue
            name, count = match[1], int(match[2])
            if name.startswith('JIT'):
                counts['deferred_rows_excluded'] += 1
                continue
            try:
                values = tuple(number(v) for v in match[3].split(','))
                inputs = [number(v) for v in match[4].split(',')]
                if len(values) != 16:
                    raise ValueError('Expected all 16 saved values')
            except ValueError as error:
                errors.append(dict(path=str(path.relative_to(ROOT)), line=line, error=str(error)))
                continue
            counts['parsed_rows'] += 1
            record = by_formula[name]
            record['rows'] += 1
            record['fixtures'].add(path.relative_to(ROOT).as_posix())
            raw = struct.pack('<i16d', count, *values)
            key = (name, raw)
            record['unique_options'].add(raw)
            for category, present in [('nan', any(math.isnan(v) for v in inputs)),
                                      ('infinite', any(math.isinf(v) for v in inputs)),
                                      ('subnormal', any(0 < abs(v) < 2.2250738585072014e-308 for v in inputs)),
                                      ('signed_zero', any(v == 0 and math.copysign(1, v) < 0 for v in inputs)),
                                      ('finite', all(math.isfinite(v) for v in inputs))]:
                if present: record['input_categories'][category] += 1
            if key not in payloads:
                payloads[key] = dict(name=name, count=count, values=values, fixture=path.relative_to(ROOT).as_posix(), line=line)
    observed = defaultdict(lambda: defaultdict(set))
    overwritten = []
    metadata = {name: parse_m3f_options(ROOT / 'mb3d/M3Formulas' / (name + '.m3f'))
                for name in by_formula if name in integer_indices}
    for item in payloads.values():
        name = item['name']
        if name not in integer_indices: continue
        data = bytearray(848 + 6 * 188)
        struct.pack_into('<iii', data, 848, 1, 20, item['count'])
        data[860:892] = name.encode('ascii').ljust(32, b'\0')
        struct.pack_into('<16d', data, 908, *item['values'])
        effective = prepare_external_options(data, slots(data)[0], metadata[name])['options']
        for index in integer_indices[name]:
            value = effective[index]
            # Same x87 int64 nearest-even/indefinite then low32 model as the
            # scene audit; native conversion boundary fixtures are separate.
            integer = round(value) if math.isfinite(value) and -(2**63) <= value < 2**63 else -(2**63)
            integer = ((integer + 2**31) % 2**32) - 2**31
            observed[name][index].add(integer)
            raw = item['values'][index]
            if struct.pack('<d', raw) != struct.pack('<d', value):
                overwritten.append(dict(formula=name, stored_value_index=index, saved_count=item['count'],
                    raw_value=repr(raw), effective_value=repr(value), fixture=item['fixture'], line=item['line']))
    formulas = []
    for name, record in sorted(by_formula.items()):
        formulas.append(dict(formula=name, rows=record['rows'], fixtures=sorted(record['fixtures']),
            unique_option_vectors=len(record['unique_options']), input_categories=dict(record['input_categories']),
            effective_integer_values={str(k): sorted(v) for k, v in observed[name].items()}))
    report = dict(scope=__doc__, totals={**counts, 'active_fixture_headers':len(active), 'formulas':len(formulas),
        'unique_option_vectors':len(payloads), 'overwritten_integer_selections':len(overwritten), 'parse_errors':len(errors)},
        input_sha256={**identities, str(source.relative_to(ROOT)):digest(source.read_bytes()),
                      str(args.selectors.resolve().relative_to(ROOT)):digest(args.selectors.read_bytes()),
                      str(Path(__file__).resolve().relative_to(ROOT)):digest(Path(__file__).read_bytes())},
        formulas=formulas, overwritten_integer_selections=overwritten, errors=errors,
        limitations=['Fixtures are finite samples; input categories do not prove all mathematical boundaries.',
                     'Prepared options are a source audit; native loader/oracle identities remain separate obligations.',
                     'Only FormulaRegression tables called by the shared runner are parsed; specialized fixtures need separate evidence.'])
    write_json(args.output, report)
    print(json.dumps(report['totals'], indent=2))
    return int(bool(errors))


if __name__ == '__main__':
    raise SystemExit(main())
