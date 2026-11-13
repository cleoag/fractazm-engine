"""Original-derived campaign004 _Translate reference; no candidate observations.

Three Double parameters are independently projected to RN-even binary32 before
three source-ordered adds. The collected corpus is Translate8065 only. The
bounded DE2 caller retains exact radius operations and proves its rounded sqrt
transport separately. Mixed/4D callers are not silently accepted by evaluate().
"""
from fractions import Fraction
import hashlib
from itertools import combinations
import json
from pathlib import Path
import re
import tarfile

import native_formula_reference as arithmetic
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
FORMULAS = {'_Translate': (8065, -1, 3, 0, 1, 1)}
IDENTITY = ['external', 8065, '_Translate']
ONE, NEG_ZERO = 0x3f800000, 0x80000000
fp32, fraction, word = arithmetic.binary32, arithmetic.fraction, arithmetic.word
SOURCE_PINS = {
    'mb3d/M3Formulas/_Translate.m3f': '3acdcb61881b9007cd1a66a33887d8a13ae7541bd3120472b582e2ac9a09b706',
    'mb3d/M3Formulas/_TranslateC4d.m3f': 'aca72cf8b072cce46646ddcc4f0710787b34f3c669c746add7042c94e195505e',
    'mb3d/M3Formulas/_scaleC.m3f': 'e1d6898c8786942a79f0350afed1af7c38bfc8468b998198de6ad36d87a651ba',
    'mb3d/M3Formulas/_ScaleC4d.m3f': '8477ca02273adf254dd957dd1deac3c073187d66638abb2854feb61808a8bf1c',
    'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
    'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
    'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
    'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
}
ABI_PINS = {
    'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a',
    'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6',
    'src/render/native_metal/generated/formula_metadata.h': 'ca05eeb9e09c89e10338ffc412ad66e8c6b3b54ec9d708af2d81925ee612a6aa',
}
DEPENDENCY_PINS = {
    'comparison/tools/native_formula_reference.py': 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e',
    'comparison/tools/gpu_pipeline_proofs.py': '0d799de96e32cc89aa4fd50d3bbd16ddf73395d1812e7c637ae1b08d50944935',
    'comparison/tools/qualify_native_metal_all.py': 'a3bbdc663f3717294434abd2d8d61108f3fd71c0b48706cf617a2052a03171e8',
}
CALLER_PINS = {
    'comparison/results/native-formula-group05-20261104/caller-profile.json': '3b9231389e037f59f9c16f425575836cbe621c34c59a41cd577aae63405993f6',
    'comparison/results/native-formula-group05-20261104/caller-proof.tar.gz': 'b10f6b0d508c7842ab30321897f943d311fdd4ec980fa38aae6753071892b090',
}
DATA_PINS = {'tests/fixtures/native_affine_reference/manifest.json': 'dea3a019e704f67fe8d04498490609ed6323becc66679211dafe0ec48a8d0276'}
CODE_PINS = {
    '_Translate': ('69ddf3b24e9496a4f136773bc69c1879958c82129e1cde6143fbda234b1b2e95', 36),
    '_TranslateC4d': ('4932bcd4b96573dad22f96a30cfbf0b6f42e089e23059148583c4504322d6569', 83),
    '_scaleC': ('aa7a74b61a9f5e03aaa15a19bdddde58d50954e9291943a369a925d480579f2e', 44),
    '_ScaleC4d': ('6d197dcd907cd7187da523baa7305fc8c518343ea78a21595d42a49046781f71', 81),
}


class ScopeError(ValueError):
    """An unsupported requirement stays OPEN, never silently passes."""


def fraction64(bits):
    if type(bits) is not int or not 0 <= bits < 1 << 64:
        raise ValueError('expected uint64 parameter word')
    exponent = (bits >> 52) & 2047
    if exponent == 2047:
        raise ScopeError('nonfinite authored Double')
    significand = (bits & ((1 << 52)-1)) | ((1 << 52) if exponent else 0)
    shift = exponent-1023-52 if exponent else -1074
    value = Fraction(significand << shift) if shift >= 0 else Fraction(significand, 1 << -shift)
    return -value if bits >> 63 else value


def binary64(value, negative_zero=False):
    """Exact rational RN-even encoding for authored Double literals."""
    value = Fraction(value)
    if not value:
        return (1 << 63) if negative_zero else 0
    sign = (1 << 63) if value < 0 else 0
    value = abs(value); n, d = value.numerator, value.denominator
    exponent = n.bit_length()-d.bit_length()
    if (n < d << exponent) if exponent >= 0 else (n << -exponent < d):
        exponent -= 1
    shift = 1074 if exponent < -1022 else 52-exponent
    numerator, denominator = (n << shift, d) if shift >= 0 else (n, d << -shift)
    q, r = divmod(numerator, denominator)
    significand = q + int(2*r > denominator or (2*r == denominator and q & 1))
    if exponent < -1022:
        return sign | significand
    if significand == 1 << 53:
        significand >>= 1; exponent += 1
    if exponent > 1023:
        raise ScopeError('authored Double overflow')
    return sign | ((exponent+1023) << 52) | (significand & ((1 << 52)-1))


def project_double(bits):
    result = fp32(fraction64(bits), bits == 1 << 63)
    fraction(result)  # Overflow cannot masquerade as a successful finite case.
    return result


def _sqrt_round(value, width):
    """Choose the adjacent representable square roots using exact inequalities."""
    if value < 0:
        raise ScopeError('negative square-root operand')
    if not value:
        return 0
    decode = fraction if width == 32 else fraction64
    low, high = 0, 0x7f7fffff if width == 32 else 0x7fefffffffffffff
    while low < high:
        middle = (low+high+1)//2
        if decode(middle)**2 <= value:
            low = middle
        else:
            high = middle-1
    if decode(low)**2 == value:
        return low
    upper = low+1
    midpoint = (decode(low)+decode(upper))/2
    side = value-midpoint**2
    return low if side < 0 or (side == 0 and low & 1 == 0) else upper


def sqrt32(bits):
    value = fraction(bits)
    if value < 0:
        raise ScopeError('negative square-root operand')
    return NEG_ZERO if bits == NEG_ZERO else _sqrt_round(value, 32)


def sqrt_transport(bits):
    """Prove RN64 SQRTSD then RN32 agrees with direct RN32 for this input.

    DE2 Translate resets w=1 and never changes it, so DIVSD by abs(w) is exact.
    This is an arithmetic extension derived from the pinned epilogue, not a
    claim that its historical exact-root controls measured irrational roots.
    """
    rounded = sqrt32(bits)
    original_double = (1 << 63) if bits == NEG_ZERO else _sqrt_round(fraction(bits), 64)
    if project_double(original_double) != rounded:
        raise ScopeError('binary64-to-binary32 square-root transport discrepancy')
    return rounded


def _state(state):
    if set(state) != set(STATE) or any(type(v) is not int or not 0 <= v <= 0xffffffff for v in state.values()):
        raise ValueError('full47 uint32 state required')
    for lane in STATE[:33]:
        fraction(state[lane])


def translate_step(state, authored_double_bits, slot=0):
    """Full-state step oracle reusable at a proved mixed callback boundary.

    CODE offsets0x0a..0x1d push z/y/x; x+=PVar−16, y+=PVar−24, z+=PVar−32.
    No other original state or SSE register is touched. current_slot is the
    observation adapter's selected callback position, not a CODE memory write.
    """
    _state(state)
    if len(authored_double_bits) != 3 or type(slot) is not int or not 0 <= slot < 6:
        raise ValueError('Translate parameter/slot arity')
    parameters = [project_double(v) for v in authored_double_bits]
    result = dict(state)
    for i in range(3):
        result[f'xyz[{i}]'] = arithmetic.add(state[f'xyz[{i}]'], parameters[i])
        fraction(result[f'xyz[{i}]'])
    result['current_slot'] = slot
    return result


def prepared_translate_parameters(program_words, slot=0):
    """Decode and verify every source-used typed alias in a full prepared program.

    Unused payload storage is retained by the caller's context binding. It is
    not zeroed or substituted for a point-probe context here.
    """
    if len(program_words) != 2048 or type(slot) is not int or not 0 <= slot < 6:
        raise ValueError('prepared program/slot width')
    if any(type(v) is not int or not 0 <= v <= 0xffffffff for v in program_words):
        raise ValueError('prepared program must contain uint32 words')
    at = 56+332*slot
    if program_words[at:at+12] != [1, 1, 8065, 0xffffffff, 3, 0, 1, 0, 0, 1, 0, 0]:
        raise ValueError('Translate descriptor differs from pinned transport metadata')
    values = [program_words[at+76+2*i] | (program_words[at+77+2*i] << 32) for i in range(3)]
    for i, bits in enumerate([binary64(Fraction(1, 2)), *values]):
        projected = project_double(bits)
        offset = at+204+62-2*i
        if program_words[offset:offset+2] != [bits & 0xffffffff, bits >> 32] or program_words[at+268+i] != projected:
            raise ValueError('Prepared PVar/raw/negative alias differs')
        if i and program_words[at+12+i-1] != projected:
            raise ValueError('Prepared binary32 parameter projection differs')
    return values


def program(parameters, s):
    if len(parameters) != 3:
        raise ValueError('three authored Double parameters required')
    p = [0]*2048
    p[:15] = [1, 1, 6, 0, 0, 0, 2, s['max_iterations'], s['julia'], s['inside'], s['calc_smooth'], 256, 256, 0, 0]
    p[16], p[18], p[21], p[22] = s['rstop'], ONE, fp32(1 << 20), ONE
    p[24:28] = [s[f'original_j[{i}]'] for i in range(4)]
    for i in range(4): p[28+5*i] = ONE
    p[50] = s['hybrid_counts[0]']
    for slot in range(6): p[56+332*slot] = 1
    p[56:68] = [1, 1, 8065, 0xffffffff, 3, 0, 1, 0, 0, 1, 0, 0]
    for i, bits in enumerate([binary64(Fraction(1, 2)), *parameters]):
        projected = project_double(bits)
        offset = 56+204+62-2*i
        p[offset:offset+2] = [bits & 0xffffffff, bits >> 32]
        p[56+268+i] = projected
        if i:
            p[68+i-1] = projected
            p[132+2*(i-1):134+2*(i-1)] = [bits & 0xffffffff, bits >> 32]
    return p


def exact_radius(s):
    squares = []
    for i in range(3):
        value = fraction(s[f'xyz[{i}]'])
        square = arithmetic.multiply(s[f'xyz[{i}]'], s[f'xyz[{i}]'])
        if fraction(square) != value*value:
            raise ScopeError('rounded or underflowing radius product remains outside caller domain')
        squares.append(square)
    for n in (2, 3):
        for subset in combinations(squares, n):
            exact = sum(map(fraction, subset), Fraction())
            if fraction(fp32(exact)) != exact:
                raise ScopeError('rounded radius reduction remains outside caller domain')
    return fp32(sum(map(fraction, squares), Fraction()))


def orbit(s, parameters):
    _state(s)
    if fraction(s['rstop']) <= 0:
        raise ScopeError('prepared radius_stop must be finite and positive')
    if s['inside'] or s['calc_smooth'] or s['julia'] not in (0, 1):
        raise ScopeError('only unsmoothed exterior Julia/Mandel DE2 caller is proved')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3 or any(s[f'hybrid_counts[{i}]'] for i in range(1, 6)):
        raise ScopeError('only positive single-slot bounded caller is proved')
    if s['vary_scale'] != ONE or s['j[3]'] != s['original_j[3]']:
        raise ScopeError('entry VaryScale/J4 differs from bounded caller profile')
    out = dict(s)
    for i in range(3):
        out[f'xyz[{i}]'] = s[f'c[{i}]']
        out[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    out['xyz[3]'] = ONE
    out['first'] = out['iteration'] = 0
    out['rout'] = out['otrap'] = exact_radius(out)
    count = remaining = s['hybrid_counts[0]']; out['tmp'] = remaining
    for _ in range(s['max_iterations']):
        out['rold'] = out['rout']
        if remaining == 0: remaining = count
        out = translate_step(out, parameters)
        remaining -= 1; out['tmp'] = remaining; out['iteration'] += 1
        out['rout'] = exact_radius(out)
        if fraction(out['rout']) < fraction(out['otrap']): out['otrap'] = out['rout']
        if fraction(out['rout']) > fraction(s['rstop']): break
    out['sse[0]'], out['sse[1]'], out['sse[2]'], out['sse[3]'] = sqrt_transport(out['rout']), 0, ONE, 0
    return out


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'} or payload['identity'] != IDENTITY:
        raise ValueError('Translate-only four-key semantic input required')
    if payload['kind'] not in ('point', 'orbit') or set(payload['input_state_bits']) != set(STATE):
        raise ValueError('wrong kind/state fields')
    def decode(v):
        if not isinstance(v, str) or not re.fullmatch(r'0x[0-9a-f]{8}', v):
            raise ValueError('canonical hex32 required')
        return int(v, 16)
    s = {k: decode(v) for k, v in payload['input_state_bits'].items()}
    p = [decode(v) for v in payload['parameter_bits']]
    _state(s)
    if fraction(s['rstop']) <= 0:
        raise ScopeError('prepared radius_stop must be finite and positive')
    parameters = prepared_translate_parameters(p)
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3 or any(s[f'hybrid_counts[{i}]'] for i in range(1, 6)):
        raise ScopeError('bounded single-slot descriptor counts')
    if any(s[k] not in (0, 1) for k in ('julia', 'inside', 'calc_smooth')) or p != program(parameters, s):
        raise ValueError('prepared single-slot program differs')
    out = orbit(s, parameters) if payload['kind'] == 'orbit' else translate_step(s, parameters)
    return {k: word(out[k]) for k in STATE}


def dependency_paths():
    return [ROOT/p for p in DEPENDENCY_PINS]


def data_paths():
    return [ROOT/p for p in (*SOURCE_PINS, *ABI_PINS, *CALLER_PINS, *DATA_PINS)]


def manifest():
    def check(pins):
        result = {}
        for path, expected in pins.items():
            raw = (ROOT/path).read_bytes()
            if hashlib.sha256(raw.replace(b'\r\n', b'\n')).hexdigest() != expected:
                raise ValueError('pinned source/helper/profile changed: '+path)
            result[path] = hashlib.sha256(raw).hexdigest()
        return result
    sources, abi, dependencies = check(SOURCE_PINS), check(ABI_PINS), check(DEPENDENCY_PINS)
    # The gzip archive is binary; unlike textual source, never newline-normalize it.
    caller = {}
    for path, expected in CALLER_PINS.items():
        actual = hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
        if actual != expected: raise ValueError('pinned original caller evidence changed: '+path)
        caller[path] = actual
    codes = {}
    for name, (expected, size) in CODE_PINS.items():
        text = (ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code = bytes.fromhex(re.search(r'\[CODE\](.*?)\[END\]', text, re.S)[1])
        if len(code) != size or hashlib.sha256(code).hexdigest() != expected:
            raise ValueError('original affine CODE changed: '+name)
        codes[name] = {'size': size, 'sha256': expected}
    with tarfile.open(ROOT/'comparison/results/native-formula-group05-20261104/caller-proof.tar.gz') as archive:
        members = {name: hashlib.sha256(archive.extractfile(name).read()).hexdigest() for name in (
            'analytic-caller/compile/original_analytic_oracle.exe',
            'analytic-caller/compile/original_analytic_oracle.pas',
            'analytic-caller/compile/original_analytic_oracle.s',
            'analytic-caller/analytic-pe-disassembly.txt')}
    return {'schema': 'mb3d.native-formula-affine-v2-reference.v1', 'semantic_sources': sources,
        'abi_sources': abi, 'dependencies': dependencies, 'input_audits': check(DATA_PINS), 'caller_profile': caller, 'caller_members': members,
        'original_code': codes, 'model_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest(),
        'arithmetic': 'Exact rational RN-even binary32 adds after independent RN64-authored-Double to RN32 projection; explicit signed zeros; exact radius/all subset sums; rational midpoint sqrt32 and RN64-to-RN32 transport witness.',
        'model_limits': ['Collected identity8065 only; full47 state words; source-used typed aliases checked.',
            'Point arithmetic includes finite normal/subnormal inputs and results; nonfinite/overflow have separate OPEN fault obligations.',
            'Orbit: positive count1..3, maxIt1..8, analytic3D DE2, no smoothing/inside/interpolation/combination; VaryScale1 and J4=originalJ4 entry.',
            'Orbit radius products/all subset sums must be exact; sqrt may be irrational but its binary64 projection must equal RN32.',
            'Compiled profile historical measurements had exact roots; rounded sqrt targets are an independent arithmetic extension, not new original execution.',
            'Actual mixed scene, negative-count caller transport, other DEoptions, 4D register transport and full-scene/range acceptance remain separately REQUIRED/OPEN.',
            'No GPU-derived target, no nativeCPU/x87 parity, no complete formula qualification.']}


def state():
    s = {k: fp32(Fraction(i+1, 32)) if i < 33 else i+11 for i, k in enumerate(STATE)}
    s.update(iteration=19, first=17, tmp=23, max_iterations=4, julia=0, inside=0, calc_smooth=0, current_slot=5)
    for i in range(6): s[f'hybrid_counts[{i}]'] = int(i == 0)
    for i, value in enumerate((3, 4, 12, -2)):
        for group, factor in (('xyz', 1), ('c', 1), ('j', 2), ('original_j', -2)):
            s[f'{group}[{i}]'] = fp32(value*factor)
    s['j[3]'] = s['original_j[3]']; s['vary_scale'] = ONE
    s['rstop'] = fp32(1 << 20)
    return s


def build_cases():
    cases = []; provenance = manifest()
    def append(tag, s, parameters, kind='point', domains=()):
        p = program(parameters, s)
        payload = {'identity': IDENTITY, 'kind': kind,
            'input_state_bits': {k: word(s[k]) for k in STATE}, 'parameter_bits': list(map(word, p))}
        writes = {f'xyz[{i}]' for i in range(3)} | {'current_slot'}
        reads = {f'xyz[{i}]' for i in range(3)}
        if kind == 'orbit':
            writes |= {f'sse[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)} | {'xyz[3]', 'first', 'iteration', 'tmp', 'rout', 'rold', 'otrap'}
            reads |= {f'{g}[{i}]' for g in ('c', 'original_j') for i in range(3)} | {'julia', 'rstop', 'max_iterations', 'hybrid_counts[0]'}
        cases.append({'id': '8065-v2-'+tag, 'identity': IDENTITY, 'variant': tag, 'input': payload,
            'expected_state_bits': evaluate(payload), 'read_lanes': [k for k in STATE if k in reads],
            'write_lanes': [k for k in STATE if k in writes], 'domains': list(domains),
            'source_hashes': provenance['semantic_sources']})
    zeros = [0]*3
    defaults = state(); defaults['rstop'] = fp32(16)
    append('authored-defaults', defaults, zeros, domains=('authored_defaults', 'state'))
    s = state(); s.update(first=0x80000000, iteration=0x7fffffff, tmp=0xffffffff)
    append('integer-state-sentinels', s, zeros, domains=('state',))
    for x, y in ((0, 0), (0, NEG_ZERO), (NEG_ZERO, 0), (NEG_ZERO, NEG_ZERO)):
        s = state()
        for i in range(3): s[f'xyz[{i}]'] = x
        append(f'zero-signs-{x:08x}-{y:08x}', s, [binary64(fraction(y), y == NEG_ZERO)]*3,
               domains=('branches_boundaries', 'state'))
    for start in (0x3f800000, 0x3f800001, 0xbf800000, 0xbf800001):
        s = state()
        for i in range(3): s[f'xyz[{i}]'] = start
        delta = Fraction(1 if start >> 31 == 0 else -1, 1 << 24)
        append(f'rounding-tie-{start:08x}', s, [binary64(delta)]*3, domains=('rounded_finite', 'branches_boundaries'))
    append('decimal-Double-parameters', state(), [binary64(v) for v in (Fraction(1,10), Fraction(-1,5), Fraction(1,3))],
           domains=('parameters_preparation', 'rounded_finite'))
    s = state(); s['xyz[0]'] = ONE
    append('Double-before-FP32-midpoint', s, [binary64(Fraction(1,1 << 24)+Fraction(1,1 << 53)), 0, 0],
           domains=('parameters_preparation', 'rounded_finite'))
    s = state(); s['xyz[0]'] = ONE
    append('Double-below-FP32-midpoint', s, [binary64(Fraction(1,1 << 24)-Fraction(1,1 << 53)), 0, 0],
           domains=('parameters_preparation', 'rounded_finite'))
    for label, coordinate, delta in (
        ('maxfinite-identity', 0x7f7fffff, 0), ('maxfinite-cancel', 0x7f7fffff, -fraction(0x7f7fffff)),
        ('minnormal-minus-largest-subnormal', 0x00800000, -fraction(0x007fffff)),
        ('minimum-subnormal-add', 1, fraction(1)), ('negative-subnormal-add', 0x80000001, -fraction(1)),
        ('normal-subnormal-boundary', 0x007fffff, fraction(1)), ('tiny-Double-rounds-zero', NEG_ZERO, -Fraction(1,1 << 1074))):
        s = state(); s['xyz[0]'] = coordinate
        append(label, s, [binary64(delta), 0, 0], domains=('branches_boundaries', 'rounded_finite', 'parameters_preparation'))
    s = state()
    for i in range(3): s[f'j[{i}]'] = fp32((1 << 25)*(-1 if i & 1 else 1))
    append('large-J-does-not-compensate-XYZ', s, [binary64(v) for v in (-3.5, 0, 0)], domains=('state', 'parameters_preparation'))
    for count in (1, 2, 3):
        for limit in (1, 2, 8):
            for julia in (0, 1):
                s = state(); s['hybrid_counts[0]'] = count; s['max_iterations'] = limit; s['julia'] = julia
                append(f'orbit-c{count}-n{limit}-j{julia}', s, [binary64(v) for v in (3, 4, 12)], 'orbit', ('callers_hybrids', 'state'))
    for tag, c, delta, stop, limit in (
        ('orbit-formula-default-stop16', (1,2,3), (0,0,0), 16, 4),
        ('orbit-zero-translation-equality', (3,4,12), (0,0,0), 169, 4),
        ('orbit-zero-radius', (0,0,0), (0,0,0), 16, 2),
        ('orbit-strict-bailout', (3,4,12), (3,4,12), 676, 4),
        ('orbit-trap-minimum', (3,4,12), (-3,-4,-12), 1 << 20, 3),
        ('orbit-rounded-sqrt2', (1,1,0), (0,0,0), 2, 2),
        ('orbit-rounded-sqrt3', (1,1,1), (0,0,0), 3, 2)):
        s = state(); s['max_iterations'] = limit; s['rstop'] = fp32(stop)
        for i, v in enumerate(c): s[f'c[{i}]'] = fp32(v)
        append(tag, s, [binary64(v) for v in delta], 'orbit', ('authored_defaults', 'callers_hybrids', 'branches_boundaries', 'rounded_finite'))
    # Historical inputs are retained verbatim as controls, but expected outputs
    # are regenerated through this independent caller implementation.
    for old in arithmetic.build_cases():
        if old['identity'] == IDENTITY and old['input']['kind'] == 'orbit':
            s = {k: int(v,16) for k,v in old['input']['input_state_bits'].items()}
            params = prepared_translate_parameters([int(v,16) for v in old['input']['parameter_bits']])
            append('legacy-'+old['variant'], s, params, 'orbit', ('callers_hybrids', 'state', 'rounded_finite'))
            if cases[-1]['input'] != old['input']:
                raise ValueError('historical Translate control input changed')
    return cases
