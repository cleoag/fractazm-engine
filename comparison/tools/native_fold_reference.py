"""Independent finite FP32 models of four original M3F folds/transforms.

Original CODE and Pascal caller/preparation are the semantic sources. Rational
IEEE primitives are imported from the frozen affine reference, never from a
translated body. No GPU result is read; this is not x87 or scene qualification.
"""
from __future__ import annotations
from fractions import Fraction
import hashlib
import itertools
import json
from pathlib import Path
import re
import struct

import native_formula_reference as affine
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
SOURCE_PINS = {'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/M3Formulas/_AbsY.m3f': '601fefe3de9464c3abed99c12cf79fe186dbf0b5550e020326b193dc87c0607e',
 'mb3d/M3Formulas/_FlipXYc.m3f': 'ab58d5c2bdaf8727a24c2c053d96dcf48e083d032ae1ae56f35fe0e3f84264c9',
 'mb3d/M3Formulas/_FoldingOct.m3f': '0401eee2876869ad35425cc92668d10e29cdfa7682d863a4a77c748ee8b1a332',
 'mb3d/M3Formulas/_recFold.m3f': 'fcc226d257abda598a9f9ae170762cc7251da1e2c4b581138db672a757a9b1a5',
 'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd'}
CODE_AUDIT = {'_AbsY': {'bytes': 33,
           'key_offsets': [{'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x000f'},
                           {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0011'},
                           {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x0016'},
                           {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x0018'},
                           {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x001a'}],
           'sha256': 'd0ae091888a50e3775517d18499e929e63309304fab13cd88013bd2ac460f968'},
 '_FlipXYc': {'bytes': 43,
              'key_offsets': [{'bytes': '837ff400',
                               'instruction': 'cmp dword ptr [edi - 0xc], 0',
                               'offset': '0x000b'},
                              {'bytes': 'dd5e18',
                               'instruction': 'fstp qword ptr [esi + 0x18]',
                               'offset': '0x0017'},
                              {'bytes': 'dd5e20',
                               'instruction': 'fstp qword ptr [esi + 0x20]',
                               'offset': '0x001a'},
                              {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x0021'},
                              {'bytes': 'dd18', 'instruction': 'fstp qword ptr [eax]', 'offset': '0x0023'}],
              'sha256': 'a3c2b51589ae7c1bae208152c1a67c85c805046961ede31919c927d235002868'},
 '_FoldingOct': {'bytes': 71,
                 'key_offsets': [{'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x000c'},
                                 {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0010'},
                                 {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0014'},
                                 {'bytes': 'd8d1', 'instruction': 'fcom st(1)', 'offset': '0x0016'},
                                 {'bytes': '80e441', 'instruction': 'and ah, 0x41', 'offset': '0x001a'},
                                 {'bytes': 'd8d2', 'instruction': 'fcom st(2)', 'offset': '0x0021'},
                                 {'bytes': '80e441', 'instruction': 'and ah, 0x41', 'offset': '0x0025'},
                                 {'bytes': 'd8d2', 'instruction': 'fcom st(2)', 'offset': '0x002e'},
                                 {'bytes': '80e441', 'instruction': 'and ah, 0x41', 'offset': '0x0032'},
                                 {'bytes': 'dd1a', 'instruction': 'fstp qword ptr [edx]', 'offset': '0x0039'},
                                 {'bytes': 'dd1f', 'instruction': 'fstp qword ptr [edi]', 'offset': '0x003b'},
                                 {'bytes': 'dd19',
                                  'instruction': 'fstp qword ptr [ecx]',
                                  'offset': '0x003d'}],
                 'sha256': '726bc0d7c863da8b7d4afa632409963c82d0070e065a58f25711b6a6413e7437'},
 '_recFold': {'bytes': 154,
              'key_offsets': [{'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0011'},
                              {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0017'},
                              {'bytes': 'd8c2', 'instruction': 'fadd st(2)', 'offset': '0x001b'},
                              {'bytes': 'd8e3', 'instruction': 'fsub st(3)', 'offset': '0x001f'},
                              {'bytes': 'dc67e8',
                               'instruction': 'fsub qword ptr [edi - 0x18]',
                               'offset': '0x0021'},
                              {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x0024'},
                              {'bytes': 'dc67e0',
                               'instruction': 'fsub qword ptr [edi - 0x20]',
                               'offset': '0x0027'},
                              {'bytes': 'dd19', 'instruction': 'fstp qword ptr [ecx]', 'offset': '0x002a'},
                              {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x002e'},
                              {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x0030'},
                              {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0035'},
                              {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x0037'},
                              {'bytes': 'd8ea', 'instruction': 'fsubr st(2)', 'offset': '0x003f'},
                              {'bytes': 'd8c3', 'instruction': 'fadd st(3)', 'offset': '0x0043'},
                              {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x0045'},
                              {'bytes': 'dc67d8',
                               'instruction': 'fsub qword ptr [edi - 0x28]',
                               'offset': '0x0047'},
                              {'bytes': 'dc0f', 'instruction': 'fmul qword ptr [edi]', 'offset': '0x004a'},
                              {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x004c'},
                              {'bytes': 'dc67d8',
                               'instruction': 'fsub qword ptr [edi - 0x28]',
                               'offset': '0x004f'},
                              {'bytes': 'dc0f', 'instruction': 'fmul qword ptr [edi]', 'offset': '0x0052'},
                              {'bytes': 'dd19', 'instruction': 'fstp qword ptr [ecx]', 'offset': '0x0054'},
                              {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x0058'},
                              {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x005a'},
                              {'bytes': 'd9e4', 'instruction': 'ftst ', 'offset': '0x0062'},
                              {'bytes': '7619', 'instruction': 'jbe 0x84', 'offset': '0x0069'},
                              {'bytes': '7e13', 'instruction': 'jle 0x84', 'offset': '0x006f'},
                              {'bytes': '3de7030000', 'instruction': 'cmp eax, 0x3e7', 'offset': '0x0071'},
                              {'bytes': '7f0b', 'instruction': 'jg 0x84', 'offset': '0x0077'},
                              {'bytes': 'd8e9', 'instruction': 'fsubr st(1)', 'offset': '0x0079'},
                              {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x007b'},
                              {'bytes': 'd8e9', 'instruction': 'fsubr st(1)', 'offset': '0x007d'},
                              {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x007f'},
                              {'bytes': '7ff5', 'instruction': 'jg 0x79', 'offset': '0x0082'},
                              {'bytes': 'ddd9', 'instruction': 'fstp st(1)', 'offset': '0x0084'},
                              {'bytes': 'dd1b', 'instruction': 'fstp qword ptr [ebx]', 'offset': '0x0086'}],
              'sha256': '23de73f75da4c4793d13d25a9603112e5741db4c08245fe43f1d04534b0bd669'}}
PRIMITIVE_LF_SHA256 = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'
FORMULAS = {
    '_FoldingOct': (2153, -1, 0, 0, 1, 1),
    '_recFold': (3676, -1, 7, 0, 1, 1),
    '_AbsY': (5002, -1, 1, 0, 1, 1),
    '_FlipXYc': (7004, -1, 1, 32, 1, 1),
}
# (PVar byte offset, primitive storage kind); constants are not UI parameters.
LAYOUT = {
    '_FoldingOct': (), '_AbsY': ((-16, 'double'),),
    '_FlipXYc': ((-12, 'int32'),),
    '_recFold': ((-16, 'double'), (-24, 'double'), (-32, 'double'), (-40, 'double'),
                 (-44, 'int32'), (0, 'constant'), (8, 'constant')),
}
CONSTANTS = {0: 0x3fe0000000000000, 8: 0x3fe6a09e667f3bcd}
POINT_LANES = {'_FoldingOct': ('xyz[0]', 'xyz[1]', 'xyz[2]'),
               '_recFold': ('xyz[0]', 'xyz[2]'), '_AbsY': ('xyz[1]',),
               '_FlipXYc': ('xyz[0]', 'xyz[1]', 'j[0]', 'j[1]')}
add, mul, fp32, fraction, word = affine.add, affine.multiply, affine.binary32, affine.fraction, affine.word


def neg(value): return value ^ 0x80000000

def absolute(value): return value & 0x7fffffff

def sub(a, b): return add(a, neg(b))


def fraction64(bits):
    exponent = (bits >> 52) & 2047
    if exponent == 2047: raise ValueError('nonfinite binary64 metadata')
    mantissa = (bits & ((1 << 52)-1)) | ((1 << 52) if exponent else 0)
    shift = exponent-1023-52 if exponent else -1074
    result = Fraction(mantissa << shift) if shift >= 0 else Fraction(mantissa, 1 << -shift)
    return -result if bits >> 63 else result


def narrow64(bits):
    exponent, payload, sign = (bits >> 52) & 2047, bits & ((1 << 52)-1), (bits >> 32) & 0x80000000
    if exponent == 2047:
        # Unconsumed raw aliases may encode a NaN for a valid int32 parameter.
        return sign | 0x7f800000 | ((0x400000 | (payload >> 29)) if payload else 0)
    return fp32(fraction64(bits), bool(sign))


def wide32(bits):
    fraction(bits)  # Only finite authored real parameters belong to this model.
    return struct.unpack('<Q', struct.pack('<d', struct.unpack('<f', struct.pack('<I', bits))[0]))[0]


def _program(name, params, state):
    ident, option, count, traits, flags, driver = FORMULAS[name]
    if len(params) != count: raise ValueError('wrong prepared parameter count')
    p = [0]*2048
    p[:15] = [1, driver, 6, 0, 0, 0, 2, state['max_iterations'], state['julia'],
              0, 0, 16384, 4096, 0, 0]
    p[16], p[18], p[21], p[22] = state['rstop'], fp32(1), fp32(1 << 20), fp32(1)
    p[24:28] = [state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4): p[28+5*i] = fp32(1)
    p[50] = state['hybrid_counts[0]']
    for slot in range(6): p[56+slot*332] = 1
    base = 56
    p[base:base+12] = [1, 1, ident, option & 0xffffffff, count, traits, flags, 0, 0, 1, 0, 0]
    memory = bytearray(512)  # PVar is byte256; unused fixture bytes are zero.
    struct.pack_into('<Q', memory, 248, 0x3fe0000000000000)
    for i, ((offset, kind), value) in enumerate(zip(LAYOUT[name], params)):
        if kind == 'int32':
            if type(value) is not int or not -(1 << 31) <= value < (1 << 31):
                raise ValueError('int32 parameter outside original storage domain')
            struct.pack_into('<i', memory, 256+offset, value)
            original = struct.unpack('<Q', struct.pack('<d', value))[0]
            prepared = fp32(value)
        else:
            original = CONSTANTS[offset] if kind == 'constant' else wide32(value)
            prepared = narrow64(original)
            if value != prepared: raise ValueError('original constant differs')
            struct.pack_into('<Q', memory, 256+offset, original)
        p[base+12+i] = prepared
        p[base+76+2*i:base+78+2*i] = [original & 0xffffffff, original >> 32]
    p[base+204:base+268] = struct.unpack('<64I', memory[:256])
    for i in range(32):
        p[base+268+i] = narrow64(struct.unpack_from('<Q', memory, 256-8*(i+1))[0])
        p[base+300+i] = narrow64(struct.unpack_from('<Q', memory, 256+8*i)[0])
    return p


def _parameters(name, program):
    values = program[68:68+FORMULAS[name][2]]
    for i, (_, kind) in enumerate(LAYOUT[name]):
        if kind == 'int32':
            exact = fraction64(program[56+76+2*i] | program[56+77+2*i] << 32)
            if exact.denominator != 1 or not -(1 << 31) <= exact < (1 << 31):
                raise ValueError('int32 metadata is not an exact signed integer')
            values[i] = int(exact)
    return values


def _point(name, state, params):
    out = dict(state)
    if name == '_FoldingOct':
        # Positive finite binary32 bit order equals magnitude order.
        ordered = sorted((absolute(state[f'xyz[{i}]']) for i in range(3)), reverse=True)
        for i, value in enumerate(ordered): out[f'xyz[{i}]'] = value
    elif name == '_AbsY':
        out['xyz[1]'] = sub(absolute(add(state['xyz[1]'], params[0])), params[0])
    elif name == '_FlipXYc':
        out['xyz[0]'], out['xyz[1]'] = state['xyz[1]'], state['xyz[0]']
        if params[0] != 0: out['j[0]'], out['j[1]'] = state['j[1]'], state['j[0]']
    elif name == '_recFold':
        a, b = absolute(state['xyz[0]']), absolute(state['xyz[2]'])
        u, v = sub(sub(b, a), params[1]), sub(add(b, a), params[2])
        folded = neg(absolute(u))
        difference, total = sub(folded, v), add(v, folded)
        x = mul(sub(neg(total), params[3]), params[5])
        z = mul(sub(difference, params[3]), params[5])
        # Original FTST/FNSTSW/SHR/JBE admits both signed zeros (TOP!=0).
        if fraction(x) >= 0 and 2 <= params[4] <= 1000:
            for _ in range(params[4]-1): x = absolute(sub(params[0], absolute(sub(params[0], x))))
        out['xyz[0]'], out['xyz[2]'] = x, z
    else: raise ValueError('formula outside independent fold model')
    out['current_slot'] = 0
    return out


def _orbit(name, state, params):
    # Original formulas.pas:3697 doHybridPasDE. The finite fixtures use
    # analytic3D, one positive-count slot, no interpolation/smoothing.
    s = dict(state)
    for i in range(3):
        s[f'xyz[{i}]'] = s[f'c[{i}]']
        s[f'j[{i}]'] = s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]'] = fp32(1)
    s['first'] = s['iteration'] = 0
    s['rout'] = s['otrap'] = affine._radius(s, False)
    remaining = count = s['hybrid_counts[0]']
    s['tmp'] = remaining
    for _ in range(s['max_iterations']):
        s['rold'] = s['rout']
        if remaining == 0: remaining = count
        s = _point(name, s, params)
        remaining -= 1
        s['tmp'] = remaining
        s['iteration'] += 1
        s['rout'] = affine._radius(s, False)
        if fraction(s['rout']) < fraction(s['otrap']): s['otrap'] = s['rout']
        if fraction(s['rout']) > fraction(s['rstop']): break
    return s


def evaluate(payload):
    if set(payload) != {'identity', 'kind', 'input_state_bits', 'parameter_bits'}:
        raise ValueError('expected exact semantic input fields')
    ident = payload['identity']
    if not isinstance(ident, list) or len(ident) != 3 or ident[0] != 'external' or ident[2] not in FORMULAS or ident[1] != FORMULAS[ident[2]][0]:
        raise ValueError('wrong ordinary fold identity')
    name = ident[2]
    def decode(value):
        if not isinstance(value, str) or not re.fullmatch(r'0x[0-9a-f]{8}', value):
            raise ValueError('expected canonical hex32 word')
        return int(value, 16)
    if set(payload['input_state_bits']) != set(STATE) or len(payload['parameter_bits']) != 2048:
        raise ValueError('wrong program/state width')
    s = {k: decode(v) for k,v in payload['input_state_bits'].items()}
    p = [decode(v) for v in payload['parameter_bits']]
    for lane in STATE[:33]: fraction(s[lane])
    if payload['kind'] not in ('point', 'orbit') or s['julia'] not in (0, 1) or s['inside'] or s['calc_smooth']:
        raise ValueError('unsupported caller configuration')
    if not 1 <= s['max_iterations'] <= 8 or not 1 <= s['hybrid_counts[0]'] <= 3:
        raise ValueError('bounded caller limits exceeded')
    params = _parameters(name, p)
    if p != _program(name, params, s): raise ValueError('program differs from audited typed ABI')
    if payload['kind'] == 'orbit':
        if s['vary_scale'] != fp32(1) or s['j[3]'] != s['original_j[3]']:
            raise ValueError('3D entry state outside audited domain')
        out = _orbit(name, s, params)
    else: out = _point(name, s, params)
    for lane in STATE[:33]: fraction(out[lane])
    return {k: word(out[k]) for k in STATE}


def dependency_paths(): return [Path(affine.__file__).resolve()]


def primitive_identity(data: bytes) -> str:
    """Accept only line-ending changes; retain actual bytes for event binding."""
    if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != PRIMITIVE_LF_SHA256:
        raise ValueError('frozen independent primitive dependency changed')
    return hashlib.sha256(data).hexdigest()


def manifest():
    sources = {}
    for path, expected in SOURCE_PINS.items():
        data = (ROOT/path).read_bytes()
        if hashlib.sha256(data.replace(b'\r\n', b'\n')).hexdigest() != expected:
            raise ValueError('original semantic source changed: '+path)
        sources[path] = hashlib.sha256(data).hexdigest()
    for name, audit in CODE_AUDIT.items():
        text = (ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code = bytes.fromhex(re.sub(r'\s+', '', re.search(r'\[CODE\](.*?)\[END\]', text, re.S)[1]))
        if len(code) != audit['bytes'] or hashlib.sha256(code).hexdigest() != audit['sha256']:
            raise ValueError('original CODE changed')
        for instruction in audit['key_offsets']:
            start = int(instruction['offset'],16); expected = bytes.fromhex(instruction['bytes'])
            if code[start:start+len(expected)] != expected: raise ValueError('original opcode audit changed')
    dependency = dependency_paths()[0]
    dependency_sha256 = primitive_identity(dependency.read_bytes())
    return {'schema':'mb3d.native-fold-reference.v1', 'semantic_sources':sources,
            'code_audit':CODE_AUDIT, 'abi_sources':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in
                ('src/render/native_metal/formula_abi.h', 'src/core/external_formulas.h')},
            'dependencies':{str(dependency.relative_to(ROOT)):dependency_sha256},
            'arithmetic':'Independent rational binary32 RN-even primitives, explicit sign bits, exact int32 parameters',
            'model_limits':['Four original M3F formulas; finite point states and exact-dyadic Hybrid3D orbits',
                            'Authored real parameters are exact widened binary32 values; original binary64 constants are pinned separately',
                            'One enabled slot0; no smoothing/interpolation/combination or range exits',
                            'No independent distance-value proof, x87 parity or full-scene credit'],
            'model_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def _state():
    s = {k:fp32(Fraction(i+1,32)) for i,k in enumerate(STATE)}
    s.update(iteration=19, first=17, tmp=23, max_iterations=4, julia=0,
             inside=0, calc_smooth=0, current_slot=5)
    for i in range(6): s[f'hybrid_counts[{i}]'] = 1 if i == 0 else 0
    for i,value in enumerate((Fraction(3,8),Fraction(-1,2),Fraction(5,8),Fraction(-3,4))):
        for a,scale in (('xyz',1),('c',1),('j',2),('original_j',-2)): s[f'{a}[{i}]'] = fp32(value*scale)
    s['rstop'], s['vary_scale'], s['j[3]'] = fp32(1<<16), fp32(1), s['original_j[3]']
    return s


def build_cases():
    provenance = manifest(); cases = []
    defaults = {'_FoldingOct':[], '_AbsY':[fp32(1)], '_FlipXYc':[1],
                '_recFold':[fp32(1),fp32(-3),fp32(10),fp32(1),4,narrow64(CONSTANTS[0]),narrow64(CONSTANTS[8])]}
    def emit(name, variant, s, params, kind='point'):
        semantic = {'identity':['external',FORMULAS[name][0],name], 'kind':kind,
                    'input_state_bits':{k:word(s[k]) for k in STATE},
                    'parameter_bits':[word(v) for v in _program(name,params,s)]}
        writes = set(POINT_LANES[name]) | {'current_slot'}; reads = set(POINT_LANES[name])
        if name == '_FlipXYc' and params[0] == 0:
            writes -= {'j[0]','j[1]'}; reads -= {'j[0]','j[1]'}
        if kind == 'orbit':
            writes |= {f'xyz[{i}]' for i in range(4)} | {f'j[{i}]' for i in range(3)} | {'rout','rold','otrap','first','iteration','tmp'}
            reads |= {f'{a}[{i}]' for a in ('c','original_j') for i in range(4)} | {'julia','max_iterations','rstop','hybrid_counts[0]'}
        cases.append({'id':f'{FORMULAS[name][0]}-{variant}', 'identity':semantic['identity'], 'variant':variant,
                      'input':semantic, 'expected_state_bits':evaluate(semantic),
                      'read_lanes':[k for k in STATE if k in reads], 'write_lanes':[k for k in STATE if k in writes],
                      'source_hashes':provenance['semantic_sources']})
    for name in FORMULAS:
        emit(name,'asymmetric',_state(),defaults[name])
        s=_state();s['first'],s['iteration'],s['tmp']=0x80000000,0x7fffffff,0xffffffff
        emit(name,'integer_sentinels',s,defaults[name])
    for i,coords in enumerate(itertools.permutations((fp32(Fraction(3,8)),fp32(Fraction(-5,8)),fp32(Fraction(1,4))))):
        s=_state();s.update({f'xyz[{j}]':v for j,v in enumerate(coords)})
        emit('_FoldingOct',f'permutation_{i}',s,[])
    for variant,coords in [('ties',(fp32(-2),fp32(2),fp32(-1))),('signed_zero',(0x80000000,0,0x80000000)),
                           ('finite_extremes',(0xff7fffff,0x00800000,fp32(1)))]:
        s=_state();s.update({f'xyz[{j}]':v for j,v in enumerate(coords)});emit('_FoldingOct',variant,s,[])
    for variant,y,fix in [('boundary',fp32(-1),fp32(1)),('boundary_below',0xbf800001,fp32(1)),
                          ('boundary_above',0xbf7fffff,fp32(1)),('negative_fix',fp32(Fraction(1,2)),fp32(-1)),
                          ('negative_zero',0x80000000,0),('negative_zero_fix',0x80000000,0x80000000),
                          ('rounding_tie',fp32(1),fp32(Fraction(1,1<<24))),('large_offset',fp32(Fraction(3,4)),fp32(1<<24))]:
        s=_state();s['xyz[1]']=y;emit('_AbsY',variant,s,[fix])
    for n in (0,1,-1,-2147483648,2147483647,16777217):
        s=_state();s['xyz[0]'],s['xyz[1]']=0x80000000,0;s['j[0]'],s['j[1]']=0,0x80000000
        emit('_FlipXYc','integer_'+str(n).replace('-','minus'),s,[n])
    for n in (-2147483648,-1,0,1,2,999,1000,1001,16777217,2147483647):
        p=defaults['_recFold'].copy();p[4]=n
        emit('_recFold','count_'+str(n).replace('-','minus'),_state(),p)
    for variant,n,fold,q in [('zero_skip',1,-1,0),('zero_enter',2,-1,0),('zero_positive_fold',2,1,0),('negative_x_skip',2,-1,4)]:
        s=_state();s['xyz[0]']=s['xyz[2]']=0
        p=[fp32(fold),0,0,fp32(q),n,narrow64(CONSTANTS[0]),narrow64(CONSTANTS[8])]
        emit('_recFold',variant,s,p)
    for name in FORMULAS:
        for variant in ('orbit_mandel','orbit_julia','orbit_escape','orbit_repeat'):
            s=_state();s['julia']=int(variant in ('orbit_julia','orbit_repeat'))
            s['hybrid_counts[0]']=2 if variant=='orbit_repeat' else 1
            if variant=='orbit_escape':s['rstop']=fp32(Fraction(1,16))
            if variant=='orbit_repeat':
                first=dict(s)
                for i in range(3):first[f'xyz[{i}]']=s[f'c[{i}]']
                s['rstop']=affine._radius(_point(name,first,defaults[name]),False)
            emit(name,variant,s,defaults[name],'orbit')
    return cases
