"""Bounded independent original CODE models for group03 geometry additions.

Only exact finite dyadic operations are admitted. Prepared matrices are ABI
fixtures, not a claim about preparation of arbitrary Euler angles. This is a
point/single-slot caller oracle, not x87 parity or full-scene qualification.
No translated formula implementation or device output supplies expected bits.
"""
from __future__ import annotations
from fractions import Fraction
import hashlib
from itertools import combinations, product
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
 'mb3d/DivUtils.pas': '879221cb1d6abb371452f1f04afcf5315af238317dcb31b6359a145cb303da96',
 'mb3d/M3Formulas/_FoldingTetra3d.m3f': '5218901ada763be70a9e975c85f7e4fcb0b8aea0187ad83f801c3172cdebee9c',
 'mb3d/M3Formulas/_addC.m3f': 'f2e47069a97b4c30ee353aac85bf595472553224300d8a1de111bf4a7a5e679f',
 'mb3d/M3Formulas/_RotatedFolding.m3f': '475a80dccf13202b915cee203f6d99cf056c723cfad6289a120e077a1bfadbdb',
 'mb3d/M3Formulas/_Transform.m3f': 'e23a99e1cf4a6f27a79377f2bcf2978cd35bf05a652b897da0627bf56bb5fe18'}
ABI_PINS = {'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a',
 'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6'}
CODE_AUDIT = {'_FoldingTetra3d': {'sha256': '3573aff71fbd3741af0e4e7b77c7d5aba8eb7d02839a7859ed6905d168b63d0d',
                     'bytes': 194,
                     'key_offsets': [{'offset': '0xe',
                                      'bytes': '837ff400',
                                      'instruction': 'cmp dword ptr [edi - 0xc], 0'},
                                     {'offset': '0x12', 'bytes': '745f', 'instruction': 'je 0x73'},
                                     {'offset': '0x1e', 'bytes': 'ded9', 'instruction': 'fcompp '},
                                     {'offset': '0x21', 'bytes': 'dfe0', 'instruction': 'fnstsw ax'},
                                     {'offset': '0x23', 'bytes': 'd0ec', 'instruction': 'shr ah, 1'},
                                     {'offset': '0x25', 'bytes': '720c', 'instruction': 'jb 0x33'},
                                     {'offset': '0x69',
                                      'bytes': '837ff000',
                                      'instruction': 'cmp dword ptr [edi - 0x10], 0'},
                                     {'offset': '0x6d', 'bytes': '7449', 'instruction': 'je 0xb8'},
                                     {'offset': '0x73',
                                      'bytes': 'dd03',
                                      'instruction': 'fld qword ptr [ebx]'},
                                     {'offset': '0x80', 'bytes': '7208', 'instruction': 'jb 0x8a'}]},
 '_addC': {'sha256': '2378c25cda30e4fdfa3d96b01225f35ad5de2d3ddbdeb9a82aa3f197036bbb5c',
           'bytes': 53,
           'key_offsets': [{'offset': '0x11', 'bytes': 'dd4618', 'instruction': 'fld qword ptr [esi + 0x18]'},
                           {'offset': '0x14',
                            'bytes': 'dc4ff0',
                            'instruction': 'fmul qword ptr [edi - 0x10]'},
                           {'offset': '0x17', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                           {'offset': '0x1b', 'bytes': 'dd4620', 'instruction': 'fld qword ptr [esi + 0x20]'},
                           {'offset': '0x25',
                            'bytes': 'dd4628',
                            'instruction': 'fld qword ptr [esi + 0x28]'}]},
 '_RotatedFolding': {'sha256': 'adc600fd27771feb17d724e97503792c99c86d9d018bac690df11b0695eaeace',
                     'bytes': 173,
                     'key_offsets': [{'offset': '0x16',
                                      'bytes': 'd84fdc',
                                      'instruction': 'fmul dword ptr [edi - 0x24]'},
                                     {'offset': '0x29',
                                      'bytes': 'd84ff4',
                                      'instruction': 'fmul dword ptr [edi - 0xc]'},
                                     {'offset': '0x3c',
                                      'bytes': 'd84fe8',
                                      'instruction': 'fmul dword ptr [edi - 0x18]'},
                                     {'offset': '0x4f',
                                      'bytes': 'dc47cc',
                                      'instruction': 'fadd qword ptr [edi - 0x34]'},
                                     {'offset': '0x56',
                                      'bytes': 'dc67cc',
                                      'instruction': 'fsub qword ptr [edi - 0x34]'},
                                     {'offset': '0x5f', 'bytes': 'dee9', 'instruction': 'fsubp st(1)'},
                                     {'offset': '0x61', 'bytes': 'd9e0', 'instruction': 'fchs '},
                                     {'offset': '0x67',
                                      'bytes': 'd84fec',
                                      'instruction': 'fmul dword ptr [edi - 0x14]'},
                                     {'offset': '0x7a',
                                      'bytes': 'd84ff4',
                                      'instruction': 'fmul dword ptr [edi - 0xc]'},
                                     {'offset': '0x8d',
                                      'bytes': 'd84ff0',
                                      'instruction': 'fmul dword ptr [edi - 0x10]'}]},
 '_Transform': {'sha256': 'f4bfe32d9a6230670783c2324d9e5355ca05faa96671d3ac440c56e0224a1530',
                'bytes': 154,
                'key_offsets': [{'offset': '0xd',
                                 'bytes': 'dc47d8',
                                 'instruction': 'fadd qword ptr [edi - 0x28]'},
                                {'offset': '0x17',
                                 'bytes': 'dc47e8',
                                 'instruction': 'fadd qword ptr [edi - 0x18]'},
                                {'offset': '0x1c',
                                 'bytes': 'dc4fa0',
                                 'instruction': 'fmul qword ptr [edi - 0x60]'},
                                {'offset': '0x2f',
                                 'bytes': 'dc4fd0',
                                 'instruction': 'fmul qword ptr [edi - 0x30]'},
                                {'offset': '0x53',
                                 'bytes': 'dd4108',
                                 'instruction': 'fld qword ptr [ecx + 8]'},
                                {'offset': '0x56',
                                 'bytes': 'dd86e8000000',
                                 'instruction': 'fld qword ptr [esi + 0xe8]'},
                                {'offset': '0x5c',
                                 'bytes': 'dd47f0',
                                 'instruction': 'fld qword ptr [edi - 0x10]'},
                                {'offset': '0x5f', 'bytes': 'dccd', 'instruction': 'fmul st(5), st(0)'},
                                {'offset': '0x67', 'bytes': 'dec9', 'instruction': 'fmulp st(1)'},
                                {'offset': '0x69',
                                 'bytes': 'dd9ee8000000',
                                 'instruction': 'fstp qword ptr [esi + 0xe8]'},
                                {'offset': '0x6f',
                                 'bytes': 'dd5908',
                                 'instruction': 'fstp qword ptr [ecx + 8]'},
                                {'offset': '0x72',
                                 'bytes': '83be9800000000',
                                 'instruction': 'cmp dword ptr [esi + 0x98], 0'},
                                {'offset': '0x79', 'bytes': '7513', 'instruction': 'jne 0x8e'},
                                {'offset': '0x7b',
                                 'bytes': '837f8c00',
                                 'instruction': 'cmp dword ptr [edi - 0x74], 0'},
                                {'offset': '0x7f', 'bytes': '740d', 'instruction': 'je 0x8e'},
                                {'offset': '0x81',
                                 'bytes': 'dd5618',
                                 'instruction': 'fst qword ptr [esi + 0x18]'}]}}
PRIMITIVE_LF_SHA256 = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'
# Transport metadata only; formula semantics below come from original CODE.
FORMULAS = {
    '_FoldingTetra3d': (7096, -1, 2, 0, 1, 1),
    '_addC': (7280, -1, 3, 0, 1, 1),
    '_RotatedFolding': (8481, -1, 10, 0, 1, 1),
    '_Transform': (8896, -1, 14, 8770, 1, 1),
}
LAYOUT = {
    '_FoldingTetra3d': ((-12, 'int32'), (-16, 'int32')),
    '_addC': ((-16, 'double'), (-24, 'double'), (-32, 'double')),
    '_RotatedFolding': tuple((-12-4*i, 'single') for i in range(9))+((-52, 'double'),),
    '_Transform': tuple((-16-8*i, 'double') for i in range(13))+((-116, 'int32'),),
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


def neg(value): return value ^ NEG_ZERO


def exact_add(a, b): return exact_sum([a, b])


def radius(state):
    # Every squared product and every partial subset must be exact. Merely
    # checking the original left-associated sum does not prove FMA invariance.
    v = [state[f'xyz[{i}]'] for i in range(3)]
    return exact_dot(v, v)


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
        if kind == 'int32':
            if type(value) is not int or not -(1 << 31) <= value < (1 << 31):
                raise ValueError('int32 parameter outside original storage domain')
            original = struct.unpack('<Q', struct.pack('<d', value))[0]
            prepared = fp32(value)
            struct.pack_into('<i', memory, 256+offset, value)
        else:
            original, prepared = wide32(value), value
            struct.pack_into('<I' if kind == 'single' else '<Q', memory,
                             256+offset, value if kind == 'single' else original)
        p[base+12+i] = prepared
        p[base+76+2*i:base+78+2*i] = [original & 0xffffffff, original >> 32]
    p[base+204:base+268] = struct.unpack('<64I', memory[:256])
    for i in range(32):
        p[base+268+i] = narrow64(struct.unpack_from('<Q', memory, 248-8*i)[0])
        p[base+300+i] = narrow64(struct.unpack_from('<Q', memory, 256+8*i)[0])
    return p


def _parameters(name, program):
    values = program[68:68+FORMULAS[name][2]]
    for i, (_, kind) in enumerate(LAYOUT[name]):
        if kind == 'int32':
            value = fraction64(program[132+2*i] | program[133+2*i] << 32)
            if value.denominator != 1 or not -(1 << 31) <= value < (1 << 31):
                raise ValueError('int32 metadata is not an exact signed integer')
            values[i] = int(value)
    return values


def matvec(matrix, vector, transpose=False):
    out = [0]*3
    for row in (2, 0, 1):  # Original x87 scheduling, with immutable inputs.
        coefficients = matrix[row::3] if transpose else matrix[3*row:3*row+3]
        out[row] = exact_dot(vector, coefficients)
    return out


def _point(name, state, params):
    out = dict(state)
    xyz = [state[f'xyz[{i}]'] for i in range(3)]
    if name == '_FoldingTetra3d':
        first, second = params
        if first:
            for a, b in ((0, 1), (0, 2), (1, 2)):
                # FCOMPP compares zero against sum; JB skips only when sum>0.
                # Equality therefore folds too, including sign-flipped zeros.
                if fraction(exact_add(xyz[a], xyz[b])) <= 0:
                    xyz[a], xyz[b] = neg(xyz[b]), neg(xyz[a])
        if not first or second:  # First flag zero bypasses the second flag test.
            for a, b in ((0, 1), (0, 2), (1, 2)):
                if fraction(exact_add(xyz[a], neg(xyz[b]))) <= 0:
                    xyz[a], xyz[b] = xyz[b], xyz[a]
    elif name == '_addC':
        xyz = [exact_dot([xyz[i], state[f'j[{i}]']], [ONE, params[i]]) for i in range(3)]
    elif name == '_RotatedFolding':
        xyz = matvec(params[:9], xyz)
        plus = exact_add(xyz[0], params[9]) & 0x7fffffff
        minus = exact_add(xyz[0], neg(params[9])) & 0x7fffffff
        difference = exact_add(plus, neg(minus))
        # DE E9 is FSUBP ST1,ST0; subsequent FCHS is observable at exact zero.
        xyz[0] = neg(exact_add(xyz[0], neg(difference)))
        xyz = matvec(params[:9], xyz, transpose=True)
    elif name == '_Transform':
        shifted = [exact_add(xyz[i], params[i+1]) for i in range(3)]
        xyz = [exact_mul(v, params[0]) for v in matvec(params[4:13], shifted)]
        out['xyz[3]'] = exact_mul(state['xyz[3]'], params[0])
        out['derivative[0]'] = exact_mul(state['derivative[0]'], params[0])
        # Original +152 DoJulia suppresses copying, even with a nonzero flag.
        if state['julia'] == 0 and params[13] != 0:
            for i in range(3): out[f'j[{i}]'] = xyz[i]
    else:
        raise ValueError('formula outside independent geometry model')
    for i, value in enumerate(xyz): out[f'xyz[{i}]'] = value
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
    s['rout'] = s['otrap'] = radius(s)
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
        s['rout'] = radius(s)
        if fraction(s['rout']) < fraction(s['otrap']):
            s['otrap'] = s['rout']
        if fraction(s['rout']) > fraction(s['rstop']):
            break
    if not fraction(s['xyz[3]']):
        raise ScopeError('zero W analytic distance domain excluded')
    return s


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('expected exact semantic input fields')
    identity = payload['identity']
    if not isinstance(identity, list) or len(identity) != 3 or identity[0] != 'external' or identity[2] not in FORMULAS or identity[1] != FORMULAS[identity[2]][0]:
        raise ValueError('wrong geometry formula identity')
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
    params = _parameters(name, p)
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
    return {'schema': 'mb3d.native-group03-geometry-reference.v1', 'semantic_sources': sources,
            'abi_sources': abi, 'code_audit': CODE_AUDIT,
            'dependencies': {str(dep.relative_to(ROOT)): primitive_identity(dep.read_bytes())},
            'arithmetic': 'Exact finite rational products and all partial subset sums; RN-even signed zeros',
            'model_limits': [
                'All47 state words; four original formulas; bounded single-slot analytic3D orbits only',
                'Prepared matrices are explicit ABI fixtures, not proofs of arbitrary Euler/trigonometric preparation',
                'Only signed-zero Euler preparation is independently modeled; no idealized 90-degree angles',
                'Scalar Double parameters/matrix entries are exact widened binary32; integer flags retain exact int32',
                'Nonfinite/subnormal values and rounded products/partial sums are rejected, including every orbit radius',
                'Zero-W analytic orbits are rejected; finite point transforms with zero scale remain valid',
                'No mixed-formula hybrid, 4D caller, distance-scalar, full-scene or x87 parity claim'],
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
    matrices = {
        'zero_angles': identity,
        'cycle_prepared': list(map(fp32, (0, 1, 0, 0, 0, 1, 1, 0, 0))),
        'quarter_prepared': list(map(fp32, (0, -1, 0, 1, 0, 0, 0, 0, 1))),
        'dense_prepared': list(map(fp32, (Fraction(1, 2), Fraction(1, 4), Fraction(-1, 8),
                                        Fraction(-1, 4), 1, Fraction(1, 2),
                                        Fraction(1, 8), Fraction(-1, 2), Fraction(1, 4)))),
    }
    def xyz_state(values, state=None):
        result = dict(state or _state())
        for i, value in enumerate(values): result[f'xyz[{i}]'] = value
        return result
    def transform_params(scale=ONE, shifts=None, matrix=None, copy=1):
        return [scale, *(shifts if shifts is not None else [0, 0, 0]),
                *(matrix if matrix is not None else identity), copy]
    def emit(name, variant, state, params, kind='point'):
        variant = variant.replace('/', 'over')
        semantic = {'identity': ['external', FORMULAS[name][0], name], 'kind': kind,
                    'input_state_bits': {k: word(state[k]) for k in STATE},
                    'parameter_bits': [word(v) for v in _program(name, params, state)]}
        reads = {f'xyz[{i}]' for i in range(3)}
        writes = set(reads) | {'current_slot'}
        if name == '_addC': reads |= {f'j[{i}]' for i in range(3)}
        if name == '_Transform':
            reads |= {'xyz[3]', 'derivative[0]', 'julia'}
            writes |= {'xyz[3]', 'derivative[0]'} | {f'j[{i}]' for i in range(3)}
        if kind == 'orbit':
            writes |= {f'xyz[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)}
            writes |= {'rout', 'rold', 'otrap', 'first', 'iteration', 'tmp'}
            reads |= {f'{lane}[{i}]' for lane in ('c', 'original_j') for i in range(4)}
            reads |= {'julia', 'max_iterations', 'rstop', 'hybrid_counts[0]'}
        cases.append({'id': f'{FORMULAS[name][0]}-{variant}', 'identity': semantic['identity'],
                      'variant': variant, 'input': semantic, 'expected_state_bits': evaluate(semantic),
                      'read_lanes': [k for k in STATE if k in reads],
                      'write_lanes': [k for k in STATE if k in writes], 'source_hashes': sources})
    for first, second in product((0, 1), repeat=2):
        emit('_FoldingTetra3d', f'flags_{first}_{second}', xyz_state(list(map(fp32, (-2, 1, -3)))), [first, second])
    for signs in product((-1, 1), repeat=3):
        values = [fp32(sign*(i+1)) for i, sign in enumerate(signs)]
        emit('_FoldingTetra3d', 'octant_'+'_'.join(str(v) for v in signs), xyz_state(values), [1, 1])
    for idx, values in enumerate(([0, 0, 0], [NEG_ZERO]*3, [NEG_ZERO, 0, NEG_ZERO], [0, NEG_ZERO, 0])):
        for flags in ([1, 0], [0, 0]):
            emit('_FoldingTetra3d', f'zero_boundary_{idx}_flags{flags[0]}{flags[1]}', xyz_state(values), flags)
    for suffix, middle in [('positive', fp32(-1)-1), ('zero', fp32(-1)), ('negative', fp32(-1)+1)]:
        emit('_FoldingTetra3d', 'sum_boundary_'+suffix, xyz_state([ONE, middle, 0]), [1, 0])
    for idx, flags in enumerate(([-1, 0], [0, -(1 << 31)], [(1 << 31)-1, 16777217])):
        emit('_FoldingTetra3d', f'raw_integer_flags_{idx}', _state(), flags)

    for variant, weights in [('keep', (0, 0, 0)), ('unit_j', (1, 1, 1)), ('asymmetric', (2, -1, 3)),
                             ('halves', (Fraction(1, 2), Fraction(-1, 2), Fraction(1, 4)))]:
        emit('_addC', variant, _state(), list(map(fp32, weights)))
    s = _state()
    for i in range(3): s[f'xyz[{i}]'], s[f'j[{i}]'] = fp32((1 << 20)+i+1), fp32(1 << 20)
    emit('_addC', 'large_exact_cancellation', s, [fp32(-1)]*3)
    for idx, zeros in enumerate(([NEG_ZERO]*3, [NEG_ZERO, 0, NEG_ZERO])):
        s = xyz_state(zeros)
        for i, v in enumerate(zeros): s[f'j[{i}]'] = v
        emit('_addC', f'signed_zero_{idx}', s, [ONE]*3)

    for x in (-3, -1, Fraction(-1, 2), 0, Fraction(1, 2), 1, 3):
        emit('_RotatedFolding', f'identity_x{x}', xyz_state([fp32(x), fp32(-2), fp32(-3)]), identity+[ONE])
    for fold in (0, -1, 2):
        emit('_RotatedFolding', f'fold_{fold}', _state(), identity+[fp32(fold)])
    for variant, matrix in matrices.items():
        if variant != 'zero_angles': emit('_RotatedFolding', variant, _state(), matrix+[fp32(Fraction(1, 4))])
    for column in range(3):
        emit('_RotatedFolding', f'dense_basis_{column}', xyz_state([ONE if i == column else 0 for i in range(3)]),
             matrices['dense_prepared']+[fp32(Fraction(1, 4))])
    emit('_RotatedFolding', 'negative_zero_angles', _state(), prepare_zero_angles([NEG_ZERO, 0, NEG_ZERO])+[ONE])
    emit('_RotatedFolding', 'negative_zero_vector', xyz_state([NEG_ZERO]*3), identity+[NEG_ZERO])

    for scale in (0, Fraction(1, 2), -1, 2):
        emit('_Transform', f'scale_{scale}', _state(), transform_params(fp32(scale), list(map(fp32, (1, -2, 3)))))
    for copy, julia in ((0, 0), (1, 0), (1, 1), (-1, 0), (-1, 1), (-(1 << 31), 0), ((1 << 31)-1, 0), (16777217, 1)):
        s = _state(); s['julia'] = julia
        emit('_Transform', f'copy{copy}_julia{julia}', s, transform_params(fp32(2), [ONE, 0, fp32(-1)], copy=copy))
    for variant, matrix in matrices.items():
        if variant != 'zero_angles': emit('_Transform', variant, _state(), transform_params(fp32(-2), matrix=matrix))
    for column in range(3):
        emit('_Transform', f'dense_basis_{column}', xyz_state([ONE if i == column else 0 for i in range(3)]),
             transform_params(matrix=matrices['dense_prepared'], copy=0))
    emit('_Transform', 'negative_zero_scale', xyz_state([NEG_ZERO]*3), transform_params(NEG_ZERO))
    emit('_Transform', 'negative_zero_angles', _state(), transform_params(matrix=prepare_zero_angles([NEG_ZERO, 0, NEG_ZERO])))

    for name in FORMULAS:
        default = {'_FoldingTetra3d': [1, 1], '_addC': [ONE]*3,
                   '_RotatedFolding': matrices['cycle_prepared']+[ONE],
                   '_Transform': transform_params(fp32(Fraction(1, 2)), matrix=matrices['cycle_prepared'])}[name]
        s = _state(); s['first'], s['iteration'], s['tmp'] = 0x80000000, 0x7fffffff, 0xffffffff
        emit(name, 'integer_state_sentinels', s, default)
        for variant in ('orbit_mandel', 'orbit_julia', 'orbit_escape', 'orbit_repeat', 'orbit_boundary'):
            s = _state()
            s['julia'] = int(variant in ('orbit_julia', 'orbit_repeat'))
            s['hybrid_counts[0]'] = 2 if variant == 'orbit_repeat' else 1
            if variant == 'orbit_escape': s['rstop'] = fp32(Fraction(1, 16))
            params = list(default)
            if variant == 'orbit_boundary':
                # Identity/magnitude-preserving fixtures test strict Rout>RStop.
                if name == '_addC': params = [0]*3
                if name == '_RotatedFolding': params = identity+[fp32(8)]
                if name == '_Transform': params = transform_params(matrix=matrices['cycle_prepared'])
                initial = {**s, **{f'xyz[{i}]': s[f'c[{i}]'] for i in range(3)}}
                s['rstop'] = radius(initial)
            emit(name, variant, s, params, 'orbit')
    emit('_FoldingTetra3d', 'orbit_first_only', _state(), [1, 0], 'orbit')
    emit('_Transform', 'orbit_add_shift_copy', _state(), transform_params(ONE, [fp32(Fraction(1, 2)), 0, 0]), 'orbit')
    return cases
