"""Passive validation of a probe's optional range-count side channel.

This proves reported count repeatability and full59 noninterference only. The
caller must bind actual process/build/input provenance separately. A range
count is not formula admission, original parity or a whole-frame fallback.
"""
from native_formula_observer import words
from qualify_native_metal_all import require


def uint(value):
    require(type(value) is int and 0 <= value <= 0xffffffff, 'Expected uint32 range metadata')
    return value


def validate_range_controls(report, expected_counts=None):
    require(report.get('schema') == 'mb3d.native-formula-probe.v1' and
            report.get('profile') == 'native_gpu_fp32' and
            words(report.get('library_profile'), 3) == [2, 4, 32] and
            words(report.get('formula_abi'), 5) == [1, 8192, 188, 236, 32], 'Wrong range probe/profile/ABI')
    count, repeats, mode = (uint(report.get(k)) for k in ('point_count', 'repeats', 'mode'))
    require(1 <= count <= 4096 and 3 <= repeats <= 16 and mode <= 2, 'Range probe bounds')
    require(all(report.get(k) is True for k in ('exact_state_repeats', 'input_immutable', 'allocation_canaries')) and
            type(report.get('invalid_descriptor_controls')) is int and report['invalid_descriptor_controls'] == 7,
            'Ordinary probe integrity failed')
    baseline = report.get('rows')
    require(isinstance(baseline, list) and len(baseline) == count, 'Missing ordinary baseline rows')
    baseline_words = []
    for i, row in enumerate(baseline):
        require(uint(row.get('index')) == i, 'Wrong ordinary row index')
        words(row.get('input_words'), 47)
        result = words(row.get('result_words'), 59)
        require(uint(row.get('fault')) == result[47] <= 9 and uint(row.get('work')) == result[51] and
                result[53:55] == [0, 0] and result[56] == int(result[47] == 0) and
                result[57] in (0, 1), 'Inconsistent ordinary result/fault/work')
        baseline_words.append(result)
    faults = sum(result[47] != 0 for result in baseline_words)
    require(uint(report.get('fault_count')) == faults and
            report.get('status') == ('POINT_FAULTS' if faults else 'COMPLETE'), 'Wrong ordinary fault disposition')
    control = report.get('range_controls')
    fields = {'schema', 'qualification', 'status', 'kernel', 'repeats', 'exact_result_matches_baseline',
              'exact_control_repeats', 'allocation_canaries', 'input_immutable', 'program_immutable',
              'resources_immutable', 'runs'}
    require(isinstance(control, dict) and set(control) == fields and
            control['schema'] == 'mb3d.native-formula-range-controls.v1' and
            control['qualification'] is False and control['status'] == 'EXACT_NONINTERFERENCE' and
            control['kernel'] == 'native_formula_range_probe' and uint(control['repeats']) == repeats,
            'Missing/malformed range controls')
    require(all(control[k] is True for k in fields if k.endswith('immutable') or
                k in ('exact_result_matches_baseline', 'exact_control_repeats', 'allocation_canaries')),
            'Range controls failed integrity/noninterference')
    runs = control['runs']
    require(isinstance(runs, list) and len(runs) == repeats, 'Missing range repeats')
    counts = None
    for repeat, run in enumerate(runs, 1):
        require(isinstance(run, dict) and set(run) == {'repeat', 'rows'} and
                uint(run['repeat']) == repeat and isinstance(run['rows'], list) and len(run['rows']) == count,
                'Malformed/reused range repeat')
        actual_counts = []
        for i, row in enumerate(run['rows']):
            require(isinstance(row, dict) and set(row) == {'index', 'range_exits', 'result_words', 'fault', 'work', 'completed'} and
                    uint(row['index']) == i, 'Malformed/reordered range row')
            result = words(row['result_words'], 59)
            require(result == baseline_words[i] and uint(row['fault']) == result[47] and
                    uint(row['work']) == result[51] and uint(row['completed']) == result[56],
                    'Range observation changed complete59 result/work/fault')
            exits = uint(row['range_exits'])
            require(exits <= result[51] and (mode != 0 or exits == 0), 'Impossible range count')
            actual_counts.append(exits)
        require(counts is None or counts == actual_counts, 'Range counts are not exactly repeatable')
        counts = actual_counts
    if expected_counts is not None:
        require(isinstance(expected_counts, list) and len(expected_counts) == count and
                all(type(v) is int and 0 <= v <= 0xffffffff for v in expected_counts) and
                counts == expected_counts, 'Range counts differ from declared source-policy expectation')
    return {'schema': 'mb3d.native-formula-range-observation.v1', 'status': 'OBSERVED_RANGE_COUNTS',
            'qualification': False, 'mode': mode, 'counts': counts,
            'faults': [r[47] for r in baseline_words], 'work': [r[51] for r in baseline_words],
            'repeats': repeats, 'limits': 'Count and noninterference only; external provenance and source proof required; no frame fallback claim'}
