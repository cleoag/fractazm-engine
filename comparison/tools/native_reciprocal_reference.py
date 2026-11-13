"""Independent original-CODE reciprocal/sphere models, finite Native FP32.

Expected arithmetic uses rational RN-even operations, including division and
original square-first order. No translated formula or device result is read.
Only bounded finite point fixtures and exact-radius Hybrid3D orbits qualify;
singularities/range exits are classified separately, never successful cases.
"""
from __future__ import annotations
from fractions import Fraction
import hashlib
from pathlib import Path
import re
import struct
import native_formula_reference as affine
from gpu_pipeline_proofs import STATE

ROOT = Path(__file__).resolve().parents[2]
SOURCE_PINS = {'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/M3Formulas/_SphereFolding1.m3f': 'fe4429a50d2ea74218b73dfe1fa38c2e429c7cb862bffb853ecdc4a4732573ef',
 'mb3d/M3Formulas/_reciprocalX2.m3f': '1ea9243bad2354d669f3cf6c7eedd247102e01140bc8f19b0f74fc763e0e31fd',
 'mb3d/M3Formulas/_reciprocalX3b.m3f': 'a9a679d08cee10f0f6042670de7439e085d98f0c9b94fa2f93b3e3d229afea57',
 'mb3d/M3Formulas/_reciprocalY3b.m3f': '49dbf0abacdb96b2edfe978f2e775e233571d337df96c129aec2eb2b8537c110',
 'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd'}
CODE_AUDIT = {'_SphereFolding1': {'bytes': 75,
                     'key_offsets': [{'bytes': 'dc57f0',
                                      'instruction': 'fcom qword ptr [edi - 0x10]',
                                      'offset': '0x0011'},
                                     {'bytes': 'dc6fe8',
                                      'instruction': 'fsubr qword ptr [edi - 0x18]',
                                      'offset': '0x001a'},
                                     {'bytes': 'dc57e0',
                                      'instruction': 'fcom qword ptr [edi - 0x20]',
                                      'offset': '0x001f'},
                                     {'bytes': 'dc6fd8',
                                      'instruction': 'fsubr qword ptr [edi - 0x28]',
                                      'offset': '0x0028'},
                                     {'bytes': 'dc7638',
                                      'instruction': 'fdiv qword ptr [esi + 0x38]',
                                      'offset': '0x002b'},
                                     {'bytes': 'd8cb', 'instruction': 'fmul st(3)', 'offset': '0x0034'},
                                     {'bytes': 'dd1b',
                                      'instruction': 'fstp qword ptr [ebx]',
                                      'offset': '0x0036'},
                                     {'bytes': 'd8ca', 'instruction': 'fmul st(2)', 'offset': '0x0038'},
                                     {'bytes': 'dd1a',
                                      'instruction': 'fstp qword ptr [edx]',
                                      'offset': '0x003a'},
                                     {'bytes': 'd8c9', 'instruction': 'fmul st(1)', 'offset': '0x003c'},
                                     {'bytes': 'dd19',
                                      'instruction': 'fstp qword ptr [ecx]',
                                      'offset': '0x0040'},
                                     {'bytes': 'ddd8', 'instruction': 'fstp st(0)', 'offset': '0x0042'}],
                     'sha256': '9065dbfd362f15fdb14bb2f0db3b17c85aaa3abea07785b1490bb8738875e3b6'},
 '_reciprocalX2': {'bytes': 39,
                   'key_offsets': [{'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0010'},
                                   {'bytes': 'dc46f0',
                                    'instruction': 'fadd qword ptr [esi - 0x10]',
                                    'offset': '0x0012'},
                                   {'bytes': '80e280', 'instruction': 'and dl, 0x80', 'offset': '0x0015'},
                                   {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x001a'},
                                   {'bytes': 'dd18',
                                    'instruction': 'fstp qword ptr [eax]',
                                    'offset': '0x001c'},
                                   {'bytes': '305007',
                                    'instruction': 'xor byte ptr [eax + 7], dl',
                                    'offset': '0x001e'}],
                   'sha256': '07d6a03811a01b2ac13061e08c41fdea8b37858ec47c9e04129a8dd72bff6d16'},
 '_reciprocalX3b': {'bytes': 70,
                    'key_offsets': [{'bytes': 'dc4ed0',
                                     'instruction': 'fmul qword ptr [esi - 0x30]',
                                     'offset': '0x0010'},
                                    {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0013'},
                                    {'bytes': 'dc46f0',
                                     'instruction': 'fadd qword ptr [esi - 0x10]',
                                     'offset': '0x0015'},
                                    {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x001a'},
                                    {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x001c'},
                                    {'bytes': 'dc46e8',
                                     'instruction': 'fadd qword ptr [esi - 0x18]',
                                     'offset': '0x001e'},
                                    {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x0023'},
                                    {'bytes': 'dc4ec8',
                                     'instruction': 'fmul qword ptr [esi - 0x38]',
                                     'offset': '0x0025'},
                                    {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0028'},
                                    {'bytes': 'dc46e0',
                                     'instruction': 'fadd qword ptr [esi - 0x20]',
                                     'offset': '0x002a'},
                                    {'bytes': '80e280', 'instruction': 'and dl, 0x80', 'offset': '0x002d'},
                                    {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x0032'},
                                    {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x0034'},
                                    {'bytes': 'dc46d8',
                                     'instruction': 'fadd qword ptr [esi - 0x28]',
                                     'offset': '0x0036'},
                                    {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x0039'},
                                    {'bytes': 'dd18',
                                     'instruction': 'fstp qword ptr [eax]',
                                     'offset': '0x003b'},
                                    {'bytes': '305007',
                                     'instruction': 'xor byte ptr [eax + 7], dl',
                                     'offset': '0x003d'}],
                    'sha256': '0ca59792b709418c9dbf979b919ae9aac70f09391ce8cfce0fc5e4b150f199aa'},
 '_reciprocalY3b': {'bytes': 70,
                    'key_offsets': [{'bytes': 'dc4ed0',
                                     'instruction': 'fmul qword ptr [esi - 0x30]',
                                     'offset': '0x0010'},
                                    {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0013'},
                                    {'bytes': 'dc46f0',
                                     'instruction': 'fadd qword ptr [esi - 0x10]',
                                     'offset': '0x0015'},
                                    {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x001a'},
                                    {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x001c'},
                                    {'bytes': 'dc46e8',
                                     'instruction': 'fadd qword ptr [esi - 0x18]',
                                     'offset': '0x001e'},
                                    {'bytes': 'd8c8', 'instruction': 'fmul st(0)', 'offset': '0x0023'},
                                    {'bytes': 'dc4ec8',
                                     'instruction': 'fmul qword ptr [esi - 0x38]',
                                     'offset': '0x0025'},
                                    {'bytes': 'd9e1', 'instruction': 'fabs ', 'offset': '0x0028'},
                                    {'bytes': 'dc46e0',
                                     'instruction': 'fadd qword ptr [esi - 0x20]',
                                     'offset': '0x002a'},
                                    {'bytes': '80e180', 'instruction': 'and cl, 0x80', 'offset': '0x002d'},
                                    {'bytes': 'def1', 'instruction': 'fdivrp st(1)', 'offset': '0x0032'},
                                    {'bytes': 'd9e0', 'instruction': 'fchs ', 'offset': '0x0034'},
                                    {'bytes': 'dc46d8',
                                     'instruction': 'fadd qword ptr [esi - 0x28]',
                                     'offset': '0x0036'},
                                    {'bytes': 'dec1', 'instruction': 'faddp st(1)', 'offset': '0x0039'},
                                    {'bytes': 'dd1a',
                                     'instruction': 'fstp qword ptr [edx]',
                                     'offset': '0x003b'},
                                    {'bytes': '304a07',
                                     'instruction': 'xor byte ptr [edx + 7], cl',
                                     'offset': '0x003d'}],
                    'sha256': '8a76d1f9923927129cfac78313c341519ca51121f31d63c3872c58f0cb8f5f20'}}
PRIMITIVE_LF_SHA256 = 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'
FORMULAS = {'_SphereFolding1':(6030,-1,4,256,1,1), '_reciprocalX2':(7007,-1,1,0,1,1),
            '_reciprocalX3b':(4812,-1,6,0,1,1), '_reciprocalY3b':(5823,-1,6,0,1,1)}
fp32, fraction, word = affine.binary32, affine.fraction, affine.word
ONE = 0x3f800000
DRECI_FLOOR64 = 0x37a16c262777579c  # Original TypeDefinitions.pas Double d1em40.


class DomainError(ValueError):
    def __init__(self, classification, detail):
        self.classification = classification
        super().__init__(classification+': '+detail)


def finite(bits, classification='FP32_RANGE'):
    if bits & 0x7f800000 == 0x7f800000:
        raise DomainError(classification,'nonfinite binary32 operand/result')
    return bits


def add(a,b): return finite(affine.add(finite(a),finite(b)))
def mul(a,b): return finite(affine.multiply(finite(a),finite(b)))
def neg(a): return a ^ 0x80000000
def absolute(a): return a & 0x7fffffff
def sub(a,b): return add(a,neg(b))


def divide(a,b):
    finite(a);finite(b)
    denominator=fraction(b)
    if denominator == 0: raise DomainError('SINGULAR_DIVISION','zero denominator')
    return finite(fp32(fraction(a)/denominator,bool((a^b)>>31)))


def fraction64(bits):
    exponent=(bits>>52)&2047
    if exponent==2047:raise DomainError('PARAMETER_RANGE','nonfinite binary64 metadata')
    m=(bits&((1<<52)-1))|((1<<52) if exponent else 0)
    shift=exponent-1075 if exponent else -1074
    value=Fraction(m<<shift) if shift>=0 else Fraction(m,1<<-shift)
    return -value if bits>>63 else value


def binary64(value, negative_zero=False):
    """Independent rational RN-even binary64 for Pascal's prepared doubles."""
    value=Fraction(value)
    if not value:return (1<<63) if negative_zero else 0
    sign=(1<<63) if value<0 else 0;value=abs(value);n,d=value.numerator,value.denominator
    exponent=n.bit_length()-d.bit_length()
    if (n<d<<exponent) if exponent>=0 else (n<<-exponent<d):exponent-=1
    shift=1074 if exponent < -1022 else 52-exponent
    significand=affine._nearest_even(n<<shift,d) if shift>=0 else affine._nearest_even(n,d<<-shift)
    if exponent < -1022:return sign|significand
    if significand==1<<53:significand>>=1;exponent+=1
    if exponent>1023:raise DomainError('PARAMETER_RANGE','binary64 preparation overflow')
    return sign|((exponent+1023)<<52)|(significand&((1<<52)-1))


def narrow64(bits):return finite(fp32(fraction64(bits),bool(bits>>63)),'PARAMETER_RANGE')
def wide32(bits):return binary64(fraction(finite(bits,'NONFINITE_INPUT')),bool(bits>>31))


def prepare(name, authored):
    """Original .Folding/.DReci2 expansions; not arbitrary prepared values.

    Authored reals are exact widened binary32 values. Reciprocal preparation
    rounds to binary64 before narrowing to FP32; integer/raw bit aliases are
    serialized separately from the arithmetic input vector.
    """
    expected=1 if name in ('_SphereFolding1','_reciprocalX2') else 4
    if len(authored)!=expected:raise ValueError('wrong authored parameter count')
    original=[wide32(v) for v in authored]
    if name=='_SphereFolding1':
        radius=original[0];twice=binary64(fraction64(radius)*2,bool(radius>>63))
        original=[radius,twice,radius^(1<<63),twice^(1<<63)]
    elif name in ('_reciprocalX3b','_reciprocalY3b'):
        l1,l2,m1,m2=original
        def reciprocal(limiter):
            return binary64(1/max(fraction64(DRECI_FLOOR64),fraction64(limiter)))
        original=[l1,reciprocal(l1),l2,reciprocal(l2),m1,m2]
    elif name!='_reciprocalX2':raise ValueError('unsupported reciprocal formula')
    return original,[narrow64(v) for v in original]


def _program(name, authored, state):
    ident,option,count,traits,flags,driver=FORMULAS[name]
    original,params=prepare(name,authored)
    p=[0]*2048
    p[:15]=[1,driver,6,0,0,0,2,state['max_iterations'],state['julia'],0,0,16384,4096,0,0]
    p[16],p[18],p[21],p[22]=state['rstop'],ONE,fp32(1<<20),ONE
    p[24:28]=[state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4):p[28+5*i]=ONE
    p[50]=state['hybrid_counts[0]']
    for slot in range(6):p[56+slot*332]=1
    base=56;p[base:base+12]=[1,1,ident,option&0xffffffff,count,traits,flags,0,0,1,0,0]
    memory=bytearray(512);struct.pack_into('<Q',memory,248,0x3fe0000000000000)
    for i,(wide,bits) in enumerate(zip(original,params)):
        p[base+12+i]=bits;p[base+76+2*i:base+78+2*i]=[wide&0xffffffff,wide>>32]
        struct.pack_into('<Q',memory,240-8*i,wide)
    p[base+204:base+268]=struct.unpack('<64I',memory[:256])
    for i in range(32):
        p[base+268+i]=narrow64(struct.unpack_from('<Q',memory,248-8*i)[0])
        p[base+300+i]=narrow64(struct.unpack_from('<Q',memory,256+8*i)[0])
    return p


def _point(name,state,params):
    out=dict(state)
    if name=='_SphereFolding1':
        radius=state['rout']
        if fraction(radius)>=fraction(params[0]):numerator=sub(params[1],radius)
        elif fraction(radius)<fraction(params[2]):numerator=sub(params[3],radius)
        else:numerator=None
        if numerator is not None:
            scale=divide(numerator,radius)
            for i in range(3):out[f'xyz[{i}]']=mul(state[f'xyz[{i}]'],scale)
    elif name=='_reciprocalX2':
        value=state['xyz[0]'];denominator=add(absolute(value),params[0])
        out['xyz[0]']=divide(ONE,denominator)^(value&0x80000000)
    elif name in ('_reciprocalX3b','_reciprocalY3b'):
        lane='xyz[1]' if name=='_reciprocalY3b' else 'xyz[0]';value=state[lane]
        d1=add(absolute(mul(value,params[4])),params[0])
        t1=sub(params[1],divide(ONE,d1))
        # Original CODE squares the coordinate before multiplying bymul2.
        # Do not adopt the translated body's opposite association on mismatch.
        square=mul(value,value)
        d2=add(absolute(mul(square,params[5])),params[2])
        t2=sub(params[3],divide(ONE,d2))
        out[lane]=add(t1,t2)^(value&0x80000000)
    else:raise ValueError('unsupported reciprocal formula')
    out['current_slot']=0
    return out


def _orbit(name,state,params):
    s=dict(state)
    for i in range(3):
        s[f'xyz[{i}]']=s[f'c[{i}]']
        s[f'j[{i}]']=s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]']=ONE;s['first']=s['iteration']=0
    s['rout']=s['otrap']=affine._radius(s,False)
    remaining=count=s['hybrid_counts[0]'];s['tmp']=remaining
    for _ in range(s['max_iterations']):
        s['rold']=s['rout']
        if remaining==0:remaining=count
        s=_point(name,s,params);remaining-=1;s['tmp']=remaining;s['iteration']+=1
        s['rout']=affine._radius(s,False)
        if fraction(s['rout'])<fraction(s['otrap']):s['otrap']=s['rout']
        if fraction(s['rout'])>fraction(s['rstop']):break
    return s


def evaluate(payload):
    if set(payload)!={'identity','kind','input_state_bits','parameter_bits'}:raise ValueError('wrong semantic fields')
    identity=payload['identity']
    if not isinstance(identity,list) or len(identity)!=3 or identity[0]!='external' or identity[2] not in FORMULAS or identity[1]!=FORMULAS[identity[2]][0]:
        raise ValueError('wrong formula identity')
    name=identity[2]
    def decode(value):
        if not isinstance(value,str) or not re.fullmatch(r'0x[0-9a-f]{8}',value):raise ValueError('expected hex32')
        return int(value,16)
    if set(payload['input_state_bits'])!=set(STATE) or len(payload['parameter_bits'])!=2048:raise ValueError('wrong ABI width')
    s={k:decode(v) for k,v in payload['input_state_bits'].items()};p=[decode(v) for v in payload['parameter_bits']]
    for lane in STATE[:33]:finite(s[lane],'NONFINITE_INPUT')
    if payload['kind'] not in ('point','orbit') or s['julia'] not in (0,1) or s['inside'] or s['calc_smooth']:
        raise ValueError('unsupported caller')
    if not 1<=s['max_iterations']<=8 or not 1<=s['hybrid_counts[0]']<=3:raise ValueError('caller bounds exceeded')
    authored=[p[68]] if name in ('_SphereFolding1','_reciprocalX2') else [p[68+i] for i in (0,2,4,5)]
    if p!=_program(name,authored,s):raise ValueError('prepared parameter relationships or ABI differ')
    _,params=prepare(name,authored)
    if payload['kind']=='orbit':
        if s['vary_scale']!=ONE or s['j[3]']!=s['original_j[3]']:raise ValueError('unsupported 3D entry state')
        out=_orbit(name,s,params)
    else:out=_point(name,s,params)
    for lane in STATE[:33]:finite(out[lane])
    return {k:word(out[k]) for k in STATE}


def dependency_paths():return [Path(affine.__file__).resolve()]


def primitive_identity(data):
    if hashlib.sha256(data.replace(b'\r\n',b'\n')).hexdigest()!=PRIMITIVE_LF_SHA256:
        raise ValueError('frozen primitive dependency changed')
    return hashlib.sha256(data).hexdigest()


def manifest():
    sources={}
    for path,pin in SOURCE_PINS.items():
        data=(ROOT/path).read_bytes()
        if hashlib.sha256(data.replace(b'\r\n',b'\n')).hexdigest()!=pin:raise ValueError('original semantic source changed: '+path)
        sources[path]=hashlib.sha256(data).hexdigest()
    for name,audit in CODE_AUDIT.items():
        text=(ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code=bytes.fromhex(re.sub(r'\s+','',re.search(r'\[CODE\](.*?)\[END\]',text,re.S)[1]))
        if len(code)!=audit['bytes'] or hashlib.sha256(code).hexdigest()!=audit['sha256']:raise ValueError('original CODE changed')
        for instruction in audit['key_offsets']:
            start=int(instruction['offset'],16);expected=bytes.fromhex(instruction['bytes'])
            if code[start:start+len(expected)]!=expected:raise ValueError('opcode audit changed')
    dep=dependency_paths()[0]
    return {'schema':'mb3d.native-reciprocal-reference.v1','semantic_sources':sources,'code_audit':CODE_AUDIT,
            'dependencies':{str(dep.relative_to(ROOT)):primitive_identity(dep.read_bytes())},
            'abi_sources':{p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in
                           ('src/render/native_metal/formula_abi.h','src/core/external_formulas.h')},
            'arithmetic':'Independent RN-even rational FP32 division/add/multiply; original square-first sequence and sign XOR',
            'model_limits':['Four original formulas; finite point domain and exact-radius single-slot analytic 3D orbits',
                            'Authored reals exactly widened binary32; source-defined expansions round to binary64 then FP32',
                            'Nonfinite prepared parameters, singular denominators and intermediate range exits are rejected explicitly',
                            'Non-dyadic division and association controls are fixed before GPU; observed mismatches stay OPEN',
                            'No distance-value proof, x87 parity, full-scene or arbitrary-input qualification'],
            'model_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def _state():
    s={k:fp32(Fraction(i+1,32)) for i,k in enumerate(STATE)}
    s.update(iteration=19,first=17,tmp=23,max_iterations=4,julia=0,inside=0,calc_smooth=0,current_slot=5)
    for i in range(6):s[f'hybrid_counts[{i}]']=1 if i==0 else 0
    for i,value in enumerate((Fraction(3,8),Fraction(-1,2),Fraction(5,8),Fraction(-3,4))):
        for a,scale in (('xyz',1),('c',1),('j',2),('original_j',-2)):s[f'{a}[{i}]']=fp32(value*scale)
    s['rstop'],s['vary_scale'],s['j[3]']=fp32(1<<16),ONE,s['original_j[3]']
    return s


def build_cases():
    sources=manifest()['semantic_sources'];cases=[]
    defaults={'_SphereFolding1':[ONE],'_reciprocalX2':[fp32(Fraction(3,2))],
              '_reciprocalX3b':[ONE,ONE,ONE,ONE],'_reciprocalY3b':[ONE,ONE,ONE,ONE]}
    def emit(name,variant,state,authored,kind='point'):
        semantic={'identity':['external',FORMULAS[name][0],name],'kind':kind,
                  'input_state_bits':{k:word(state[k]) for k in STATE},
                  'parameter_bits':[word(v) for v in _program(name,authored,state)]}
        touched={f'xyz[{i}]' for i in range(3)} if name=='_SphereFolding1' else {'xyz[1]' if name=='_reciprocalY3b' else 'xyz[0]'}
        reads=set(touched);writes=touched|{'current_slot'}
        if name=='_SphereFolding1':reads.add('rout')
        if kind=='orbit':
            writes|={f'xyz[{i}]' for i in range(4)}|{f'j[{i}]' for i in range(3)}|{'rout','rold','otrap','first','iteration','tmp'}
            reads|={f'{a}[{i}]' for a in ('c','original_j') for i in range(4)}|{'julia','max_iterations','rstop','hybrid_counts[0]'}
        cases.append({'id':f'{FORMULAS[name][0]}-{variant}','identity':semantic['identity'],'variant':variant,'input':semantic,
                      'expected_state_bits':evaluate(semantic),'read_lanes':[k for k in STATE if k in reads],
                      'write_lanes':[k for k in STATE if k in writes],'source_hashes':sources})
    for name in FORMULAS:
        s=_state()
        if name=='_SphereFolding1':s['rout']=fp32(4)
        emit(name,'asymmetric',s,defaults[name])
        s['first'],s['iteration'],s['tmp']=0x80000000,0x7fffffff,0xffffffff
        emit(name,'integer_sentinels',s,defaults[name])
    for variant,radius,R in [('high_equal',ONE,ONE),('high_below',0x3f7fffff,ONE),('high_above',0x3f800001,ONE),
                             ('low_equal',0xbf800000,ONE),('low_below',0xbf800001,ONE),('low_above',0xbf7fffff,ONE),
                             ('zero',0,ONE),('negative_zero',0x80000000,ONE),('negative_R_high',fp32(Fraction(1,2)),fp32(-1)),
                             ('negative_R_low',fp32(-2),fp32(-1)),('zero_R_finite',fp32(2),0)]:
        s=_state();s['rout']=radius;emit('_SphereFolding1',variant,s,[R])
    s=_state();s['rout']=fp32(4);s['xyz[0]'],s['xyz[1]']=0x80000000,0
    emit('_SphereFolding1','signed_coordinates',s,[ONE])
    for variant,x,L in [('zero_positive',0,fp32(2)),('zero_negative',0x80000000,fp32(2)),
                        ('negative_denominator_zero',0,fp32(-2)),('negative_denominator_negative_zero',0x80000000,fp32(-2)),
                        ('negative_denominator_positive',ONE,fp32(-2)),('negative_denominator_negative',fp32(-1),fp32(-2)),
                        ('pole_below',0x3f7fffff,fp32(-1)),('pole_above',0x3f800001,fp32(-1)),
                        ('non_dyadic_division',fp32(3),0)]:
        s=_state();s['xyz[0]']=x;emit('_reciprocalX2',variant,s,[L])
    for name in ('_reciprocalX3b','_reciprocalY3b'):
        lane='xyz[1]' if name=='_reciprocalY3b' else 'xyz[0]'
        for variant,value,authored in [('zero',0,defaults[name]),('negative_zero',0x80000000,defaults[name]),
                                      ('negative_fixed',fp32(-1),defaults[name]),
                                      ('unequal_limiters',fp32(2),[fp32(2),fp32(4),ONE,fp32(-1)]),
                                      ('negative_unequal_limiters',fp32(-2),[fp32(2),fp32(4),fp32(-1),ONE]),
                                      ('first_term_only',ONE,[ONE,ONE,ONE,0]),('second_term_only',ONE,[ONE,ONE,0,ONE]),
                                      ('prepared_non_dyadic',ONE,[fp32(3),fp32(5),ONE,ONE])]:
            s=_state();s[lane]=value;emit(name,variant,s,authored)
        # Predetermined operation-order controls; never filtered by device results.
        for numerator in (3,13):
            for limiter in (Fraction(1,2),1,2):
                s=_state();s[lane]=fp32(Fraction(numerator,10))
                emit(name,f'square_order_{numerator}_{str(limiter).replace("/","over")}',s,
                     [ONE,fp32(limiter),0,fp32(Fraction(7,10))])
    for name in FORMULAS:
        for variant in ('orbit_mandel','orbit_julia','orbit_escape','orbit_repeat'):
            s=_state();s['julia']=int(variant in ('orbit_julia','orbit_repeat'));s['hybrid_counts[0]']=2 if variant=='orbit_repeat' else 1
            coords=(1,0,0) if name=='_SphereFolding1' else ((Fraction(1,4),-1,Fraction(1,2)) if name=='_reciprocalY3b' else
                    (Fraction(1,2) if name=='_reciprocalX2' else 1,Fraction(1,4),Fraction(1,2)))
            for i,v in enumerate(coords):s[f'c[{i}]']=fp32(v)
            if variant=='orbit_escape':s['rstop']=fp32(Fraction(1,16))
            if variant=='orbit_repeat':s['rstop']=affine._radius({**s,**{f'xyz[{i}]':s[f'c[{i}]'] for i in range(3)}},False)
            emit(name,variant,s,defaults[name],'orbit')
    s=_state();s['c[0]'],s['c[1]'],s['c[2]']=ONE,ONE,0
    emit('_SphereFolding1','orbit_collapse_to_zero',s,[ONE],'orbit')
    s=_state();s['c[0]'],s['c[1]'],s['c[2]']=fp32(-2),fp32(Fraction(1,2)),fp32(Fraction(1,4))
    emit('_reciprocalX2','orbit_two_cycle',s,[0],'orbit')
    return cases
