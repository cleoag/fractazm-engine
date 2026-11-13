"""Independent original-CODE/ABI models for four group03 Julia transforms.

Finite RN32 point and single-slot analytic3D state proofs only. Expectations
come from rational arithmetic and original machine-code operations, never CPU
translations, Metal bodies or observations. Helispiral uses ideal-RN32 rational
sin/cos enclosures; precise Metal trig need not universally equal that target.
"""
from __future__ import annotations
from fractions import Fraction
import hashlib
from pathlib import Path
import re
import struct

import native_formula_reference as affine
import native_matrix_reference as matrix
import native_group02_tail_reference as tail
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
ScopeError = tail.ScopeError
fp32, fraction, word = affine.binary32, affine.fraction, affine.word
ONE, NEG_ZERO = 0x3f800000, 0x80000000
add, mul, sub, neg = tail.add, tail.mul, tail.sub, tail.neg
SOURCE_PINS = {k: v for k, v in tail.SOURCE_PINS.items() if '/M3Formulas/' not in k}
SOURCE_PINS.update({
    'mb3d/M3Formulas/_JuliaSets.m3f': 'c1fd8e2ac9419583a38bb3b93c2daec128f4857803aecb6b955ee102c16d9e5e',
    'mb3d/M3Formulas/_PartlyJuliaRoff.m3f': '85392a20e14e8e30c695461146111aa876f2338e91fb4223eed1709288bb9a06',
    'mb3d/M3Formulas/_helispiral.m3f': '6f09cc7c37fe5466d6a39ea78ccdad0534ff0b5a5284b7af370c10f2261fa7b2',
    'mb3d/M3Formulas/_updateC2.m3f': '5631a9552d3a6f1af30b77e86234af1fae08b94ebea9d0904fd530361b8a1c36',
})
ABI_PINS = dict(matrix.ABI_PINS)
DEPENDENCY_PINS = {
    'native_formula_reference.py': 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e',
    'native_matrix_reference.py': '02da663186287cf810d0ba976af25ab32570b5235623d4cba4acd2dd4a941811',
    'native_group02_tail_reference.py': 'a8247e6156e22f8fcbb52f2e3fa7876f9c8683c8802ea9f435195b7ef9ae6cb6',
}
# Native-slot metadata only; arithmetic below is independently reconstructed
# from original CODE, not the metadata generator or translated formula bodies.
FORMULAS = {'_JuliaSets': (10003, -1, 2, 2112, 1, 1),
            '_PartlyJuliaRoff': (10005, -1, 5, 4160, 1, 1),
            '_helispiral': (10010, -1, 15, 0, 1, 1),
            '_updateC2': (10014, -1, 13, 576, 1, 1)}
MATRIX_LAYOUT = tuple((-12-4*i, 'single') for i in range(9))
LAYOUT = {'_JuliaSets': ((-16, 'double'), (-24, 'double')),
          '_PartlyJuliaRoff': tuple((-16-8*i, 'double') for i in range(4))+((0, 'constant'),),
          '_helispiral': MATRIX_LAYOUT+((-52, 'double'), (-60, 'double'),
                                        (-64, 'int32'), (-68, 'int32'),
                                        (-76, 'double'), (0, 'constant')),
          '_updateC2': MATRIX_LAYOUT+tuple((-52-8*i, 'double') for i in range(4))}
# Literal decimal constants are rounded to the original binary64 storage first.
CONSTANTS = {'_PartlyJuliaRoff': tail.binary64(Fraction('1e-10')),
             '_helispiral': tail.binary64(Fraction('0.017453293'))}
EPS = tail.narrow64(CONSTANTS['_PartlyJuliaRoff'])
DEG = tail.narrow64(CONSTANTS['_helispiral'])
CODE_AUDIT = {'_JuliaSets': {'sha256': 'b27eb8960d1cda26bc310fc005ba42d2fdba99192575f31000fa390d01e60c88',
                'bytes': 89,
                'key_offsets': [{'offset': '0x000b',
                                 'bytes': 'dd47f0',
                                 'instruction': 'fld qword ptr [edi - 0x10]'},
                                {'offset': '0x0012',
                                 'bytes': 'dd4608',
                                 'instruction': 'fld qword ptr [esi + 8]'},
                                {'offset': '0x0017', 'bytes': 'dd06', 'instruction': 'fld qword ptr [esi]'},
                                {'offset': '0x001b',
                                 'bytes': 'dd4610',
                                 'instruction': 'fld qword ptr [esi + 0x10]'},
                                {'offset': '0x0022', 'bytes': 'd9fc', 'instruction': 'frndint '},
                                {'offset': '0x0026', 'bytes': 'd9fc', 'instruction': 'frndint '},
                                {'offset': '0x002a', 'bytes': 'd9fc', 'instruction': 'frndint '},
                                {'offset': '0x0041',
                                 'bytes': 'dd5e18',
                                 'instruction': 'fstp qword ptr [esi + 0x18]'},
                                {'offset': '0x0046',
                                 'bytes': 'dd5ee0',
                                 'instruction': 'fstp qword ptr [esi - 0x20]'}]},
 '_PartlyJuliaRoff': {'sha256': 'a140d1fe6bcb03aff068d8c68d24a36cbe1d239a4a7a81b02ac33182b2a5f179',
                      'bytes': 133,
                      'key_offsets': [{'offset': '0x000c',
                                       'bytes': 'dd06',
                                       'instruction': 'fld qword ptr [esi]'},
                                      {'offset': '0x0011',
                                       'bytes': 'dd4608',
                                       'instruction': 'fld qword ptr [esi + 8]'},
                                      {'offset': '0x0017',
                                       'bytes': 'dd4610',
                                       'instruction': 'fld qword ptr [esi + 0x10]'},
                                      {'offset': '0x002d', 'bytes': 'd9fa', 'instruction': 'fsqrt '},
                                      {'offset': '0x002f',
                                       'bytes': 'dc57d8',
                                       'instruction': 'fcom qword ptr [edi - 0x28]'},
                                      {'offset': '0x0037', 'bytes': '7541', 'instruction': 'jne 0x7a'},
                                      {'offset': '0x0042',
                                       'bytes': 'dc1f',
                                       'instruction': 'fcomp qword ptr [edi]'},
                                      {'offset': '0x0049', 'bytes': '7413', 'instruction': 'je 0x5e'},
                                      {'offset': '0x004f', 'bytes': 'ded9', 'instruction': 'fcompp '},
                                      {'offset': '0x0058', 'bytes': '7502', 'instruction': 'jne 0x5c'},
                                      {'offset': '0x005e', 'bytes': 'def1', 'instruction': 'fdivrp st(1)'},
                                      {'offset': '0x0075',
                                       'bytes': 'dd5e18',
                                       'instruction': 'fstp qword ptr [esi + 0x18]'}]},
 '_helispiral': {'sha256': '9bde5143477fb32c88717de1f328ebdf7e64240356ab70513ec6e08027df22d4',
                 'bytes': 252,
                 'key_offsets': [{'offset': '0x0059',
                                  'bytes': '837fbc01',
                                  'instruction': 'cmp dword ptr [edi - 0x44], 1'},
                                 {'offset': '0x005f',
                                  'bytes': 'dd4618',
                                  'instruction': 'fld qword ptr [esi + 0x18]'},
                                 {'offset': '0x0073', 'bytes': 'd9fa', 'instruction': 'fsqrt '},
                                 {'offset': '0x007a',
                                  'bytes': '837fc001',
                                  'instruction': 'cmp dword ptr [edi - 0x40], 1'},
                                 {'offset': '0x0080',
                                  'bytes': 'dd4628',
                                  'instruction': 'fld qword ptr [esi + 0x28]'},
                                 {'offset': '0x0087',
                                  'bytes': 'dd47cc',
                                  'instruction': 'fld qword ptr [edi - 0x34]'},
                                 {'offset': '0x0091', 'bytes': 'dc0f', 'instruction': 'fmul qword ptr [edi]'},
                                 {'offset': '0x0096', 'bytes': 'd9fb', 'instruction': 'fsincos '},
                                 {'offset': '0x00a8', 'bytes': 'dee9', 'instruction': 'fsubp st(1)'},
                                 {'offset': '0x00b6',
                                  'bytes': 'd84fec',
                                  'instruction': 'fmul dword ptr [edi - 0x14]'},
                                 {'offset': '0x00f1',
                                  'bytes': 'dd19',
                                  'instruction': 'fstp qword ptr [ecx]'}]},
 '_updateC2': {'sha256': 'e54e5cf466f8c5da3df9a530be5f9344f258a1ddbc048b92d1e8f1e5de94e715',
               'bytes': 265,
               'key_offsets': [{'offset': '0x004d',
                                'bytes': 'dc4fcc',
                                'instruction': 'fmul qword ptr [edi - 0x34]'},
                               {'offset': '0x0050',
                                'bytes': 'dc47c4',
                                'instruction': 'fadd qword ptr [edi - 0x3c]'},
                               {'offset': '0x0065',
                                'bytes': 'dd4108',
                                'instruction': 'fld qword ptr [ecx + 8]'},
                               {'offset': '0x0068',
                                'bytes': 'dc4fcc',
                                'instruction': 'fmul qword ptr [edi - 0x34]'},
                               {'offset': '0x006b',
                                'bytes': 'dd5908',
                                'instruction': 'fstp qword ptr [ecx + 8]'},
                               {'offset': '0x0074', 'bytes': 'd8d1', 'instruction': 'fcom st(1)'},
                               {'offset': '0x0079', 'bytes': 'd0ec', 'instruction': 'shr ah, 1'},
                               {'offset': '0x007b', 'bytes': '7602', 'instruction': 'jbe 0x7f'},
                               {'offset': '0x007d', 'bytes': 'd9c9', 'instruction': 'fxch st(1)'},
                               {'offset': '0x007f',
                                'bytes': 'dd5e18',
                                'instruction': 'fstp qword ptr [esi + 0x18]'},
                               {'offset': '0x0082', 'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]'},
                               {'offset': '0x00f0',
                                'bytes': 'dc67c4',
                                'instruction': 'fsub qword ptr [edi - 0x3c]'},
                               {'offset': '0x00f5',
                                'bytes': 'dc67bc',
                                'instruction': 'fsub qword ptr [edi - 0x44]'},
                               {'offset': '0x00fa',
                                'bytes': 'dc67b4',
                                'instruction': 'fsub qword ptr [edi - 0x4c]'}]}}


def sqrt_bits(bits):
    """Correctly rounded positive sqrt from exact rational midpoint tests."""
    value = tail.normal_or_zero(bits)
    if value < 0: raise ScopeError('negative square root')
    if not value: return bits  # IEEE sqrt(-0)=-0.
    lo, hi = 0, 0x7f7fffff
    while lo < hi:
        mid = (lo+hi+1)//2
        if fraction(mid)**2 <= value: lo = mid
        else: hi = mid-1
    if fraction(lo)**2 == value: result = lo
    else:
        midpoint = (fraction(lo)+fraction(lo+1))/2
        result = lo if value < midpoint**2 or (value == midpoint**2 and lo % 2 == 0) else lo+1
    tail.normal_or_zero(result)
    return result


def dot(left, right):
    return tail.sum_products(list(zip(left, right)))


def rotate(vector, params, transpose=False):
    # Original first block reads offsets -12..-44 as row-major Singles;
    # the final block reads the transpose. Each output uses the pre-store vector.
    return [matrix.exact_dot(vector, [params[3*c+r] if transpose else params[3*r+c]
                         for c in range(3)]) for r in range(3)]


def _point(name, state, params):
    s = dict(state)
    if name == '_JuliaSets':
        # ESI points at C1, not current x; writes J(+24..) and xyz(-32..).
        reciprocal = tail.divide(ONE, params[0])
        for i in range(3):
            scaled = mul(state[f'c[{i}]'], reciprocal)
            center = tail.frndint(scaled)  # Real FRNDINT RN-even, not FISTP/int32.
            s[f'j[{i}]'] = mul(center, params[1])
            s[f'xyz[{i}]'] = mul(sub(scaled, center), params[0])
    elif name == '_PartlyJuliaRoff':
        delta = [sub(state[f'c[{i}]'], params[i]) for i in range(3)]
        r = sqrt_bits(dot(delta, delta))
        if fraction(r) > fraction(params[3]):
            numerator = sub(r, params[3])
            # FCOMP then AH&0x41: the guard includes equality. The following
            # 0-versus-r comparison picks positive epsilon for r==0.
            denominator = r if abs(fraction(r)) > fraction(params[4]) else params[4]
            scale = tail.divide(numerator, denominator)
            for i in range(3):
                s[f'j[{i}]'] = dot([delta[i], state[f'j[{i}]']], [scale, ONE])
    elif name == '_helispiral':
        v = rotate([state[f'xyz[{i}]'] for i in range(3)], params)
        # CMP raw dword,1 is equality, not truthiness or rounded float flags.
        radial = [state[f'j[{i}]'] for i in range(2)] if params[12] == 1 else v[:2]
        rho = sqrt_bits(dot(radial, radial))
        phi = state['j[2]'] if params[11] == 1 else v[2]
        angle = dot([rho, phi, params[13]], [params[10], params[9], params[14]])
        sine, cosine = tail.sin_bits(angle), tail.cos_bits(angle)
        v = [dot([v[0], v[1]], [cosine, sine]),
             dot([v[1], v[0]], [cosine, neg(sine)]), v[2]]
        v = rotate(v, params, transpose=True)
        for i in range(3): s[f'xyz[{i}]'] = v[i]
    elif name == '_updateC2':
        v = rotate([state[f'xyz[{i}]'] for i in range(3)], params)
        v = [dot([v[i], params[10+i]], [params[9], ONE]) for i in range(3)]
        s['xyz[3]'] = mul(state['xyz[3]'], params[9])  # ECX+8 is W.
        for i in range(3):
            # FCOM J,xyz; stack depth2 => status TOP6. SHR AH,1 leaves
            # ZF clear even at equality; JBE skips FXCH only for J<xyz.
            # Thus J>=xyz swaps, including signed-zero ties. Prose says the
            # opposite inequality; actual CODE is the authority.
            if fraction(state[f'j[{i}]']) >= fraction(v[i]):
                s[f'j[{i}]'], v[i] = v[i], state[f'j[{i}]']
        v = rotate(v, params, transpose=True)
        for i in range(3): s[f'xyz[{i}]'] = sub(v[i], params[10+i])
    else: raise ValueError('unknown original formula')
    s['current_slot'] = 0
    return s


def _program(name, params, state):
    ident, option, count, traits, flags, driver = FORMULAS[name]
    if len(params) != count: raise ValueError('wrong parameter count')
    p = [0]*2048
    p[:15] = [1, driver, 6, 0, 0, 0, 2, state['max_iterations'], state['julia'], 0, 0, 16384, 4096, 0, 0]
    p[16], p[18], p[21], p[22] = state['rstop'], ONE, fp32(1 << 20), ONE
    p[24:28] = [state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4): p[28+5*i] = ONE
    p[50] = state['hybrid_counts[0]']
    for slot in range(6): p[56+slot*332] = 1
    base = 56
    p[base:base+12] = [1, 1, ident, option & 0xffffffff, count, traits, flags, 0, 0, 1, 0, 0]
    memory = bytearray(512)
    struct.pack_into('<Q', memory, 248, 0x3fe0000000000000)
    for i, ((offset, kind), value) in enumerate(zip(LAYOUT[name], params)):
        if kind == 'int32':
            if type(value) is not int or not -(1 << 31) <= value < (1 << 31): raise ScopeError('int32 metadata range')
            struct.pack_into('<i', memory, 256+offset, value)
            original, prepared = tail.binary64(value), fp32(value)
        else:
            original = CONSTANTS[name] if kind == 'constant' else tail.wide32(value)
            prepared = tail.narrow64(original)
            if prepared != value: raise ValueError('wrong original constant')
            struct.pack_into('<I' if kind == 'single' else '<Q', memory, 256+offset,
                             value if kind == 'single' else original)
        p[base+12+i] = prepared
        p[base+76+2*i:base+78+2*i] = [original & 0xffffffff, original >> 32]
    p[base+204:base+268] = struct.unpack('<64I', memory[:256])
    for i in range(32):
        p[base+268+i] = tail.narrow64(struct.unpack_from('<Q', memory, 248-8*i)[0])
        p[base+300+i] = tail.narrow64(struct.unpack_from('<Q', memory, 256+8*i)[0])
    return p


def _parameters(name, program):
    params = program[68:68+FORMULAS[name][2]]
    for i, (_, kind) in enumerate(LAYOUT[name]):
        if kind == 'int32':
            value = tail.fraction64(program[132+2*i] | program[133+2*i] << 32)
            if value.denominator != 1 or not -(1 << 31) <= value < (1 << 31): raise ScopeError('invalid typed int32')
            params[i] = int(value)
    return params


def _orbit(name, state, params):
    # Original formulas.pas doHybridPasDE: positive-count single-slot DEoption2,
    # no smoothing, distance combination or interpolation. Distance excluded.
    s = dict(state)
    for i in range(3):
        s[f'xyz[{i}]'] = s[f'c[{i}]']
        s[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]'] = ONE
    s['first'] = s['iteration'] = 0
    s['rout'] = s['otrap'] = tail.radius(s)
    remaining = count = s['hybrid_counts[0]']; s['tmp'] = remaining
    for _ in range(s['max_iterations']):
        s['rold'] = s['rout']
        if remaining == 0: remaining = count
        s = _point(name, s, params)
        remaining -= 1; s['tmp'] = remaining; s['iteration'] += 1
        s['rout'] = tail.radius(s)
        if fraction(s['rout']) < fraction(s['otrap']): s['otrap'] = s['rout']
        if fraction(s['rout']) > fraction(s['rstop']): break
    if not fraction(s['xyz[3]']): raise ScopeError('zero W analytic distance domain excluded')
    return s


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}: raise ValueError('exact semantic fields required')
    ident = payload['identity']
    if not isinstance(ident, list) or len(ident) != 3 or ident[0] != 'external' or ident[2] not in FORMULAS or ident[1] != FORMULAS[ident[2]][0]: raise ValueError('wrong Julia-group identity')
    name = ident[2]
    def decode(value):
        if not isinstance(value, str) or not re.fullmatch(r'0x[0-9a-f]{8}', value): raise ValueError('expected hex32')
        return int(value, 16)
    if set(payload['input_state_bits']) != set(STATE) or len(payload['parameter_bits']) != 2048: raise ValueError('wrong ABI width')
    s = {k: decode(v) for k, v in payload['input_state_bits'].items()}; p = [decode(v) for v in payload['parameter_bits']]
    for lane in STATE[:33]: tail.normal_or_zero(s[lane])
    if payload['kind'] not in ('point', 'orbit') or s['julia'] not in (0, 1) or s['inside'] or s['calc_smooth']: raise ScopeError('unsupported analytic3D caller')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3: raise ScopeError('work bound exceeded')
    params = _parameters(name, p)
    if p != _program(name, params, s): raise ValueError('typed prepared ABI differs')
    if payload['kind'] == 'orbit':
        if s['vary_scale'] != ONE or s['j[3]'] != s['original_j[3]']: raise ScopeError('unsupported analytic entry state')
        out = _orbit(name, s, params)
    else: out = _point(name, s, params)
    for lane in STATE[:33]: tail.normal_or_zero(out[lane])
    return {k: word(out[k]) for k in STATE}


def dependency_paths():
    return [Path(__file__).with_name(name).resolve() for name in DEPENDENCY_PINS]


def manifest():
    def verified(pins, directory=ROOT):
        result = {}
        for name, pin in pins.items():
            path = directory/name; raw = path.read_bytes()
            if hashlib.sha256(raw.replace(b'\r\n', b'\n')).hexdigest() != pin: raise ValueError('frozen source/dependency changed: '+name)
            result[str(path.relative_to(ROOT))] = hashlib.sha256(raw).hexdigest()
        return result
    deps = verified(DEPENDENCY_PINS, Path(__file__).parent)
    sources, abi = verified(SOURCE_PINS), verified(ABI_PINS)
    for name, audit in CODE_AUDIT.items():
        text = (ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code = bytes.fromhex(re.sub(r'\s+', '', re.search(r'\[CODE\](.*?)\[END\]', text, re.S)[1]))
        if len(code) != audit['bytes'] or hashlib.sha256(code).hexdigest() != audit['sha256']: raise ValueError('original CODE changed')
        for op in audit['key_offsets']:
            at, expected = int(op['offset'], 16), bytes.fromhex(op['bytes'])
            if code[at:at+len(expected)] != expected: raise ValueError('original opcode changed')
    return {'schema': 'mb3d.native-group03-julia-reference.v1', 'semantic_sources': sources,
            'abi_sources': abi, 'dependencies': deps, 'code_audit': CODE_AUDIT,
            'arithmetic': 'Exact rational RN32; sqrt midpoint proof; FRNDINT nearest-even; singleton FMA/grouping controls; ideal-RN32 Taylor trig',
            'model_limits': [
                'Four original CODE bodies, all47 lanes, bounded point/single-slot analytic3D orbit; no distance scalar/full scenes',
                'JuliaSets and PartlyJuliaRoff read immutable C; updateC2 scales W and CODE swaps J>=xyz including ties',
                'Parameters are exact widened binary32 reals plus original decimal constants and exact typed int32 flags',
                'Single matrices require exact products/all partial sums; prepared ABI fixtures, authored Euler preparation only +/-zero',
                'updateC2 FCOM/SHR/JBE proof assumes original balanced x87 caller with empty TOP0 entry',
                'Finite normal/zero arithmetic only; ambiguous grouping/FMA and singular/zero-W orbit inputs fail closed',
                'Helispiral ideal-RN32 sin/cos targets are independent; Metal trig not universally RN32 and mismatches remain diagnostics',
                'No arbitrary angles, hybrid compositions, x87 parity, nonfinite/range exits or GPU qualification implied'],
            'model_sha256': hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def _state():
    return matrix._state()


def _parameters_default(name):
    identity = matrix.prepare_zero_angles([0, 0, 0])
    if name == '_JuliaSets': return [fp32(2), fp32(Fraction(1, 4))]
    if name == '_PartlyJuliaRoff': return [0, 0, 0, 0, EPS]
    if name == '_helispiral': return identity+[fp32(Fraction(1, 1 << 20)), 0, 0, 0, 0, DEG]
    return identity+[ONE, 0, 0, 0]


def build_cases():
    sources = manifest()['semantic_sources']; cases = []
    def emit(name, variant, state, params, kind='point'):
        payload = {'identity': ['external', FORMULAS[name][0], name], 'kind': kind,
                   'input_state_bits': {k: word(state[k]) for k in STATE},
                   'parameter_bits': [word(v) for v in _program(name, params, state)]}
        reads, writes = set(), {'current_slot'}
        if name in ('_JuliaSets', '_PartlyJuliaRoff'): reads |= {f'c[{i}]' for i in range(3)}
        if name != '_JuliaSets': reads |= {f'j[{i}]' for i in range(3)}
        if name in ('_helispiral', '_updateC2'): reads |= {f'xyz[{i}]' for i in range(4 if name == '_updateC2' else 3)}
        if name != '_helispiral': writes |= {f'j[{i}]' for i in range(3)}
        if name != '_PartlyJuliaRoff': writes |= {f'xyz[{i}]' for i in range(4 if name == '_updateC2' else 3)}
        if kind == 'orbit':
            reads |= {f'{v}[{i}]' for v in ('c', 'original_j') for i in range(4)} | {'julia','max_iterations','rstop','hybrid_counts[0]'}
            writes |= {f'xyz[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)} | {'first','iteration','tmp','rout','rold','otrap'}
        try: expected = evaluate(payload)
        except Exception as exc: raise ValueError(f'fixture {name}/{variant}: {exc}') from exc
        cases.append({'id': f'{FORMULAS[name][0]}-{variant}', 'identity': payload['identity'],
                      'variant': variant, 'input': payload, 'expected_state_bits': expected,
                      'read_lanes': [k for k in STATE if k in reads], 'write_lanes': [k for k in STATE if k in writes],
                      'source_hashes': sources})
    def vector(s, lane, values):
        for i, value in enumerate(values): s[f'{lane}[{i}]'] = int(value, 16) if isinstance(value, str) else fp32(value)
    name = '_JuliaSets'
    for variant, coords, separation, factor in [
        ('positive_ties', (1,3,5), 2, Fraction(1,4)),
        ('negative_ties', (-1,-3,-5), 2, Fraction(1,4)),
        ('negative_separation', (1,3,-5), -2, Fraction(-1,2)),
        ('fractional_separation', (Fraction(3,8),Fraction(-5,8),Fraction(1,16)), Fraction(1,2), 2),
        ('signed_zero', ('0x80000000',0,-1), 2, 0),
        ('beyond_int32', ((1<<31)+256,-(1<<31),7), 1, Fraction(1,2)),
        ('asymmetric_C_vs_xyz', (Fraction(1,4),Fraction(9,4),Fraction(-11,4)), 2, 1)]:
        s = _state(); vector(s, 'c', coords); emit(name, variant, s, [fp32(separation),fp32(factor)])
    for raw, variant in [(0x3effffff,'below_half'),(0x3f000001,'above_half')]:
        s=_state();s['c[0]']=raw;emit(name,variant,s,[ONE,ONE])
    name = '_PartlyJuliaRoff'
    for variant, coords, radius, center, j in [
        ('inside', (3,4,0), 6, (0,0,0), (1,2,3)),
        ('on_radius', (3,4,0), 5, (0,0,0), (1,2,3)),
        ('outside_exact', (3,4,0), 0, (0,0,0), (1,-2,3)),
        ('outside_fractional', (3,4,0), 4, (0,0,0), (0,0,0)),
        ('offset_sphere', (4,6,3), 0, (1,2,3), (1,2,3)),
        ('epsilon_guard', (Fraction(1,1<<40),0,0), 0, (0,0,0), (0,0,0)),
        ('zero_radius_negative_bound', (0,0,0), -1, (0,0,0), (0,0,0)),
        ('signed_zero_inside', ('0x80000000',0,0), 0, (0,0,0), ('0x80000000',0,1)),
        ('irrational_sqrt', (1,1,0), 0, (0,0,0), (0,0,0)),
        ('irrational_shell', (1,1,0), 1, (0,0,0), (0,0,0)),
        ('epsilon_equal', (word(EPS),0,0), 0, (0,0,0), (0,0,0)),
        ('epsilon_above', (word(mul(EPS,fp32(2))),0,0), 0, (0,0,0), (0,0,0))]:
        s=_state();vector(s,'c',coords);vector(s,'j',j)
        emit(name,variant,s,[fp32(v) for v in (*center,radius)]+[EPS])
    name = '_helispiral'
    for h, sp in [(0,0),(1,0),(0,1),(1,1),(2,-1),(16777217,-2147483648)]:
        s=_state();vector(s,'xyz',(1,0,3));vector(s,'j',(3,4,2));p=_parameters_default(name)
        p[10]=fp32(Fraction(1,1<<20));p[11:13]=[h,sp]
        emit(name,f'flags_{h}_{sp}',s,p)
    for sign in (-1,1):
        s=_state();vector(s,'xyz',(1,0,1));p=_parameters_default(name);p[9]=fp32(Fraction(sign,8))
        emit(name,'moderate_angle_'+str(sign),s,p)
    for variant, m in [('cycle_prepared',[0,1,0,0,0,1,1,0,0]),('quarter_prepared',[0,-1,0,1,0,0,0,0,1])]:
        s=_state();vector(s,'xyz',(1,0,1));p=_parameters_default(name);p[:9]=[fp32(v) for v in m]
        emit(name,variant,s,p)
    for variant, fix in [('fixed_angle',Fraction(1,1<<12)),('negative_fixed',Fraction(-1,1<<12))]:
        s=_state();vector(s,'xyz',(1,0,0));p=_parameters_default(name);p[9]=0;p[13]=fp32(fix)
        emit(name,variant,s,p)
    s=_state();vector(s,'xyz',('0x80000000','0x80000000',0));p=_parameters_default(name);p[9]=0
    emit(name,'signed_zero_rotation',s,p)
    name = '_updateC2'
    for variant, coords, j, scale, offsets in [
        ('mixed_branches',(1,2,3),(2,1,3),1,(0,0,0)),
        ('all_swap',(1,2,3),(4,5,6),1,(0,0,0)),
        ('no_swap',(4,5,6),(1,2,3),1,(0,0,0)),
        ('negative_scale',(1,-2,3),(0,0,0),-2,(1,2,-1)),
        ('zero_scale',(1,2,3),(0,0,0),0,(1,-2,3)),
        ('scale_offsets',(1,2,3),(4,0,8),2,(1,-1,2)),
        ('signed_zero_ties',(0,0,0),('0x80000000',0,'0x80000000'),1,(0,0,0))]:
        s=_state();vector(s,'xyz',coords);vector(s,'j',j);p=_parameters_default(name);p[9:]=[fp32(v) for v in (scale,*offsets)]
        emit(name,variant,s,p)
    for variant,m in [('cycle_prepared',[0,1,0,0,0,1,1,0,0]),('dense_prepared',[1,Fraction(1,2),0,0,1,Fraction(-1,4),Fraction(1,8),0,1])]:
        s=_state();p=_parameters_default(name);p[:9]=[fp32(v) for v in m];emit(name,variant,s,p)
    # Integer sentinel fields must survive point evaluation without float routing.
    for name in FORMULAS:
        s=_state();s.update(first=0xffffffff, iteration=0x1000001,tmp=0x80000000)
        vector(s,'xyz',(1,0,1));vector(s,'c',(1,0,1));emit(name,'integer_sentinels',s,_parameters_default(name))
    # True early exit and equal-threshold controls have max_iterations > 1.
    for name in FORMULAS:
        for julia in (0,1):
            s=_state();s['julia']=julia;s['hybrid_counts[0]']=3
            vector(s,'c',(Fraction(1,4),0,Fraction(1,2)));vector(s,'original_j',(1,0,1))
            s['j[3]']=s['original_j[3]'];p=_parameters_default(name)
            emit(name,'orbit_julia' if julia else 'orbit_mandel',s,p,'orbit')
        for variant,stop in [('orbit_escape',Fraction(1,256)),('orbit_equal_threshold',Fraction(5,16))]:
            s=_state();vector(s,'c',(Fraction(1,4),0,Fraction(1,2)));p=_parameters_default(name)
            s['rstop']=fp32(stop);s['hybrid_counts[0]']=3
            if name=='_helispiral':p[9]=0
            emit(name,variant,s,p,'orbit')
    for scale in (-2,2):
        name='_updateC2';s=_state();vector(s,'c',(Fraction(1,4),0,Fraction(1,2)))
        p=_parameters_default(name);p[9]=fp32(scale)
        emit(name,'orbit_W_scale_'+str(scale),s,p,'orbit')
    return cases
