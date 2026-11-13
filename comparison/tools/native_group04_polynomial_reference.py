"""Independent bounded original-M3F polynomial targets for group04.

The scalar operations are exact finite rational algebra, including exact square
roots and divisions. These pre-device targets do not promise general correctly
rounded Metal math, x87 image parity, arbitrary orbits, or distance gradients.
"""
from __future__ import annotations
from fractions import Fraction
import hashlib
from itertools import combinations, product
from math import isqrt
from pathlib import Path
import re
import struct
import native_formula_reference as affine
from gpu_pipeline_proofs import STATE
ROOT = Path(__file__).resolve().parents[2]
CALLER_PROFILE = {
    'path': 'comparison/results/native-formula-group04-20261101/caller-profile.json',
    'sha256': '52a02f1b539b022d9975baf0ac027e3463d5441aa755c4bee143506aa1e1ed6e',
    'scope': 'New original FPC i386/SSE2 doHybridPas; positive counts; no smoothing',
}
SOURCE_PINS = {'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
 'mb3d/DivUtils.pas': '879221cb1d6abb371452f1f04afcf5315af238317dcb31b6359a145cb303da96',
 'mb3d/M3Formulas/PseudoXDB.m3f': 'b8e8d6001903631d3ee67096040c7b5e4461ed28bbcf274dd339a470eb5a4ecb',
 'mb3d/M3Formulas/IdesFormula.m3f': 'c974714dc49d210a231f8006e0489d02ccb513d278f14a70a2e0af1a8ed7af2d',
 'mb3d/M3Formulas/Quadrat3D.m3f': '35244971e92d6eefa1e961ecc8b09493e9b95e555c6cb338f3afc80ffc0149a9'}
ABI_PINS = {'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a',
 'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6'}
CODE_AUDIT = {'PseudoXDB': {'sha256': '590454d3442ba2c33c3f9f9afcec4d9bd80121972a163650d166f79f74c6636c',
               'bytes': 79,
               'key_offsets': [{'offset': '0x1f', 'bytes': 'd8e2', 'instruction': 'fsub st(2)'},
                               {'offset': '0x21',
                                'bytes': 'dc4ff0',
                                'instruction': 'fmul qword ptr [edi - 0x10]'},
                               {'offset': '0x27',
                                'bytes': 'dd19',
                                'instruction': 'fstp qword ptr [ecx]'},
                               {'offset': '0x2d', 'bytes': 'd9fa', 'instruction': 'fsqrt '},
                               {'offset': '0x2f', 'bytes': 'def9', 'instruction': 'fdivp st(1)'},
                               {'offset': '0x3c',
                                'bytes': 'dd1a',
                                'instruction': 'fstp qword ptr [edx]'},
                               {'offset': '0x40', 'bytes': 'dee2', 'instruction': 'fsubrp st(2)'},
                               {'offset': '0x44',
                                'bytes': 'dc4618',
                                'instruction': 'fadd qword ptr [esi + 0x18]'}]},
 'IdesFormula': {'sha256': '3e6341bf056ac983c588258c829a54ef632fb11be44379e0c98462bc21ab8bed',
                 'bytes': 83,
                 'key_offsets': [{'offset': '0x13', 'bytes': 'd8ca', 'instruction': 'fmul st(2)'},
                                 {'offset': '0x15', 'bytes': 'd8cb', 'instruction': 'fmul st(3)'},
                                 {'offset': '0x17',
                                  'bytes': 'dc4ee8',
                                  'instruction': 'fmul qword ptr [esi - 0x18]'},
                                 {'offset': '0x1d',
                                  'bytes': 'dd1a',
                                  'instruction': 'fstp qword ptr [edx]'},
                                 {'offset': '0x2b', 'bytes': 'd8c3', 'instruction': 'fadd st(3)'},
                                 {'offset': '0x2d',
                                  'bytes': 'dc4ed8',
                                  'instruction': 'fmul qword ptr [esi - 0x28]'},
                                 {'offset': '0x35', 'bytes': 'dee1', 'instruction': 'fsubrp st(1)'},
                                 {'offset': '0x43',
                                  'bytes': 'dc4ed0',
                                  'instruction': 'fmul qword ptr [esi - 0x30]'},
                                 {'offset': '0x46', 'bytes': 'dee9', 'instruction': 'fsubp st(1)'}]},
 'Quadrat3D': {'sha256': '7596ffd3ed54b8db67e8090996647a834ecbcb553cfeb7821a9a7a1774fa4c38',
               'bytes': 166,
               'key_offsets': [{'offset': '0x1c',
                                'bytes': 'dd47d8',
                                'instruction': 'fld qword ptr [edi - 0x28]'},
                               {'offset': '0x1f', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                               {'offset': '0x21', 'bytes': 'dec1', 'instruction': 'faddp st(1)'},
                               {'offset': '0x25', 'bytes': 'd8d9', 'instruction': 'fcomp st(1)'},
                               {'offset': '0x27', 'bytes': 'dfe0', 'instruction': 'fnstsw ax'},
                               {'offset': '0x29', 'bytes': '9e', 'instruction': 'sahf '},
                               {'offset': '0x2a', 'bytes': '7766', 'instruction': 'ja 0x92'},
                               {'offset': '0x2c', 'bytes': '746c', 'instruction': 'je 0x9a'},
                               {'offset': '0x46', 'bytes': 'd8f3', 'instruction': 'fdiv st(3)'},
                               {'offset': '0x61', 'bytes': 'd8f4', 'instruction': 'fdiv st(4)'},
                               {'offset': '0x72', 'bytes': 'd9fa', 'instruction': 'fsqrt '},
                               {'offset': '0x92', 'bytes': 'd9ee', 'instruction': 'fldz '},
                               {'offset': '0x9a',
                                'bytes': 'dd4618',
                                'instruction': 'fld qword ptr [esi + 0x18]'}]}}
PRIMITIVE_LF_SHA256 = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'
# Transport only. Quadrat has FIVE authored parameters; its wrapper appends J
# dynamically. Synthetic J values are NOT authored PVar or metadata parameters.
FORMULAS = {
    'PseudoXDB': (7025, 0, 1, 0, 0, 0),
    'IdesFormula': (7046, 0, 5, 0, 0, 0),
    'Quadrat3D': (9813, 0, 5, 0, 1, 0),
}
LAYOUT = {name: tuple((-16-8*i, 'double') for i in range(row[2]))
          for name, row in FORMULAS.items()}
fp32, fraction, word = affine.binary32, affine.fraction, affine.word
ONE, NEG_ZERO = 0x3f800000, 0x80000000

class ScopeError(ValueError):
    """No finite exact proof exists for this input in this bounded model."""

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
    p[:15] = [1, driver, 6, 0, 0, 0, 0, state['max_iterations'], state['julia'],
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

def exact_product(values):
    # Check every grouping before choosing original CODE's left-to-right order.
    for count in range(2, len(values)+1):
        for subset in combinations(values, count):
            value = Fraction(1)
            for v in subset: value *= normal_or_zero(v)
            if normal_or_zero(fp32(value)) != value:
                raise ScopeError('rounded regrouped product outside exact domain')
    out = values[0]
    for v in values[1:]: out = exact_mul(out, v)
    return out


def exact_divide(numerator, denominator):
    a, b = normal_or_zero(numerator), normal_or_zero(denominator)
    if not b: raise ScopeError('singular division is not a finite successful case')
    value = a / b
    out = fp32(value, bool((numerator ^ denominator) & NEG_ZERO))
    if normal_or_zero(out) != value:
        raise ScopeError('nonrepresentable exact quotient remains OPEN')
    return out


def exact_sqrt(bits):
    value = normal_or_zero(bits)
    if value < 0: raise ScopeError('negative square root outside finite domain')
    if not value: return bits
    n, d = isqrt(value.numerator), isqrt(value.denominator)
    if n*n != value.numerator or d*d != value.denominator:
        raise ScopeError('irrational square root remains OPEN')
    exact = Fraction(n, d)
    result = fp32(exact)
    if normal_or_zero(result) != exact:
        raise ScopeError('nonrepresentable square root remains OPEN')
    return result


def _point(name, state, params):
    out = dict(state)
    x, y, z = (state[f'xyz[{i}]'] for i in range(3))
    jx, jy, jz = (state[f'j[{i}]'] for i in range(3))
    xx, yy = exact_mul(x, x), exact_mul(y, y)
    if name == 'PseudoXDB':
        # Original stack: y², x², duplicate/add => radial; abs(z) is retained.
        # Z is stored before Y, but saved original X²/Y² keep X independent.
        radial = exact_add(xx, yy)
        k = exact_divide(exact_add(z & 0x7fffffff, z & 0x7fffffff), exact_sqrt(radial))
        zz = exact_mul(z, z)
        new_z = exact_add(exact_mul(exact_add(zz, neg(radial)), params[0]), jz)
        new_y = exact_add(exact_product([k, x, y, fp32(2)]), jy)
        # DE E2 after FXCH: ST2 = ST0 - ST2, namely X² - Y².
        new_x = exact_add(exact_mul(exact_add(xx, neg(yy)), k), jx)
    elif name == 'IdesFormula':
        # Y stores first; the stack retains original XYZ for all squared terms.
        new_y = exact_add(exact_product([x, y, z, params[1]]), jy)
        zz = exact_mul(z, z)
        new_x = exact_sum([exact_mul(xx, params[0]),
                           neg(exact_mul(exact_add(zz, yy), params[3])), jx])
        new_z = exact_sum([exact_mul(zz, params[2]),
                           neg(exact_mul(exact_add(yy, xx), params[4])), jz])
    elif name == 'Quadrat3D':
        # H = X² + (Y² + HRadius), then FCOMP compares +0 against H.
        # SAHF JA -> H<0, JE -> H==0; NaNs are not finite successful cases.
        exact_sum([xx, yy, params[3]])  # Prove all contraction/grouping alternatives.
        h = exact_add(xx, exact_add(yy, params[3]))
        if fraction(h) < 0:
            new_x = new_y = new_z = 0  # FLDZ, not J and not incoming signed zero.
        elif not fraction(h):
            new_x, new_y, new_z = jx, jy, jz
        else:
            zz = exact_mul(z, z)
            factor = exact_add(params[4], neg(exact_divide(zz, h)))
            new_x = exact_add(exact_product([exact_add(xx, neg(yy)), params[0], factor]), jx)
            new_y = exact_add(exact_product([x, y, params[1], factor]), jy)
            new_z = exact_add(exact_product([z, exact_sqrt(h), params[2]]), jz)
    else:
        raise ValueError('unknown independent polynomial formula')
    for i, value in enumerate((new_x, new_y, new_z)): out[f'xyz[{i}]'] = value
    out['current_slot'] = 0
    return out


def _orbit(name, state, params):
    # Original formulas.pas doHybridPas (not doHybridPasDE): W starts at +0.
    s = dict(state)
    for i in range(3):
        s[f'xyz[{i}]'] = s[f'c[{i}]']
        s[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]'] = 0
    s['first'] = s['iteration'] = 0
    s['rout'] = s['otrap'] = radius(s)
    # Independent compiled-original caller proof, not formula record fields.
    s['sse[0]'] = exact_add(exact_product([s['xyz[0]']]*2), exact_product([s['xyz[1]']]*2))
    s['sse[1]'], s['sse[2]'], s['sse[3]'] = 0, s['rout'], 0
    remaining = count = s['hybrid_counts[0]']
    s['tmp'] = remaining
    for _ in range(s['max_iterations']):
        s['rold'] = s['rout']
        if remaining == 0: remaining = count
        s = _point(name, s, params)
        remaining -= 1
        s['tmp'] = remaining
        s['iteration'] += 1
        s['rout'] = radius(s)
        if fraction(s['rout']) < fraction(s['otrap']): s['otrap'] = s['rout']
        # Original maxIt short-circuits before CVTSS2SD loads RStop.
        s['sse[0]'] = s['rout'] if s['iteration'] >= s['max_iterations'] else s['rstop']
        s['sse[1]'], s['sse[2]'], s['sse[3]'] = 0, s['rout'], 0
        if fraction(s['rout']) > fraction(s['rstop']): break
    return s


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('expected exact semantic input fields')
    identity = payload['identity']
    if not isinstance(identity, list) or len(identity) != 3 or identity[0] != 'external' or identity[2] not in FORMULAS or identity[1] != FORMULAS[identity[2]][0]:
        raise ValueError('wrong polynomial formula identity')
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
        raise ScopeError('caller outside bounded nonanalytic3D model')
    if fraction(s['rstop']) <= 0:
        raise ScopeError('nonpositive bailout radius is outside the caller domain')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3:
        raise ScopeError('caller work bounds exceeded')
    params = _parameters(name, p)
    if p != _program(name, params, s):
        raise ValueError('typed prepared parameter ABI differs')
    if payload['kind'] == 'orbit':
        if s['vary_scale'] != ONE or s['j[3]'] != s['original_j[3]']:
            raise ScopeError('unsupported nonanalytic3D entry state')
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
    if hashlib.sha256((ROOT/CALLER_PROFILE['path']).read_bytes()).hexdigest() != CALLER_PROFILE['sha256']:
        raise ValueError('pinned independent caller proof changed')
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
    return {'schema': 'mb3d.native-group04-polynomial-reference.v2',
            'caller_profile': dict(CALLER_PROFILE),
            'semantic_sources': sources, 'abi_sources': abi, 'code_audit': CODE_AUDIT,
            'dependencies': {str(dep.relative_to(ROOT)): primitive_identity(dep.read_bytes())},
            'arithmetic': 'Exact finite rational polynomial algebra; exact square roots and quotients; all subset sums/products checked',
            'model_limits': [
                'Three original CODE bodies; all47 state words; bounded single-slot nonanalytic Hybrid3D caller',
                'Authored Double parameters are exact widened binary32; Quadrat has five authored parameters, mutable J comes from caller state',
                'PseudoXDB zero radial denominator is explicitly excluded; no epsilon is invented',
                'Quadrat negative H returns positive zeros; zero H copies mutable J; only positive H reaches sqrt/division',
                'Irrational roots, nonrepresentable quotients, rounded products or partial sums, subnormal/nonfinite arithmetic are rejected',
                'Exact perfect-root/quotient targets do not assert universal correctly-rounded Metal math; device disagreements remain OPEN',
                'No arbitrary mixed neighbour, smoothing, distance/gradient, image, FP32 range fallback, or x87 parity qualification'],
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
    defaults = {'PseudoXDB': [ONE],
                'IdesFormula': list(map(fp32, (1, 2, 1, Fraction(1, 2), Fraction(1, 2)))),
                'Quadrat3D': list(map(fp32, (1, 2, -2, 0, 1)))}

    def state_xyz(values, *, raw=False):
        s = _state()
        for i, value in enumerate(values): s[f'xyz[{i}]'] = value if raw else fp32(value)
        return s

    def emit(name, variant, s, params=None, kind='point'):
        params = defaults[name] if params is None else params
        payload = {'identity': ['external', FORMULAS[name][0], name], 'kind': kind,
                   'input_state_bits': {k: word(s[k]) for k in STATE},
                   'parameter_bits': [word(v) for v in _program(name, params, s)]}
        reads = {f'{lane}[{i}]' for lane in ('xyz', 'j') for i in range(3)}
        writes = {f'xyz[{i}]' for i in range(3)} | {'current_slot'}
        if kind == 'orbit':
            writes |= {f'xyz[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)}
            writes |= {'rout', 'rold', 'otrap', 'first', 'iteration', 'tmp'}
            writes |= {f'sse[{i}]' for i in range(4)}
            reads |= {f'{lane}[{i}]' for lane in ('c', 'original_j') for i in range(4)}
            reads |= {'julia', 'max_iterations', 'rstop', 'hybrid_counts[0]'}
        cases.append({'id': f'{FORMULAS[name][0]}-{variant}', 'identity': payload['identity'],
                      'variant': variant, 'input': payload, 'expected_state_bits': evaluate(payload),
                      'read_lanes': [k for k in STATE if k in reads],
                      'write_lanes': [k for k in STATE if k in writes], 'source_hashes': sources})

    for name in FORMULAS:
        emit(name, 'asymmetric_sentinel_state', _state())
        for signs in product((-1, 1), repeat=3):
            emit(name, 'octant_'+'_'.join(map(str, signs)),
                 state_xyz([sign*value for sign, value in zip(signs, (3, 4, 5))]))
        s = _state()
        s['first'], s['iteration'], s['tmp'] = 0x80000000, 0x7fffffff, 0xffffffff
        emit(name, 'integer_state_sentinels', s)
        # Deliberately distinct C/originalJ versus mutable J, then perturb J only.
        s = _state()
        for i, value in enumerate((2, -3, 4)): s[f'j[{i}]'] = fp32(value)
        emit(name, 'mutable_J_not_C_or_originalJ', s)

    for multiplier in (0, -1, 2, Fraction(1, 2)):
        emit('PseudoXDB', 'z_multiplier_'+str(multiplier).replace('/', 'over'),
             state_xyz((3, 4, 10)), [fp32(multiplier)])
    for i, values in enumerate(((0, 2, 1), (2, 0, -1), (Fraction(3, 4), 1, Fraction(5, 4)))):
        emit('PseudoXDB', f'axis_or_dyadic_{i}', state_xyz(values))
    for sign in (0, NEG_ZERO):
        s = state_xyz([fp32(-2), NEG_ZERO, sign], raw=True)
        for i in range(3): s[f'j[{i}]'] = NEG_ZERO
        emit('PseudoXDB', f'abs_z_signed_zero_{sign:08x}', s)

    for index in range(5):
        params = list(defaults['IdesFormula']); params[index] = fp32(-2 if index & 1 else Fraction(1, 4))
        emit('IdesFormula', f'parameter_{index}_independent', _state(), params)
    emit('IdesFormula', 'all_zero_multipliers_copy_J', _state(), [0]*5)
    for i, values in enumerate(((0, 2, -3), (2, 0, -3), (2, -3, 0))):
        emit('IdesFormula', f'zero_axis_{i}', state_xyz(values))
    s = state_xyz([NEG_ZERO]*3, raw=True)
    for i in range(3): s[f'j[{i}]'] = NEG_ZERO
    emit('IdesFormula', 'negative_zero_product_and_addition', s)

    for variant, h in [('negative', fp32(-2)), ('zero', fp32(-1)),
                       ('positive_ulp', fp32(-1)-1), ('negative_ulp', fp32(-1)+1),
                       ('positive', fp32(3))]:
        params = list(defaults['Quadrat3D']); params[3] = h
        emit('Quadrat3D', 'H_branch_'+variant, state_xyz((1, 0, 0)), params)
    for index, value in ((0, -2), (1, Fraction(1, 2)), (2, 2), (4, 2), (4, -1)):
        params = list(defaults['Quadrat3D']); params[index] = fp32(value)
        emit('Quadrat3D', f'parameter_{index}_{value}'.replace('/', 'over'), state_xyz((3, 4, 10)), params)
    params = list(defaults['Quadrat3D']); params[3] = fp32(7)
    emit('Quadrat3D', 'nonzero_HRadius_perfect_square', state_xyz((3, 0, 2)), params)
    for h in (-1, 0, 1):
        s = state_xyz([NEG_ZERO]*3, raw=True)
        for i in range(3): s[f'j[{i}]'] = NEG_ZERO
        params = list(defaults['Quadrat3D']); params[3] = fp32(h)
        emit('Quadrat3D', f'zero_sign_H_{h}', s, params)

    def orbit(name, variant, c, j=(0, 0, 0), *, julia=1, iterations=3, count=1, stop=1 << 20, params=None):
        s = _state(); s['julia'] = julia; s['max_iterations'] = iterations
        s['hybrid_counts[0]'] = count; s['rstop'] = fp32(stop)
        for i, value in enumerate(c): s[f'c[{i}]'] = fp32(value)
        for i, value in enumerate(j): s[f'original_j[{i}]'] = fp32(value)
        emit(name, variant, s, params, 'orbit')

    orbit('PseudoXDB', 'orbit_mandel_four', (1, 0, 0), julia=0, iterations=4)
    orbit('PseudoXDB', 'orbit_mandel_other_axis', (0, 1, 0), julia=0, iterations=2)
    orbit('PseudoXDB', 'orbit_julia_dyadic_two', (Fraction(3, 8), Fraction(-1, 2), Fraction(5, 8)), iterations=2)
    orbit('PseudoXDB', 'orbit_julia_exact_fixed_point', (3, 4, 0), (3, 4, 25), count=2, iterations=4)
    orbit('PseudoXDB', 'orbit_strict_escape', (1, 0, 0), julia=0, stop=1)
    orbit('PseudoXDB', 'orbit_equality_continues', (1, 0, 0), julia=0, stop=2)
    orbit('PseudoXDB', 'orbit_negative_multiplier', (1, 0, 0), julia=0, params=[fp32(-1)], iterations=4, count=3)

    orbit('IdesFormula', 'orbit_mandel_dyadic_two', (Fraction(1, 2), 0, 0), julia=0, iterations=2)
    orbit('IdesFormula', 'orbit_julia_three', (1, 0, 0), iterations=3)
    params = list(map(fp32, (1, 2, 1, 0, 0)))
    orbit('IdesFormula', 'orbit_triple_product_growth', (1, 1, 1), params=params, count=2, iterations=4)
    orbit('IdesFormula', 'orbit_equality_continues', (1, 0, 0), julia=0, params=params, stop=4)
    orbit('IdesFormula', 'orbit_strict_escape', (1, 0, 0), julia=0, params=params, stop=3)
    orbit('IdesFormula', 'orbit_zero_weights_copies_J', (1, 2, -3), (Fraction(1, 2), -1, 2), params=[0]*5, count=3)

    orbit('Quadrat3D', 'orbit_mandel_four', (1, 0, 0), julia=0, iterations=4)
    orbit('Quadrat3D', 'orbit_julia_pythagorean_two', (3, 4, 0), iterations=2, count=2)
    orbit('Quadrat3D', 'orbit_positive_then_zero_H', (1, 0, 1), iterations=3)
    params = list(defaults['Quadrat3D']); params[3] = fp32(-2)
    orbit('Quadrat3D', 'orbit_negative_H_ignores_J', (1, 0, 0), (2, 3, 4), params=params)
    params = list(defaults['Quadrat3D']); params[3] = fp32(-1)
    orbit('Quadrat3D', 'orbit_zero_H_mutable_J', (1, 0, 0), (0, 1, 0), params=params, count=3)
    orbit('Quadrat3D', 'orbit_equality_continues', (1, 0, 0), julia=0, stop=4)
    orbit('Quadrat3D', 'orbit_strict_escape', (1, 0, 0), julia=0, stop=3)
    return cases
