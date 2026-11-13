"""Source-ordered RN32 diagnostic for LightBulbMoon's actual mixed chain.

This is a diagnostic reference, not a formula admission. A separately reviewed
original mixed FPC caller proves the return-register overwrite, completing all
47 independently modelled final fields. No candidate output is imported.
Each arithmetic primitive rounds RN-even to binary32. Metal contraction may
disagree with these targets; such a difference is a mismatch, never a learned
target or permission to relax the comparison.
"""
from __future__ import annotations

from fractions import Fraction
import hashlib
from pathlib import Path
import struct

import native_formula_context_input as binding
import native_formula_reference as arithmetic
import native_formula_affine_v2_reference as affine
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
ONE = 0x3f800000
SSE_FIELDS = tuple(f'sse[{i}]' for i in range(4))
KNOWN_FIELDS = tuple(k for k in STATE if k not in SSE_FIELDS)
SOURCE_PINS = {
    'mb3d/M3Formulas/Amazing Surf.m3f': '19f5cfa2cdd127a34851f7312c0226078aea675ca9d17e6079b46b9ec70be03d',
    'mb3d/M3Formulas/_AmazingBox.m3f': 'a3e18ad5fb97eaf275534d24e5b4bece03110384940edfa90b8fe93d3f2e06ce',
    'mb3d/M3Formulas/_Translate.m3f': '3acdcb61881b9007cd1a66a33887d8a13ae7541bd3120472b582e2ac9a09b706',
    'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
    'mb3d/HeaderTrafos.pas': '52b3209608ba3360f4be7d821866c37a1845bc09f9aa2ec56d1912616f5d7f9d',
    'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
    'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
    'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
}
PRIMITIVE_SHA = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'
ABI_SHA = 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a'
AFFINE_MODEL_SHA = '807f8d041375412280e39a63c640305e41008fdfbe5b956cf8d22a540783b7f0'
CALLER_PROFILE = {
    'path': 'comparison/results/formula-campaign-004/group01/8065/context-model/original-caller-profile.json',
    'sha256': 'afa74aa06d2e186dcfd81a750edca2d21a6fd0381494fb6ac8334fb900b38387',
}


class ScopeError(ValueError):
    """The source-derived finite diagnostic cannot establish this case."""


def finite(v):
    if v & 0x7f800000 == 0x7f800000:
        raise ScopeError('nonfinite arithmetic: range/retry semantics remain OPEN')
    return arithmetic.fraction(v)


def rounded(v, negative_zero=False):
    word = arithmetic.binary32(v, negative_zero)
    finite(word)
    return word


def add(a, b):
    return rounded(finite(a) + finite(b), a == b == 0x80000000)


def sub(a, b):
    return add(a, b ^ 0x80000000)


def mul(a, b):
    return rounded(finite(a) * finite(b), bool((a ^ b) >> 31))


def div(a, b):
    if not finite(b):
        raise ScopeError('division by zero: original/native nonfinite policy unproved')
    return rounded(finite(a) / finite(b), bool((a ^ b) >> 31))


def fold(v, radius):
    # Original x87: abs(v+radius) - (abs(v-radius)+v), not 2*clamp-v.
    return sub(add(v, radius) & 0x7fffffff, add(sub(v, radius) & 0x7fffffff, v))


def radius(s):
    squares = [mul(s[f'xyz[{i}]'], s[f'xyz[{i}]']) for i in range(3)]
    return add(add(squares[0], squares[1]), squares[2])


def surf(s, params):
    """264 original CODE bytes: counter/vary, XY folds, sphere, Julia, rows."""
    previous = s['vary_scale']
    if signed(s['first']) <= 0:
        s['first'] = (s['first'] + 1) & 0xffffffff
        previous = params[0]
    vary = add(previous, mul(sub(previous & 0x7fffffff, ONE), params[13]))
    s['vary_scale'] = vary
    x, y, z = (s[f'xyz[{i}]'] for i in range(3))
    x, y = fold(x, params[3]), fold(y, params[3])
    rr = add(mul(y, y), mul(x, x))  # CODE 005f..0067: Y square then X.
    if params[14] == 0:
        rr = add(rr, mul(z, z))
    if finite(rr) < finite(params[2]):
        scale = params[1]
        finite(scale)
    elif finite(rr) <= 1:
        scale = div(vary, rr)
    else:
        scale = vary
    s['xyz[3]'] = mul(s['xyz[3]'], scale)
    vec = [add(mul(v, scale), s[f'j[{i}]']) for i, v in enumerate((x, y, z))]
    for row in range(3):
        products = [mul(vec[i], params[4 + 3 * row + i]) for i in range(3)]
        s[f'xyz[{row}]'] = add(add(products[0], products[1]), products[2])


def box(s, params):
    """153 original CODE bytes, no C addition; PVar[-24,-32,-16]."""
    xyz = [fold(s[f'xyz[{i}]'], ONE) for i in range(3)]
    rr = add(add(mul(xyz[2], xyz[2]), mul(xyz[1], xyz[1])), mul(xyz[0], xyz[0]))
    if finite(rr) < finite(params[1]):
        scale = params[0]
    elif finite(rr) <= 1:
        scale = div(params[2], rr)
    else:
        scale = params[2]
    s['xyz[3]'] = mul(s['xyz[3]'], scale)
    for i, value in enumerate(xyz):
        s[f'xyz[{i}]'] = mul(value, scale)


def signed(v):
    return v - (1 << 32) if v & 0x80000000 else v


def decode(v):
    if not isinstance(v, str) or len(v) != 10 or not v.startswith('0x') or any(c not in '0123456789abcdef' for c in v[2:]):
        raise ScopeError('expected canonical 32-bit hexadecimal word')
    return int(v, 16)


def validate(case):
    if set(case) != {'identity', 'kind', 'input_state_bits', 'parameter_bits', 'context'}:
        raise ScopeError('explicit mixed context semantic input required')
    if set(case['input_state_bits']) != set(STATE) or len(case['parameter_bits']) != 2048:
        raise ScopeError('wrong state/program width')
    state = {k: decode(v) for k, v in case['input_state_bits'].items()}
    program = [decode(v) for v in case['parameter_bits']]
    binding.validate(case, program, [state[k] for k in STATE])
    for lane in STATE[:33]:
        finite(state[lane])
    # Pascal initializes three Julia lanes; this diagnostic does not invent a
    # claim about the fourth-lane difference of an arbitrary incoming state.
    if state['j[3]'] != state['original_j[3]']:
        raise ScopeError('initial J4 outside actual scene preparation')
    return state, program


def evaluate_semantics(case):
    s, p = validate(case)
    params = []
    for slot in range(3):
        base = 56 + slot * 332
        params.append(p[base + 12:base + 12 + p[base + 4]])
    for i in range(3):
        s[f'xyz[{i}]'] = s[f'c[{i}]']
        s[f'j[{i}]'] = s[f'original_j[{i}]']
    s['xyz[3]'] = ONE
    s['first'] = 0
    s['iteration'] = 0
    s['rout'] = radius(s)
    s['otrap'] = s['rout']
    slot, remaining, work = 0, 30, 0
    s['tmp'] = remaining
    visits = [0] * 6
    trace = []
    # 400 counted passes plus at most12 uncounted modifiers; the authored
    # limit stays400. A diagnostic safeguard cannot become an early result.
    for _ in range(1024):
        s['rold'] = s['rout']
        if remaining <= 0:
            slot = (slot + 1) % 3
            remaining = s[f'hybrid_counts[{slot}]'] & 0x7fffffff
        s['tmp'] = remaining
        s['current_slot'] = slot
        before = {k: arithmetic.word(s[k]) for k in KNOWN_FIELDS}
        work += 1
        visits[slot] += 1
        if slot == 0:
            surf(s, params[0])
        elif slot == 1:
            for i in range(3):
                s[f'xyz[{i}]'] = add(s[f'xyz[{i}]'], params[1][i])
        else:
            box(s, params[2])
        after = {k: arithmetic.word(s[k]) for k in KNOWN_FIELDS}
        remaining -= 1
        s['tmp'] = remaining
        counted = signed(s[f'hybrid_counts[{slot}]']) >= 0
        if counted:
            s['iteration'] += 1
            s['rout'] = radius(s)
            if finite(s['rout']) < finite(s['otrap']):
                s['otrap'] = s['rout']
        trace.append({'work': work, 'slot': slot, 'formula_id': (1509, 8065, 7039)[slot],
                      'counted': counted, 'remaining': remaining,
                      'iteration': s['iteration'], 'before': before, 'after': after})
        if s['iteration'] >= s['max_iterations'] or finite(s['rout']) > finite(s['rstop']):
            break
    else:
        raise ScopeError('source scheduler exceeded diagnostic bound')
    # The independently captured original DE2 epilogue loads Rout and W from
    # memory before SQRTSD/abs/DIVSD. Both upper64 lanes are overwritten with
    # zero, independent of entry poison/carry. Native targets apply RN32 at
    # each primitive; original binary64 results are never copied as targets.
    absolute_w = s['xyz[3]'] & 0x7fffffff
    s['sse[0]'] = div(affine.sqrt32(s['rout']), absolute_w)
    s['sse[1]'], s['sse[2]'], s['sse[3]'] = 0, absolute_w, 0
    known = {k: arithmetic.word(s[k]) for k in STATE}
    return {
        'schema': 'mb3d.native-formula-context-semantic-diagnostic.v2',
        'status': 'SOURCE_MODEL_READY_PENDING_GPU', 'qualification': False,
        'expected_state_bits': known,
        'unproved_fields': {}, 'trace_fields': list(KNOWN_FIELDS),
        'expected_work': work, 'expected_slot_visits': visits,
        'expected_formula_visits': {'1509': visits[0], '8065': visits[1], '7039': visits[2]},
        'target_and_successor_reached': visits[1] > 0 and visits[2] > 0,
        'expected_fault_code': 0, 'trace': trace,
        'limitations': ['source-ordered RN32, not universally contraction-invariant',
                        'full47 final fields; intermediate source trace has43 non-SSE fields',
                        'no distance scalar, range-retry, frame or effect qualification'],
    }


def evaluate(case):
    return evaluate_semantics(case)['expected_state_bits']


def source_manifest():
    pins = dict(SOURCE_PINS)
    pins['comparison/tools/native_formula_reference.py'] = PRIMITIVE_SHA
    pins['comparison/tools/native_formula_affine_v2_reference.py'] = AFFINE_MODEL_SHA
    pins.update(affine.DEPENDENCY_PINS)
    pins['src/render/native_metal/formula_abi.h'] = ABI_SHA
    for path, expected in pins.items():
        if hashlib.sha256((ROOT / path).read_bytes()).hexdigest() != expected:
            raise ScopeError('source/helper pin changed: ' + path)
    # The new profile and every captured input/output/build/disassembly record
    # are explicit data dependencies, checked before any diagnostic dispatch.
    import json
    profile_path = ROOT / CALLER_PROFILE['path']
    if hashlib.sha256(profile_path.read_bytes()).hexdigest() != CALLER_PROFILE['sha256']:
        raise ScopeError('original mixed caller profile changed')
    profile = json.loads(profile_path.read_text())
    if profile['status'] != 'PASS_SOURCE_BOUND_RETURN_TRANSPORT' or profile['qualification'] is not False:
        raise ScopeError('original mixed caller profile is not a scoped source proof')
    affine.manifest()  # Recheck the independently reviewed sqrt helper closure.
    data = {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in affine.data_paths()}
    data[CALLER_PROFILE['path']] = CALLER_PROFILE['sha256']
    for record in profile['artifacts']:
        path = (ROOT / record['path']).resolve()
        if not path.is_relative_to(ROOT) or hashlib.sha256(path.read_bytes()).hexdigest() != record['sha256']:
            raise ScopeError('original mixed caller proof dependency changed')
        data[record['path']] = record['sha256']
    return {'source_hashes': {k: pins[k] for k in SOURCE_PINS},
            'helper_hashes': {k: v for k, v in pins.items() if k not in SOURCE_PINS},
            'context_dependencies': {str(path.relative_to(ROOT)): hashlib.sha256(path.read_bytes()).hexdigest()
                                     for path in binding.dependency_paths()},
            'data_dependencies': data,
            'qualification': False, 'status': 'SOURCE_MODEL_READY_FOR_DIAGNOSTIC'}


def program_words():
    _, ctx = binding.context()
    path = ROOT / ctx['program_artifacts'][0]['path']
    raw = path.read_bytes()
    if hashlib.sha256(raw).hexdigest() != binding.PROGRAM_SHA:
        raise ScopeError('prepared scene program changed')
    return list(struct.unpack('<2048I', raw))


def input_case(point, poison=0):
    p = program_words()
    s = dict.fromkeys(STATE, 0)
    for i in range(3):
        s[f'c[{i}]'] = rounded(Fraction(point[i]))
    for i in range(4):
        s[f'j[{i}]'] = s[f'original_j[{i}]'] = p[24 + i]
    s['derivative[0]'] = ONE
    s['vary_scale'] = ONE
    s['rstop'] = p[16]
    s['max_iterations'], s['julia'], s['inside'], s['calc_smooth'] = p[7], p[8], p[9], p[10]
    for i in range(6):
        s[f'hybrid_counts[{i}]'] = p[50 + i]
    if poison:
        for i, name in enumerate(('derivative[1]', 'derivative[2]', 'derivative[3]', 'dfree1', 'dfree2', 'smooth', 'sse[0]', 'sse[1]', 'sse[2]', 'sse[3]')):
            s[name] = rounded(Fraction((-1) ** i * (i + poison), 8))
        s['first'], s['tmp'], s['iteration'], s['current_slot'] = 7, 8, 9, 5
    return {'identity': binding.IDENTITY, 'kind': 'orbit',
            'input_state_bits': {k: arithmetic.word(s[k]) for k in STATE},
            'parameter_bits': list(map(arithmetic.word, p)), 'context': binding.descriptor()}


# Source-only candidate points declared before any actual GPU observation.
# No point is removed because a device disagrees. Domain/reachability outcomes
# are retained, including early exits and source-model scope failures.
POINTS = ((0, 0, 0), (1, 0, 0), (-1, 0, 0), (0, 1, 0),
          (0, 0, 1), (1, 1, 1), (-1, -1, -1), (4, 2, 1))


def build_cases():
    source_manifest()
    rows = []
    for i, point in enumerate(POINTS):
        case = input_case(point, poison=i % 2)
        row = {'id': f'translate-lightbulbmoon-{i:02d}', 'identity': binding.IDENTITY,
               'variant': f'original-context-point-{point}', 'input': case,
               'source_hashes': SOURCE_PINS, 'qualification': False}
        try:
            row['semantic_diagnostic'] = evaluate_semantics(case)
        except ScopeError as error:
            row['semantic_diagnostic'] = {'status': 'OPEN_MODEL_DOMAIN',
                                          'qualification': False, 'reason': str(error)}
        rows.append(row)
    # Carry all47 independently predicted words, then replace only the next
    # point C. These are source-derived Native targets, never the original
    # binary64 diagnostic outputs or candidate GPU observations.
    for source_index, point_index in ((3, 4), (4, 3)):
        previous = rows[source_index]['semantic_diagnostic']
        case = input_case(POINTS[point_index], poison=3)
        original_c = {f'c[{i}]': case['input_state_bits'][f'c[{i}]'] for i in range(3)}
        case['input_state_bits'].update(previous['expected_state_bits'])
        case['input_state_bits'].update(original_c)
        rows.append({'id': f'translate-lightbulbmoon-carry-{source_index}-{point_index}',
                     'identity': binding.IDENTITY, 'variant': 'full47-carried-second-orbit',
                     'input': case, 'source_hashes': SOURCE_PINS, 'qualification': False,
                     'carry': {'source_case_id': rows[source_index]['id'],
                               'carried_fields': list(STATE),
                               'new_point_fields': list(original_c),
                               'unproved_fields': []},
                     'semantic_diagnostic': evaluate_semantics(case)})
    return rows
