"""Original group05 IFS CODE oracles over exact prepared-parameter domains.

Targets precede device observations. Matrix products, subset sums, scaling and
Julia additions must be exact binary32; signed-zero operations remain explicit.
Prepared matrices do not establish arbitrary authored-angle preparation. The
analytic caller is restricted to the separately proved positive-count DE2 ABI.
"""
from fractions import Fraction
import hashlib
from itertools import combinations, product as cartesian_product
from math import isqrt
from pathlib import Path
import re
import struct

import native_formula_reference as arithmetic
import native_group05_scaling_reference as caller
import native_matrix_reference as matrix
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
FORMULAS = {'genIFS': (429, 2, 16, 0, 0, 1), 'SierpHilbert': (590, 2, 24, 0, 1, 1)}
ABI_PINS = dict(caller.ABI_PINS, **{
    'src/render/native_metal/generated/formula_metadata.h': 'ca05eeb9e09c89e10338ffc412ad66e8c6b3b54ec9d708af2d81925ee612a6aa',
})
SOURCE_PINS = {k: v for k, v in caller.SOURCE_PINS.items() if not k.endswith('/_Scaling.m3f')}
SOURCE_PINS.update({
    'mb3d/M3Formulas/genIFS.m3f': '8cef62cc4bc5b70ca6ce4e2495f7749e6b08b526e341ad7e97af467d35c787fd',
    'mb3d/M3Formulas/SierpHilbert.m3f': 'c61dce70c9b9005eb41aa420fcb532ba4146ff302963f964d2743076f6ed0468',
    'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
})
DEPENDENCY_PINS = {
    'comparison/tools/native_formula_reference.py': 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e',
    'comparison/tools/native_group05_scaling_reference.py': 'f1ff2664e6164fc7564bea07a0316a8afaf782da348cb9ea2e0e8c5658fca77e',
    'comparison/tools/native_matrix_reference.py': '02da663186287cf810d0ba976af25ab32570b5235623d4cba4acd2dd4a941811',
}
CODE_SHA256 = '384dfd2c5c57b350fb9f4d4ce573df057cefa1ae9f9380ddeff94c9633f9f3f1'
CODE_PINS = {'genIFS': (143, CODE_SHA256),
             'SierpHilbert': (289, '8117d2d6884d96bc759c8eccb9c1530b07d75afebdd04a53c678585777a6b285')}
LAYOUT = [(-16, 'double')] + [(-20-4*i, 'integer') for i in range(3)] + [(-32-4*i, 'single') for i in range(12)]
LAYOUTS = {'genIFS': LAYOUT,
           'SierpHilbert': [(-16-8*i, 'double') for i in range(4)] +
               [(-44-4*i, 'single') for i in range(18)] + [(-120, 'double'), (-128, 'double')]}
fp32, fraction, word = arithmetic.binary32, arithmetic.fraction, arithmetic.word
ONE, NEG_ZERO = caller.ONE, caller.NEG_ZERO
ScopeError = caller.ScopeError


def exact_mul(a, b):
    result = caller.product(a, b)
    if fraction(result) != fraction(a)*fraction(b):
        raise ScopeError('rounded formula product')
    return result


def exact_add(a, b):
    caller.normal_or_zero(a); caller.normal_or_zero(b)
    result = arithmetic.add(a, b)
    if caller.normal_or_zero(result) != fraction(a)+fraction(b):
        raise ScopeError('rounded formula sum')
    return result


def dot(values, row):
    terms = [exact_mul(a, b) for a, b in zip(values, row)]
    for count in (2, 3):
        for subset in combinations(terms, count):
            total = sum(map(fraction, subset), Fraction())
            if caller.normal_or_zero(fp32(total)) != total:
                raise ScopeError('rounded matrix subset sum')
    # Original row reduction order, including the signs of zero products.
    return exact_add(exact_add(terms[0], terms[1]), terms[2])


def sierp_folds(values):
    out = list(values)
    for a, b in ((0, 1), (0, 2), (1, 2)):
        # Original FCOM(-a,b), C0/JB: equality takes the fold. FCHS flips
        # the sign bit; substituting 0-value loses the sign of zero.
        if fraction(out[a] ^ NEG_ZERO) >= fraction(out[b]):
            out[a], out[b] = out[b] ^ NEG_ZERO, out[a] ^ NEG_ZERO
    return out


def sierp_point(s, params):
    if len(params) != 24:
        raise ValueError('wrong SierpHilbert parameter count')
    xyz = [s[f'xyz[{i}]'] for i in range(3)]
    first = {row: dot(xyz, params[4+row*3:7+row*3]) for row in (2, 0, 1)}
    folded = sierp_folds([first[i] for i in range(3)])
    for index, edge in ((1, params[22]), (0, params[23])):
        if fraction(edge) > 0:
            delta = exact_add(edge, folded[index] ^ NEG_ZERO)
            folded[index] = exact_add((delta & 0x7fffffff) ^ NEG_ZERO, edge)
    second = {row: dot(folded, params[13+row*3:16+row*3]) for row in (2, 0, 1)}
    out = dict(s)
    out['xyz[3]'] = exact_mul(s['xyz[3]'], params[0])
    minus_one = exact_add(params[0], ONE ^ NEG_ZERO)
    for i in range(3):
        correction = exact_mul(minus_one, params[1+i])
        out[f'xyz[{i}]'] = exact_add(exact_mul(second[i], params[0]), correction ^ NEG_ZERO)
    out['current_slot'] = 0
    return out


def point(s, params, name='genIFS'):
    if name == 'SierpHilbert':
        return sierp_point(s, params)
    if name != 'genIFS':
        raise ValueError('unknown original formula')
    if len(params) != 16:
        raise ValueError('wrong genIFS parameter count')
    u = [exact_add(s[f'xyz[{i}]'] & (0x7fffffff if params[1+i] else 0xffffffff),
                   params[4+i]) for i in range(3)]
    # CODE computes rows2,0,1 from the same original u. Writes occur afterward.
    rows = {}
    for row in (2, 0, 1):
        rows[row] = dot(u, params[7+row*3:10+row*3])
    out = dict(s)
    out['xyz[3]'] = exact_mul(s['xyz[3]'], params[0])
    for i in range(3):
        out[f'xyz[{i}]'] = exact_add(exact_mul(rows[i], params[0]), s[f'j[{i}]'])
    out['current_slot'] = 0  # Probe wrapper, not an original CODE write.
    return out


def orbit(s, params, name='genIFS'):
    # Both original callbacks balance x87 and do not use SSE, so the independently
    # compiled positive-count doHybridPasDE caller's final transport applies.
    out = dict(s)
    for i in range(3):
        out[f'xyz[{i}]'] = s[f'c[{i}]']
        out[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    out['xyz[3]'] = ONE
    out['first'] = out['iteration'] = 0
    out['rout'] = out['otrap'] = caller.radius(out)
    count = remaining = s['hybrid_counts[0]']
    out['tmp'] = remaining
    for _ in range(s['max_iterations']):
        out['rold'] = out['rout']
        if remaining == 0:
            remaining = count
        out = point(out, params, name)
        remaining -= 1
        out['tmp'] = remaining
        out['iteration'] += 1
        out['rout'] = caller.radius(out)
        if fraction(out['rout']) < fraction(out['otrap']):
            out['otrap'] = out['rout']
        if fraction(out['rout']) > fraction(s['rstop']):
            break
    if not fraction(out['xyz[3]']):
        raise ScopeError('zero denominator outside finite distance caller scope')
    value = fraction(out['rout'])
    a, b = isqrt(value.numerator), isqrt(value.denominator)
    if a*a != value.numerator or b*b != value.denominator:
        raise ScopeError('irrational distance transport')
    distance = Fraction(a, b)/abs(fraction(out['xyz[3]']))
    rounded = fp32(distance)
    if caller.normal_or_zero(rounded) != distance:
        raise ScopeError('rounded distance transport')
    out['sse[0]'], out['sse[1]'] = rounded, 0
    out['sse[2]'], out['sse[3]'] = out['xyz[3]'] & 0x7fffffff, 0
    return out


def signed32(bits):
    if type(bits) is not int or not 0 <= bits <= 0xffffffff:
        raise ValueError('expected raw Int32 word')
    return bits-(1 << 32) if bits >> 31 else bits


def program(params, s, name='genIFS'):
    ident, option, count, traits, flags, driver = FORMULAS[name]
    if len(params) != count:
        raise ValueError('wrong prepared parameter count')
    p = caller.program(params[0], s)
    base = 56
    p[1] = driver
    p[base:base+12] = [1, 1, ident, option, count, traits, flags, 0, 0, 1, 0, 0]
    memory = bytearray(512)
    struct.pack_into('<Q', memory, 248, 0x3fe0000000000000)
    for i, ((offset, kind), value) in enumerate(zip(LAYOUTS[name], params)):
        if kind == 'integer':
            integer = signed32(value)
            original = struct.unpack('<Q', struct.pack('<d', integer))[0]
            p[base+12+i] = fp32(integer)  # Convenience mirror; never the predicate oracle.
            struct.pack_into('<I', memory, 256+offset, value)
        else:
            caller.normal_or_zero(value)
            original = matrix.wide32(value)
            p[base+12+i] = value
            struct.pack_into('<Q' if kind == 'double' else '<I', memory,
                             256+offset, original if kind == 'double' else value)
        p[base+76+2*i:base+78+2*i] = [original & 0xffffffff, original >> 32]
    p[base+204:base+268] = struct.unpack('<64I', memory[:256])
    # Raw aliases can encode nonfinite doubles when adjacent typed Int32/Single
    # words are paired. They are transport caches, not arithmetic inputs.
    for i in range(32):
        p[base+268+i] = matrix.narrow64(struct.unpack_from('<Q', memory, 248-8*i)[0])
        p[base+300+i] = matrix.narrow64(struct.unpack_from('<Q', memory, 256+8*i)[0])
    return p


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('wrong semantic input/identity')
    identity = payload['identity']
    if not isinstance(identity, list) or len(identity) != 3 or not isinstance(identity[2], str) or identity[2] not in FORMULAS:
        raise ValueError('wrong formula identity')
    name = identity[2]
    if identity != ['external', FORMULAS[name][0], name]:
        raise ValueError('wrong formula identity')
    if set(payload['input_state_bits']) != set(STATE) or len(payload['parameter_bits']) != 2048:
        raise ValueError('wrong state/program width')
    def decode(v):
        if not isinstance(v, str) or not re.fullmatch(r'0x[0-9a-f]{8}', v):
            raise ValueError('expected canonical hex32')
        return int(v, 16)
    s = {k: decode(v) for k, v in payload['input_state_bits'].items()}
    p = [decode(v) for v in payload['parameter_bits']]
    for lane in STATE[:33]:
        caller.normal_or_zero(s[lane])
    if payload['kind'] not in ('point', 'orbit') or s['julia'] not in (0, 1) or s['inside'] or s['calc_smooth']:
        raise ScopeError('unsupported caller')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3 or any(s[f'hybrid_counts[{i}]'] for i in range(1, 6)):
        raise ScopeError('single positive-slot caller bounds')
    params = p[68:68+FORMULAS[name][2]]
    for i, (offset, kind) in enumerate(LAYOUTS[name]):
        if kind == 'integer':
            params[i] = p[56+204+(256+offset)//4]
    if p != program(params, s, name):
        raise ValueError('typed prepared ABI or caller metadata differs')
    if payload['kind'] == 'orbit':
        if s['vary_scale'] != ONE or s['j[3]'] != s['original_j[3]']:
            raise ScopeError('entry state outside audited caller domain')
        out = orbit(s, params, name)
    else:
        out = point(s, params, name)
    for lane in STATE[:33]:
        caller.normal_or_zero(out[lane])
    return {k: word(out[k]) for k in STATE}


def dependency_paths():
    return [Path(module.__file__).resolve() for module in (arithmetic, caller, matrix)]


def manifest():
    def check(pins):
        result = {}
        for path, pin in pins.items():
            data = (ROOT/path).read_bytes()
            if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != pin:
                raise ValueError('pinned source changed: '+path)
            result[path] = hashlib.sha256(data).hexdigest()
        return result
    sources, abi = check(SOURCE_PINS), check(ABI_PINS)
    dependencies = check(DEPENDENCY_PINS)
    profile = check({caller.CALLER_PROFILE[0]: caller.CALLER_PROFILE[1]})
    audit = {}
    for name, (length, pin) in CODE_PINS.items():
        text = (ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code = bytes.fromhex(re.search(r'\[CODE\](.*?)\[END\]', text, re.S)[1])
        if len(code) != length or hashlib.sha256(code).hexdigest() != pin:
            raise ValueError('original CODE changed: '+name)
        audit[name] = {'bytes': length, 'sha256': pin,
                      'parameter_layout': [list(entry) for entry in LAYOUTS[name]],
                      'row_execution_order': [2, 0, 1],
                      'operations': ('conditional abs, offset, row dot, signed scale, current J addition; signed w scaling'
                                     if name == 'genIFS' else
                                     'matrix1, inclusive signed pair folds, positive Y/X edges, matrix2, Scale*q-(Scale-1)*CScale; signed w scaling')}
    return {'schema': 'mb3d.native-group05-ifs-reference.v1',
            'semantic_sources': sources, 'abi_sources': abi, 'dependencies': dependencies,
            'caller_profile': profile,
            'code_audit': audit,
            'arithmetic': 'Independent rational exact binary32 operations/subset sums; explicit signed-zero order',
            'model_limits': ['genIFS429 and SierpHilbert590 bounded subsets; all47 state words',
                'Typed Int32 predicates and exact widened Scale; finite normal arithmetic/zeros',
                'Prepared matrix ABI; only signed-zero authored-angle preparation; genIFS defaults and arbitrary trig not covered',
                'Single positive slot count1..3, maxIt1..8; inactive counts zero; analytic3D DE2 without smoothing',
                'Exact rational radius/root/distance transport; zero-w orbit rejected',
                'No arbitrary-input, hybrid, scene, x87 or cross-platform qualification'],
            'model_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def prepared_matrix(name):
    if name == 'zero_angles':
        return matrix.prepare_zero_angles([0, 0, 0])
    values = {
        'cycle': [0, 1, 0, 0, 0, 1, 1, 0, 0],
        'quarter': [0, -1, 0, 1, 0, 0, 0, 0, 1],
        'dense': [Fraction(1, 2), Fraction(1, 4), Fraction(-1, 8),
                  Fraction(-1, 4), 1, Fraction(1, 2), Fraction(1, 8), Fraction(-1, 2), Fraction(1, 4)],
    }
    return list(map(fp32, values[name]))


def parameters(scale=-2, flags=(1, 0, 1), offsets=(Fraction(1, 2), Fraction(-1, 2), 1), rotation='cycle'):
    return [fp32(scale), *[v & 0xffffffff for v in flags], *map(fp32, offsets), *prepared_matrix(rotation)]


def sierp_parameters(scale=3, cscale=(1, 1, 1), edges=(2, 1), first='zero_angles', second='zero_angles'):
    return [fp32(scale), *map(fp32, cscale), *prepared_matrix(first), *prepared_matrix(second), *map(fp32, edges)]


def state():
    s = caller.state()
    for i, v in enumerate((-2, 4, -4, -3)):
        s[f'xyz[{i}]'] = fp32(v)
    for i, v in enumerate((8, -8, 9)):
        s[f'j[{i}]'] = fp32(v)
    return s


def orbit_state(julia=1, count=1):
    s = state(); s['julia'] = julia; s['hybrid_counts[0]'] = count
    for i, v in enumerate((3, 4, 12)):
        s[f'c[{i}]'] = fp32(v); s[f'original_j[{i}]'] = 0
    return s


def build_cases():
    cases = []; pins = manifest()['semantic_sources']
    def emit(tag, s, params, kind='point', name='genIFS'):
        ident = FORMULAS[name][0]
        semantic = {'identity': ['external', ident, name], 'kind': kind,
                    'input_state_bits': {k: word(s[k]) for k in STATE},
                    'parameter_bits': [word(v) for v in program(params, s, name)]}
        reads = {f'xyz[{i}]' for i in range(4)}
        if name == 'genIFS':
            reads |= {f'j[{i}]' for i in range(3)}
        writes = {f'xyz[{i}]' for i in range(4)} | {'current_slot'}
        if kind == 'orbit':
            reads |= {f'{a}[{i}]' for a in ('c', 'original_j') for i in range(3)} | {'julia', 'max_iterations', 'rstop', 'hybrid_counts[0]'}
            writes |= {f'sse[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)} | {'rout', 'rold', 'otrap', 'first', 'iteration', 'tmp'}
        sources = {k: v for k, v in pins.items() if '/M3Formulas/' not in k or k.endswith('/'+name+'.m3f')}
        cases.append({'id': str(ident)+'-'+tag, 'identity': semantic['identity'], 'variant': tag,
                      'input': semantic, 'expected_state_bits': evaluate(semantic),
                      'read_lanes': [k for k in STATE if k in reads],
                      'write_lanes': [k for k in STATE if k in writes], 'source_hashes': sources})
    for mask in range(8):
        s = state(); s['xyz[1]'] = fp32(-4)
        emit(f'point-abs-{mask}', s, parameters(flags=tuple((mask >> i) & 1 for i in range(3)), rotation='dense'))
    for axis in range(3):
        for flag in (-1, -(1 << 31), (1 << 31)-1, 1 << 24, (1 << 24)+1):
            flags = [0, 0, 0]; flags[axis] = flag
            s = state(); s['xyz[1]'] = fp32(-4)
            emit(f'point-int-{axis}-{flag}', s, parameters(flags=flags))
    for rotation in ('zero_angles', 'cycle', 'quarter', 'dense'):
        emit('point-matrix-'+rotation, state(), parameters(scale=1, rotation=rotation))
    for index, scale in enumerate((-2, Fraction(-1, 2), 0, Fraction(1, 2), 1, 2)):
        emit(f'point-scale-{index}', state(), parameters(scale=scale, flags=(0, 1, 0), rotation='zero_angles'))
    p = parameters(scale=0, flags=(0, 1, 0), rotation='zero_angles'); p[0] = NEG_ZERO
    emit('point-scale-negative-zero', state(), p)
    for flag in (0, 1):
        s = state(); s['xyz[0]'] = NEG_ZERO; s['xyz[1]'] = ONE; s['xyz[2]'] = fp32(-1); s['j[0]'] = NEG_ZERO
        p = parameters(scale=1, flags=(flag, 0, 0), offsets=(0, 0, 0), rotation='zero_angles'); p[4] = NEG_ZERO
        emit(f'point-signed-zero-abs-{flag}', s, p)
    s = state(); s.update(first=0x80000000, iteration=0x7fffffff, tmp=0xffffffff)
    emit('point-integer-state-sentinels', s, parameters())
    scales = (-2, Fraction(-1, 2), Fraction(1, 2), 1, 2)
    for rotation in ('zero_angles', 'cycle', 'quarter'):
        for index, scale in enumerate(scales):
            emit(f'orbit-julia-{rotation}-{index}', orbit_state(), parameters(scale, (0, 0, 0), (0, 0, 0), rotation), 'orbit')
    for rotation in ('zero_angles', 'cycle'):
        for scale in (-2, 2):
            for count in (2, 3):
                emit(f'orbit-count-{rotation}-{scale}-{count}', orbit_state(count=count), parameters(scale, (0, 0, 0), (0, 0, 0), rotation), 'orbit')
    for index, scale in enumerate(scales):
        emit(f'orbit-mandel-{index}', orbit_state(julia=0), parameters(scale, (0, 0, 0), (0, 0, 0), 'zero_angles'), 'orbit')
    for count in (2, 3):
        emit(f'orbit-mandel-count-{count}', orbit_state(julia=0, count=count), parameters(2, (0, 0, 0), (0, 0, 0), 'zero_angles'), 'orbit')
    for mask in range(8):
        emit(f'orbit-abs-{mask}', orbit_state(), parameters(-1, tuple((mask >> i) & 1 for i in range(3)), (0, 0, 0), 'quarter'), 'orbit')
    for index, scale in enumerate((-2, Fraction(-1, 2), Fraction(1, 2), 2)):
        emit(f'orbit-offset-{index}', orbit_state(), parameters(scale, (0, 0, 0), (Fraction(3, 4), 1, 3), 'zero_angles'), 'orbit')
    for tag, stop in [('escape', 338), ('bailout-equality', 676)]:
        s = orbit_state(); s['rstop'] = fp32(stop)
        emit('orbit-'+tag, s, parameters(2, (0, 0, 0), (0, 0, 0), 'zero_angles'), 'orbit')
    for maximum in (1, 2, 8):
        s = orbit_state(count=3); s['max_iterations'] = maximum
        emit(f'orbit-max-{maximum}', s, parameters(Fraction(1, 2), (0, 0, 0), (0, 0, 0), 'zero_angles'), 'orbit')
    append_sierp_cases(emit)
    return cases


def append_sierp_cases(emit):
    def add(tag, s, params, kind='point'):
        emit(tag, s, params, kind, 'SierpHilbert')
    # Select one exact nonzero triple for every reachable branch combination.
    # This is corpus coverage, not a substitute for literal/CODE replay tests.
    branches = {}
    for xyz in cartesian_product((-3, -2, -1, 1, 2, 3), repeat=3):
        v = list(xyz); mask = 0
        for bit, (a, b) in enumerate(((0, 1), (0, 2), (1, 2))):
            if -v[a] >= v[b]:
                mask |= 1 << bit; v[a], v[b] = -v[b], -v[a]
        branches.setdefault(mask, xyz)
    if set(branches) != set(range(8)):
        raise ValueError('missing original fold branch combination')
    for mask, xyz in sorted(branches.items()):
        s = state()
        for i, value in enumerate(xyz): s[f'xyz[{i}]'] = fp32(value)
        add(f'point-fold-{mask}', s, sierp_parameters(1, (0, 0, 0), (0, 0)))
    for index, xyz in enumerate(((2, -2, 3), (2, 3, -2), (3, 2, -2))):
        s = state()
        for i, value in enumerate(xyz): s[f'xyz[{i}]'] = fp32(value)
        add(f'point-fold-equality-{index}', s, sierp_parameters(1, (0, 0, 0), (0, 0)))
    for mask in range(8):
        s = state()
        for i in range(3): s[f'xyz[{i}]'] = NEG_ZERO if mask & (1 << i) else 0
        add(f'point-signed-zero-{mask}', s, sierp_parameters(1, (1, 1, 1), (0, 0)))
    for axis in (0, 1):
        for label, edge in [('negative', fp32(-1)), ('negative-zero', NEG_ZERO), ('positive', ONE)]:
            s = state()
            for i in range(3): s[f'xyz[{i}]'] = 0
            p = sierp_parameters(1, (1, 1, 1), (0, 0)); p[22+axis] = edge
            add(f'point-zero-edge-{axis}-{label}', s, p)
    for axis in (0, 1):
        for value in (1, 2, 3):
            s = state()
            for i in range(3): s[f'xyz[{i}]'] = fp32(value if i == axis else 0)
            edges = (2, 0) if axis == 1 else (0, 2)
            add(f'point-edge-threshold-{axis}-{value}', s, sierp_parameters(1, (0, 0, 0), edges))
    for first, second in [('dense', 'cycle'), ('cycle', 'quarter'), ('quarter', 'dense'), ('zero_angles', 'dense')]:
        add(f'point-matrices-{first}-{second}', state(), sierp_parameters(-2, (1, 2, -1), (2, 1), first, second))
    for label, xyz in [('positive', (3, 4, 12)), ('signed', (-2, 4, -4))]:
        s = state()
        for i, value in enumerate(xyz): s[f'xyz[{i}]'] = fp32(value)
        add('point-default-'+label, s, sierp_parameters())
    for index, scale in enumerate((-2, Fraction(-1, 2), 0, Fraction(1, 2), 1, 2, 3)):
        add(f'point-scale-{index}', state(), sierp_parameters(scale, (1, 2, -1), (0, 0)))
    p = sierp_parameters(0, (1, 2, -1), (0, 0)); p[0] = NEG_ZERO
    add('point-scale-negative-zero', state(), p)
    s = state(); s.update(first=0x80000000, iteration=0x7fffffff, tmp=0xffffffff)
    add('point-integer-state-sentinels', s, sierp_parameters())
    for first, second in [('zero_angles', 'zero_angles'), ('cycle', 'quarter'), ('quarter', 'cycle')]:
        for index, scale in enumerate((-2, Fraction(-1, 2), Fraction(1, 2), 1, 2)):
            add(f'orbit-permutation-{first}-{second}-{index}', orbit_state(), sierp_parameters(scale, (0, 0, 0), (0, 0), first, second), 'orbit')
    for scale in (-2, 2):
        for count in (2, 3):
            add(f'orbit-count-{scale}-{count}', orbit_state(count=count), sierp_parameters(scale, (0, 0, 0), (0, 0)), 'orbit')
    for maximum in (1, 2, 8):
        s = orbit_state(count=3); s['max_iterations'] = maximum
        add(f'orbit-max-{maximum}', s, sierp_parameters(Fraction(1, 2), (0, 0, 0), (0, 0)), 'orbit')
    for index, scale in enumerate((-2, Fraction(1, 2), 2)):
        add(f'orbit-mandel-{index}', orbit_state(julia=0), sierp_parameters(scale, (0, 0, 0), (0, 0)), 'orbit')
    for axis in (0, 1):
        for index, scale in enumerate((-2, Fraction(1, 2), 1, 2)):
            s = orbit_state()
            for i in range(3): s[f'c[{i}]'] = fp32(4 if i == axis else 0)
            edges = (1, 0) if axis == 1 else (0, 1)
            add(f'orbit-active-edge-{axis}-{index}', s, sierp_parameters(scale, (0, 0, 0), edges), 'orbit')
    for label, edge in [('negative', fp32(-1)), ('negative-zero', NEG_ZERO)]:
        p = sierp_parameters(2, (0, 0, 0), (0, 0)); p[22:24] = [edge, edge]
        add('orbit-disabled-edges-'+label, orbit_state(), p, 'orbit')
    for julia in (0, 1):
        s = orbit_state(julia=julia); s['max_iterations'] = 1
        for i in range(3): s[f'c[{i}]'] = ONE if i == 0 else 0
        add(f'orbit-default-one-iteration-{julia}', s, sierp_parameters(), 'orbit')
    for label, stop in [('escape', 338), ('bailout-equality', 676)]:
        s = orbit_state(); s['rstop'] = fp32(stop)
        add('orbit-'+label, s, sierp_parameters(2, (0, 0, 0), (0, 0)), 'orbit')
    s = orbit_state(); s['max_iterations'] = 3
    for i in range(3): s[f'c[{i}]'] = 0
    add('orbit-zero-radius', s, sierp_parameters(1, (1, 1, 1), (0, 0)), 'orbit')
