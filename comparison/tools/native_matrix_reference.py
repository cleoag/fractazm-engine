"""Independent original-CODE models for mutable Julia updates and 3D matrices.

The finite corpus uses exact products/subset sums, hence no chosen FMA policy.
Nonidentity matrices exercise the prepared Single ABI, not Euler preparation.
Only signed-zero Euler angles have independently implemented preparation;
arbitrary angles/trig, distance scalars and full scenes remain outside scope.
Expected values never depend on translated formula bodies or GPU observations.
"""
from __future__ import annotations

from fractions import Fraction
import hashlib
from itertools import combinations
from pathlib import Path
import re
import struct

import native_formula_reference as affine
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
SOURCE_PINS = {'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
 'mb3d/M3Formulas/_updateC.m3f': 'de5cdad9bd1066351cc84865c92382b74e9a9d69aa3402dd0ec2e443b3fe7c33',
 'mb3d/M3Formulas/_Rotate.m3f': '34e751a8760ffb22c095f16fa270072301a686c624517eb778d83c3f1712a7e6',
 'mb3d/M3Formulas/_RotateC.m3f': 'af56a8229a0818f09504d7c54fe33bb8e36aadc78b622652dbcb2558ccab0c09'}
ABI_PINS = {'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a',
 'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6'}
CODE_AUDIT = {'_updateC': {'sha256': 'c43eaa5ad5132884ddcda46b90a3e00a77eab96b12680eeee2e85ac6796ed653',
              'bytes': 70,
              'key_offsets': [{'offset': '0x000f',
                               'bytes': 'dc4ff0',
                               'instruction': 'fmul qword ptr [edi - 0x10]'},
                              {'offset': '0x0015',
                               'bytes': 'dc4fe8',
                               'instruction': 'fmul qword ptr [edi - 0x18]'},
                              {'offset': '0x0018', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                              {'offset': '0x001a',
                               'bytes': 'dd5e18',
                               'instruction': 'fstp qword ptr [esi + 0x18]'},
                              {'offset': '0x0020',
                               'bytes': 'dc4ff0',
                               'instruction': 'fmul qword ptr [edi - 0x10]'},
                              {'offset': '0x0026',
                               'bytes': 'dc4fe8',
                               'instruction': 'fmul qword ptr [edi - 0x18]'},
                              {'offset': '0x0029', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                              {'offset': '0x002b',
                               'bytes': 'dd5e20',
                               'instruction': 'fstp qword ptr [esi + 0x20]'},
                              {'offset': '0x0031',
                               'bytes': 'dc4ff0',
                               'instruction': 'fmul qword ptr [edi - 0x10]'},
                              {'offset': '0x0037',
                               'bytes': 'dc4fe8',
                               'instruction': 'fmul qword ptr [edi - 0x18]'},
                              {'offset': '0x003a', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                              {'offset': '0x003c',
                               'bytes': 'dd5e28',
                               'instruction': 'fstp qword ptr [esi + 0x28]'}]},
 '_Rotate': {'sha256': 'bb35d2ba6c5e9f3088fdbf5b7bcd8802d6e0fab1664f396d7287a5c82df6ba1d',
             'bytes': 84,
             'key_offsets': [{'offset': '0x0012',
                              'bytes': 'd84edc',
                              'instruction': 'fmul dword ptr [esi - 0x24]'},
                             {'offset': '0x0017',
                              'bytes': 'd84ed8',
                              'instruction': 'fmul dword ptr [esi - 0x28]'},
                             {'offset': '0x001a', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                             {'offset': '0x001e',
                              'bytes': 'd84ed4',
                              'instruction': 'fmul dword ptr [esi - 0x2c]'},
                             {'offset': '0x0021', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                             {'offset': '0x0025',
                              'bytes': 'd84ef4',
                              'instruction': 'fmul dword ptr [esi - 0xc]'},
                             {'offset': '0x002a',
                              'bytes': 'd84ef0',
                              'instruction': 'fmul dword ptr [esi - 0x10]'},
                             {'offset': '0x002d', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                             {'offset': '0x0031',
                              'bytes': 'd84eec',
                              'instruction': 'fmul dword ptr [esi - 0x14]'},
                             {'offset': '0x0034', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                             {'offset': '0x0038',
                              'bytes': 'd84ee8',
                              'instruction': 'fmul dword ptr [esi - 0x18]'},
                             {'offset': '0x003d',
                              'bytes': 'd84ee4',
                              'instruction': 'fmul dword ptr [esi - 0x1c]'},
                             {'offset': '0x0040', 'bytes': 'dec3', 'instruction': 'faddp st(3)'},
                             {'offset': '0x0044',
                              'bytes': 'd84ee0',
                              'instruction': 'fmul dword ptr [esi - 0x20]'},
                             {'offset': '0x0047', 'bytes': 'dec2', 'instruction': 'faddp st(2)'},
                             {'offset': '0x0049',
                              'bytes': 'dd18',
                              'instruction': 'fstp qword ptr [eax]'},
                             {'offset': '0x004b',
                              'bytes': 'dd1a',
                              'instruction': 'fstp qword ptr [edx]'},
                             {'offset': '0x004d',
                              'bytes': 'dd19',
                              'instruction': 'fstp qword ptr [ecx]'}]},
 '_RotateC': {'sha256': '29c9826d94f5eb3bd1d33fc48740f6c5a33d2c692a3146a7ff5c30a5deac8c82',
              'bytes': 92,
              'key_offsets': [{'offset': '0x0016',
                               'bytes': 'd84fdc',
                               'instruction': 'fmul dword ptr [edi - 0x24]'},
                              {'offset': '0x001b',
                               'bytes': 'd84fd8',
                               'instruction': 'fmul dword ptr [edi - 0x28]'},
                              {'offset': '0x001e', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                              {'offset': '0x0022',
                               'bytes': 'd84fd4',
                               'instruction': 'fmul dword ptr [edi - 0x2c]'},
                              {'offset': '0x0025', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                              {'offset': '0x0029',
                               'bytes': 'd84ff4',
                               'instruction': 'fmul dword ptr [edi - 0xc]'},
                              {'offset': '0x002e',
                               'bytes': 'd84ff0',
                               'instruction': 'fmul dword ptr [edi - 0x10]'},
                              {'offset': '0x0031', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                              {'offset': '0x0035',
                               'bytes': 'd84fec',
                               'instruction': 'fmul dword ptr [edi - 0x14]'},
                              {'offset': '0x0038', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                              {'offset': '0x003c',
                               'bytes': 'd84fe8',
                               'instruction': 'fmul dword ptr [edi - 0x18]'},
                              {'offset': '0x0041',
                               'bytes': 'd84fe4',
                               'instruction': 'fmul dword ptr [edi - 0x1c]'},
                              {'offset': '0x0044', 'bytes': 'dec3', 'instruction': 'faddp st(3)'},
                              {'offset': '0x0048',
                               'bytes': 'd84fe0',
                               'instruction': 'fmul dword ptr [edi - 0x20]'},
                              {'offset': '0x004b', 'bytes': 'dec2', 'instruction': 'faddp st(2)'},
                              {'offset': '0x004d',
                               'bytes': 'dd5e18',
                               'instruction': 'fstp qword ptr [esi + 0x18]'},
                              {'offset': '0x0050',
                               'bytes': 'dd5e20',
                               'instruction': 'fstp qword ptr [esi + 0x20]'},
                              {'offset': '0x0053',
                               'bytes': 'dd5e28',
                               'instruction': 'fstp qword ptr [esi + 0x28]'}]}}
PRIMITIVE_LF_SHA256 = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'

FORMULAS = {
    '_updateC': (7055, -1, 2, 0, 1, 1),
    '_Rotate': (7060, -1, 9, 0, 1, 1),
    '_RotateC': (7062, -1, 9, 0, 1, 1),
}
LAYOUT = {
    '_updateC': ((-16, 'double'), (-24, 'double')),
    '_Rotate': tuple((-12-4*i, 'single') for i in range(9)),
    '_RotateC': tuple((-12-4*i, 'single') for i in range(9)),
}
fp32, fraction, word = affine.binary32, affine.fraction, affine.word
ONE, NEG_ZERO = 0x3f800000, 0x80000000


class ScopeError(ValueError):
    """Input has no proof in this bounded model; never a successful fixture."""


def normal_or_zero(bits):
    value = fraction(bits)
    if value and bits & 0x7f800000 == 0:
        raise ScopeError('subnormal arithmetic is outside the proven domain')
    return value


def exact_mul(a, b):
    value = normal_or_zero(a)*normal_or_zero(b)
    result = affine.multiply(a, b)
    if normal_or_zero(result) != value:
        raise ScopeError('rounded product is outside the contraction-invariant domain')
    return result


def exact_sum(products):
    # Check all partial subsets, not just the original left-associated sum.
    # This closes both fused multiply/add choices and alternate grouping for
    # these exact finite cases without defining a general compiler-FMA oracle.
    for count in range(1, len(products)+1):
        for subset in combinations(products, count):
            value = sum((fraction(v) for v in subset), Fraction())
            if normal_or_zero(fp32(value)) != value:
                raise ScopeError('rounded partial sum is outside the contraction-invariant domain')
    result = products[0]
    for value in products[1:]:
        result = affine.add(result, value)
    return result


def exact_dot(left, right):
    return exact_sum([exact_mul(a, b) for a, b in zip(left, right)])


def prepare_zero_angles(angles):
    """CustomFormulas.pas type6 and Math3D.pas BuildRotMatrix, only +/-0.

    FSINCOS(+/-0) gives sin=+/-0, cos=1. All preparation operations here are
    exact in both binary64 and binary32, including their signed-zero rules.
    Nonzero angles are deliberately not snapped to mathematical quarter turns.
    """
    if len(angles) != 3 or any(v not in (0, NEG_ZERO) for v in angles):
        raise ScopeError('arbitrary Euler/trigonometric preparation remains OPEN')
    sx, sy, sz = angles
    mul, add = exact_mul, affine.add
    sub = lambda a, b: add(a, b ^ NEG_ZERO)
    return [ONE, mul(ONE ^ NEG_ZERO, sz), sy,
            add(mul(mul(sx, sy), ONE), mul(ONE, sz)),
            sub(ONE, mul(mul(sx, sy), sz)), mul(sx ^ NEG_ZERO, ONE),
            sub(mul(sx, sz), mul(mul(ONE, sy), ONE)),
            add(mul(mul(ONE, sy), sz), mul(sx, ONE)), ONE]


def fraction64(bits):
    exponent = (bits >> 52) & 2047
    if exponent == 2047:
        raise ScopeError('nonfinite binary64 metadata')
    mantissa = (bits & ((1 << 52)-1)) | ((1 << 52) if exponent else 0)
    shift = exponent-1075 if exponent else -1074
    value = Fraction(mantissa << shift) if shift >= 0 else Fraction(mantissa, 1 << -shift)
    return -value if bits >> 63 else value


def narrow64(bits):
    exponent, payload = (bits >> 52) & 2047, bits & ((1 << 52)-1)
    sign = (bits >> 32) & NEG_ZERO
    if exponent == 2047:
        # Packed Single pairs are raw aliases, not extra arithmetic parameters.
        return sign | 0x7f800000 | ((0x400000 | (payload >> 29)) if payload else 0)
    return fp32(fraction64(bits), bool(sign))


def wide32(bits):
    normal_or_zero(bits)
    # Widening a finite binary32 to binary64 is exact on the host; no expected
    # arithmetic or transcendental result is obtained through host math.
    return struct.unpack('<Q', struct.pack('<d', struct.unpack('<f', struct.pack('<I', bits))[0]))[0]


def _program(name, params, state):
    ident, option, count, traits, flags, driver = FORMULAS[name]
    if len(params) != count:
        raise ValueError('wrong prepared parameter count')
    p = [0]*2048
    p[:15] = [1, driver, 6, 0, 0, 0, 2, state['max_iterations'], state['julia'],
              0, 0, 16384, 4096, 0, 0]
    p[16], p[18], p[21], p[22] = state['rstop'], ONE, fp32(1 << 20), ONE
    p[24:28] = [state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4):
        p[28+5*i] = ONE
    p[50] = state['hybrid_counts[0]']
    for slot in range(6):
        p[56+slot*332] = 1
    base = 56
    p[base:base+12] = [1, 1, ident, option & 0xffffffff, count, traits, flags, 0, 0, 1, 0, 0]
    memory = bytearray(512)  # Original PVar is at byte256, not at matrix start.
    struct.pack_into('<Q', memory, 248, 0x3fe0000000000000)
    for i, ((offset, kind), value) in enumerate(zip(LAYOUT[name], params)):
        original = wide32(value)
        struct.pack_into('<I' if kind == 'single' else '<Q', memory,
                         256+offset, value if kind == 'single' else original)
        p[base+12+i] = value
        p[base+76+2*i:base+78+2*i] = [original & 0xffffffff, original >> 32]
    p[base+204:base+268] = struct.unpack('<64I', memory[:256])
    for i in range(32):
        p[base+268+i] = narrow64(struct.unpack_from('<Q', memory, 248-8*i)[0])
        p[base+300+i] = narrow64(struct.unpack_from('<Q', memory, 256+8*i)[0])
    return p


def _point(name, state, params):
    out = dict(state)
    if name == '_updateC':
        # Original [PIteration3D+24,+32,+40] are J, despite the formula name.
        for i in range(3):
            out[f'j[{i}]'] = exact_dot([state[f'xyz[{i}]'], state[f'j[{i}]']], params)
    elif name in ('_Rotate', '_RotateC'):
        lane = 'xyz' if name == '_Rotate' else 'j'
        original = [state[f'{lane}[{i}]'] for i in range(3)]
        # Original x87 scheduling calculates row2, row0, row1 but stores the
        # usual row0/1/2. Every output must read the unmodified input vector.
        for row in (2, 0, 1):
            out[f'{lane}[{row}]'] = exact_dot(original, params[3*row:3*row+3])
    else:
        raise ValueError('formula outside independent matrix model')
    out['current_slot'] = 0
    return out


def _orbit(name, state, params):
    # formulas.pas doHybridPasDE: one positive-count analytic3D slot, no
    # interpolation, smoothing or mixed-formula distance combination.
    s = dict(state)
    for i in range(3):
        s[f'xyz[{i}]'] = s[f'c[{i}]']
        s[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]'] = ONE
    s['first'] = s['iteration'] = 0
    s['rout'] = s['otrap'] = affine._radius(s, False)
    remaining = count = s['hybrid_counts[0]']
    s['tmp'] = remaining
    for _ in range(s['max_iterations']):
        s['rold'] = s['rout']
        if remaining == 0:
            remaining = count
        s = _point(name, s, params)
        remaining -= 1
        s['tmp'] = remaining
        s['iteration'] += 1
        s['rout'] = affine._radius(s, False)
        if fraction(s['rout']) < fraction(s['otrap']):
            s['otrap'] = s['rout']
        if fraction(s['rout']) > fraction(s['rstop']):
            break
    return s


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('expected exact semantic input fields')
    identity = payload['identity']
    if not isinstance(identity, list) or len(identity) != 3 or identity[0] != 'external' or identity[2] not in FORMULAS or identity[1] != FORMULAS[identity[2]][0]:
        raise ValueError('wrong matrix formula identity')
    name = identity[2]
    def decode(value):
        if not isinstance(value, str) or not re.fullmatch(r'0x[0-9a-f]{8}', value):
            raise ValueError('expected hex32')
        return int(value, 16)
    if set(payload['input_state_bits']) != set(STATE) or len(payload['parameter_bits']) != 2048:
        raise ValueError('wrong ABI width')
    s = {k: decode(v) for k, v in payload['input_state_bits'].items()}
    p = [decode(v) for v in payload['parameter_bits']]
    for lane in STATE[:33]:
        normal_or_zero(s[lane])
    if payload['kind'] not in ('point', 'orbit') or s['julia'] not in (0, 1) or s['inside'] or s['calc_smooth']:
        raise ScopeError('caller outside bounded analytic3D model')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3:
        raise ScopeError('caller work bounds exceeded')
    params = p[68:68+FORMULAS[name][2]]
    if p != _program(name, params, s):
        raise ValueError('typed prepared parameter ABI differs')
    if payload['kind'] == 'orbit':
        if s['vary_scale'] != ONE or s['j[3]'] != s['original_j[3]']:
            raise ScopeError('unsupported analytic3D entry state')
        out = _orbit(name, s, params)
    else:
        out = _point(name, s, params)
    for lane in STATE[:33]:
        normal_or_zero(out[lane])
    return {k: word(out[k]) for k in STATE}


def dependency_paths():
    return [Path(affine.__file__).resolve()]


def primitive_identity(data):
    if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != PRIMITIVE_LF_SHA256:
        raise ValueError('frozen arithmetic dependency changed')
    return hashlib.sha256(data).hexdigest()


def manifest():
    def verify_pins(pins):
        result = {}
        for path, pin in pins.items():
            data = (ROOT/path).read_bytes()
            if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != pin:
                raise ValueError('pinned original source/ABI changed: '+path)
            result[path] = hashlib.sha256(data).hexdigest()
        return result
    sources, abi = verify_pins(SOURCE_PINS), verify_pins(ABI_PINS)
    for name, audit in CODE_AUDIT.items():
        text = (ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code = bytes.fromhex(re.sub(r'\s+', '', re.search(r'\[CODE\](.*?)\[END\]', text, re.S)[1]))
        if len(code) != audit['bytes'] or hashlib.sha256(code).hexdigest() != audit['sha256']:
            raise ValueError('original CODE changed')
        for instruction in audit['key_offsets']:
            start = int(instruction['offset'], 16)
            expected = bytes.fromhex(instruction['bytes'])
            if code[start:start+len(expected)] != expected:
                raise ValueError('original opcode contract changed')
    dep = dependency_paths()[0]
    return {'schema': 'mb3d.native-matrix-reference.v1', 'semantic_sources': sources,
            'abi_sources': abi, 'code_audit': CODE_AUDIT,
            'dependencies': {str(dep.relative_to(ROOT)): primitive_identity(dep.read_bytes())},
            'arithmetic': 'Rational exact finite products/all subset sums; RN-even signed-zero addition',
            'model_limits': [
                'All47 state words; three original formulas; bounded single-slot analytic3D orbits',
                'Prepared matrices use nine row-major Singles; authored-angle preparation proven only for +/-zero',
                'Nonidentity prepared matrices are ABI fixtures, not claims about Euler/trigonometric preparation',
                'Scalar doubles are exact widened binary32; nonfinite/subnormal arithmetic and rounded products/sums excluded',
                'Exact products/all subset sums avoid choosing a compiler contraction or reassociation policy',
                'No arbitrary Euler preparation, hybrid chains, distance scalar, x87 parity or full-scene qualification'],
            'model_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def _state():
    s = {k: fp32(Fraction(i+1, 32)) for i, k in enumerate(STATE)}
    s.update(iteration=19, first=17, tmp=23, max_iterations=4, julia=0,
             inside=0, calc_smooth=0, current_slot=5)
    for i in range(6):
        s[f'hybrid_counts[{i}]'] = 1 if i == 0 else 0
    for i, value in enumerate((Fraction(3, 8), Fraction(-1, 2), Fraction(5, 8), Fraction(-3, 4))):
        for lane, scale in (('xyz', 1), ('c', 3), ('j', 2), ('original_j', -2)):
            s[f'{lane}[{i}]'] = fp32(value*scale)
    s['j[3]'] = s['original_j[3]']
    s['rstop'], s['vary_scale'] = fp32(1 << 16), ONE
    return s


def build_cases():
    sources = manifest()['semantic_sources']
    cases = []
    identity = prepare_zero_angles([0, 0, 0])
    # Nonidentity matrices below deliberately test the prepared-matrix ABI.
    # They are not asserted to equal host preparation of any particular angle.
    matrices = {
        'zero_angle': identity,
        'cycle': [fp32(v) for v in (0, 1, 0, 0, 0, 1, 1, 0, 0)],
        'quarter_turn_prepared': [fp32(v) for v in (0, -1, 0, 1, 0, 0, 0, 0, 1)],
        'dense_prepared': [fp32(v) for v in (Fraction(1, 2), Fraction(1, 4), Fraction(-1, 8),
                                           Fraction(-1, 4), 1, Fraction(1, 2),
                                           Fraction(1, 8), Fraction(-1, 2), Fraction(1, 4))],
        'zero_prepared': [0]*9,
    }
    def emit(name, variant, state, params, kind='point'):
        semantic = {'identity': ['external', FORMULAS[name][0], name], 'kind': kind,
                    'input_state_bits': {k: word(state[k]) for k in STATE},
                    'parameter_bits': [word(v) for v in _program(name, params, state)]}
        lane = 'xyz' if name == '_Rotate' else 'j'
        writes = {f'{lane}[{i}]' for i in range(3)} | {'current_slot'}
        reads = {f'{lane}[{i}]' for i in range(3)}
        if name == '_updateC':
            reads |= {f'xyz[{i}]' for i in range(3)}
        if kind == 'orbit':
            writes |= {f'xyz[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)}
            writes |= {'rout', 'rold', 'otrap', 'first', 'iteration', 'tmp'}
            reads |= {f'{v}[{i}]' for v in ('c', 'original_j') for i in range(4)}
            reads |= {'julia', 'max_iterations', 'rstop', 'hybrid_counts[0]'}
        cases.append({'id': f'{FORMULAS[name][0]}-{variant}', 'identity': semantic['identity'],
                      'variant': variant, 'input': semantic, 'expected_state_bits': evaluate(semantic),
                      'read_lanes': [k for k in STATE if k in reads],
                      'write_lanes': [k for k in STATE if k in writes], 'source_hashes': sources})
    for variant, scalars in [('copy_xyz', (1, 0)), ('keep_j', (0, 1)), ('zero', (0, 0)),
                             ('sum', (1, 1)), ('half_difference', (Fraction(1, 2), Fraction(-1, 2))),
                             ('negative_weights', (-2, -1))]:
        emit('_updateC', variant, _state(), list(map(fp32, scalars)))
    s = _state()
    for i in range(3):
        s[f'xyz[{i}]'], s[f'j[{i}]'] = fp32((1 << 20)+i+1), fp32(1 << 20)
    emit('_updateC', 'large_exact_cancellation', s, [ONE, fp32(-1)])
    for variant, zeros in [('all_negative_zero', [NEG_ZERO]*3), ('mixed_zeros', [NEG_ZERO, 0, NEG_ZERO])]:
        s = _state()
        for i, value in enumerate(zeros):
            s[f'xyz[{i}]'] = s[f'j[{i}]'] = value
        emit('_updateC', variant, s, [ONE, ONE])
    for name in ('_Rotate', '_RotateC'):
        lane = 'xyz' if name == '_Rotate' else 'j'
        for variant, matrix in matrices.items():
            emit(name, variant, _state(), matrix)
        for column in range(3):
            s = _state()
            for i in range(3):
                s[f'{lane}[{i}]'] = ONE if i == column else 0
            emit(name, f'dense_basis_{column}', s, matrices['dense_prepared'])
        for variant, zeros in [('all_negative_zero', [NEG_ZERO]*3), ('mixed_zeros', [NEG_ZERO, 0, NEG_ZERO])]:
            s = _state()
            for i, value in enumerate(zeros):
                s[f'{lane}[{i}]'] = value
            emit(name, variant, s, identity)
        s = _state()
        emit(name, 'negative_zero_angles', s, prepare_zero_angles([NEG_ZERO, 0, NEG_ZERO]))
    for name in FORMULAS:
        s = _state()
        s['first'], s['iteration'], s['tmp'] = 0x80000000, 0x7fffffff, 0xffffffff
        params = [ONE, ONE] if name == '_updateC' else matrices['dense_prepared']
        emit(name, 'integer_state_sentinels', s, params)
        for variant in ('orbit_mandel', 'orbit_julia', 'orbit_escape', 'orbit_repeat'):
            s = _state()
            s['julia'] = int(variant in ('orbit_julia', 'orbit_repeat'))
            s['hybrid_counts[0]'] = 2 if variant == 'orbit_repeat' else 1
            params = [ONE, ONE] if name == '_updateC' else matrices['cycle']
            if variant == 'orbit_escape':
                s['rstop'] = fp32(Fraction(1, 16))
            if variant == 'orbit_repeat':
                s['rstop'] = affine._radius({**s, **{f'xyz[{i}]': s[f'c[{i}]'] for i in range(3)}}, False)
            emit(name, variant, s, params, 'orbit')
        if name == '_updateC':
            emit(name, 'orbit_half_recurrence', _state(), [ONE, fp32(Fraction(1, 2))], 'orbit')
        else:
            dense_state = _state()
            dense_state['max_iterations'] = 2  # Keep every squared-radius operation exact.
            emit(name, 'orbit_dense', dense_state, matrices['dense_prepared'], 'orbit')
            emit(name, 'orbit_quarter_turn_prepared', _state(), matrices['quarter_turn_prepared'], 'orbit')
    return cases
