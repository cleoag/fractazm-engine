"""Original CODE/Pascal RN32 references for three group04 3D formulas.

Finite bounded point/single-slot Hybrid3D state only. Integer Power is the
original raw int32, never a float cast. RN32 sqrt/division targets are derived
from exact rationals; no translated/native implementation supplies an oracle.
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
import native_group03_julia_reference as julia
from gpu_pipeline_proofs import STATE
ROOT = Path(__file__).resolve().parents[2]
CALLER_PROFILE = {'path': 'comparison/results/native-formula-group04-20261101/caller-profile.json',
                  'sha256': '52a02f1b539b022d9975baf0ac027e3463d5441aa755c4bee143506aa1e1ed6e'}
ScopeError = tail.ScopeError
fp32, fraction, word = affine.binary32, affine.fraction, affine.word
add, sub, mul, neg, divide = tail.add, tail.sub, tail.mul, tail.neg, tail.divide
ONE, HALF, NEG_ZERO = 0x3f800000, 0x3f000000, 0x80000000
FORMULAS = {'MsltoeSym2': (7071, 0, 1, 0, 1, 0),
            'Benesi3Pow2': (7085, 0, 0, 0, 1, 0),
            'KaliLinCombSSE2sm': (10352, 0, 3, 4, 1, 0)}
# PVar[-8] is always initialized to 0.5 by CustomFormulas.pas. It is an
# additional runtime parameter for Kali, not an authored SIpower declaration.
LAYOUT = {'MsltoeSym2': ((-16, 'double'),), 'Benesi3Pow2': (),
          'KaliLinCombSSE2sm': ((-12, 'int32'), (0, 'constant'), (-8, 'constant'))}
SOURCE_PINS = {'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
 'mb3d/DivUtils.pas': '879221cb1d6abb371452f1f04afcf5315af238317dcb31b6359a145cb303da96',
 'mb3d/M3Formulas/MsltoeSym2.m3f': '8a331bec7106aeb2f3cd3f20b8a1e625e7f18bcdff2c10e99f00879b5c591a39',
 'mb3d/M3Formulas/Benesi3Pow2.m3f': 'e107e43db926afa2d72e0794c8b390e940fa588daff011493b6c0b6504941d4f',
 'mb3d/M3Formulas/KaliLinCombSSE2sm.m3f': 'c39edfdfbd8dc3a0dd79c8354c27801403767d2468a0f364b24246a776e980a4'}
ABI_PINS = {'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a',
 'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6'}
DEPENDENCY_PINS = {'native_formula_reference.py': 'a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e',
 'native_matrix_reference.py': '02da663186287cf810d0ba976af25ab32570b5235623d4cba4acd2dd4a941811',
 'native_group02_tail_reference.py': 'a8247e6156e22f8fcbb52f2e3fa7876f9c8683c8802ea9f435195b7ef9ae6cb6',
 'native_group03_julia_reference.py': '8b5088f5d93157fc99fc1c62db611608949eac9f239c320461e2ff47f3166956'}
CODE_AUDIT = {'MsltoeSym2': {'sha256': '1ae7cf5d8a392a5e6528c414dba4191fd9d43267d42000fc2229fd31a91cc094',
                'bytes': 110},
 'Benesi3Pow2': {'sha256': '959683f6444bbdf7f4384d9c7a10ef981546577b3e7ee59fa934d496cbdf0c94',
                 'bytes': 101},
 'KaliLinCombSSE2sm': {'sha256': '4cfb307aa18a89d75f0fe800d3283cc2bb9e80c30212dfe010017c74d8563481',
                       'bytes': 323}}


def parsed_kali_constant():
    """Fixed original loader case, not a new permissive number parser.

    LoadNextStr only trims. StrLastWord strips '=' and spaces, leaving the
    semicolon. StrToFloatParsePhiTry delegates that token to TryStrToFloat;
    its failure path explicitly assigns zero. The text is NOT repaired.
    """
    text = (ROOT/'mb3d/M3Formulas/KaliLinCombSSE2sm.m3f').read_text()
    token = text.split('[CONSTANTS]',1)[1].split('[CODE]',1)[0].strip()
    if token != 'Double = 1e-30;': raise ScopeError('original constant spelling changed')
    return 0


def _point(name, state, params):
    s = dict(state); x,y,z = [s[f'xyz[{i}]'] for i in range(3)]
    jx,jy,jz = [s[f'j[{i}]'] for i in range(3)]
    if name == 'MsltoeSym2':
        # Original FCOMPP/AND41/JE swaps on abs(y)<=abs(z), including ties.
        if abs(fraction(y)) <= abs(fraction(z)): y,z = z,y
        # SHR AH leaves C0 in carry; equality takes FCHS as well.
        if fraction(y) >= fraction(z): x = neg(x)
        # 0x33 reads caller Rout at PIteration+56, not a recomputed radius.
        zr = sub(ONE, divide(mul(z,z),s['rout']))
        x2,y2 = mul(x,x),mul(y,y)
        nx = add(mul(sub(x2,y2),zr),jx)
        rho = julia.sqrt_bits(add(x2,y2))
        # Original stores Z without Jz; Y applies authored multiplier + Jy.
        nz = add(mul(z,rho),mul(z,rho))
        xy = mul(y,mul(x,zr))
        ny = add(mul(add(xy,xy),params[0]),jy)
        s['xyz[0]'],s['xyz[1]'],s['xyz[2]'] = nx,ny,nz
    elif name == 'Benesi3Pow2':
        z2,y2,x2 = mul(z,z),mul(y,y),mul(x,x)
        r = add(z2,y2); rho = julia.sqrt_bits(r)
        nx = sub(x2,r)
        # Original FCOM rho,x then the HIGH BYTE of stored binary64 Jx.
        # Thus negative zero Jx takes the negative-sign branch too.
        if fraction(rho) < fraction(x) and not (jx >> 31): nx = neg(nx)
        s['xyz[0]'] = add(nx,jx)
        scale = mul(divide(fp32(-2),rho),x & 0x7fffffff)
        s['xyz[1]'] = add(mul(sub(y2,z2),scale),jy)
        yz = mul(z,mul(y,scale))
        s['xyz[2]'] = add(add(yz,yz),jz)
    elif name == 'KaliLinCombSSE2sm':
        # ESI is PIteration+128: +58/+70 address +216/+224/+240/+248.
        # Trait4 maps these to Dfree1/Dfree2/Deriv2/Deriv3, not SSE transport.
        if not s['first']:
            s['first'] = 1
            imag0,real0 = [x,y],[z,z]
            s['xyz[2]'] = 0  # original MOVLPD zero to PIteration-16
        else:
            imag0 = [s['dfree1'],s['dfree2']]
            real0 = [s['derivative[1]'],s['derivative[2]']]
        def square(real,imag):
            # CALL44: imaginary is multiplied before old real is overwritten.
            ri = [mul(i,r) for r,i in zip(real,imag)]
            return ([sub(mul(r,r),mul(i,i)) for r,i in zip(real,imag)],
                    [add(v,v) for v in ri])
        def times_base(real,imag):
            return ([sub(mul(r,b),mul(i,a)) for r,i,a,b in zip(real,imag,imag0,real0)],
                    [add(mul(i,b),mul(r,a)) for r,i,a,b in zip(real,imag,imag0,real0)])
        real,imag = square(real0,imag0)
        power = params[0] & 0xffffffff
        shifted = power >> 1  # logical SHR, never signed division
        if shifted == 3: real,imag = times_base(real,imag)
        shifted = (shifted-2) & 0xffffffff
        if not shifted & 0x80000000: real,imag = square(real,imag)
        shifted = (shifted-2) & 0xffffffff
        if not shifted & 0x80000000: real,imag = square(real,imag)
        if power & 1: real,imag = times_base(real,imag)
        imag = [add(imag[0],jx),add(imag[1],jy)]
        real = [add(v,jz) for v in real]
        s['dfree1'],s['dfree2'] = imag
        s['derivative[1]'],s['derivative[2]'] = real
        # Original SHUFPD low lanes add high+low, preserving signed zeros.
        sr,si = add(real[1],real[0]),add(imag[1],imag[0])
        energy = [add(mul(r,r),mul(i,i)) for r,i in zip(real,imag)]
        numerator = mul(add(energy[0],energy[1]),params[2])
        denominator = add(add(mul(si,si),mul(sr,sr)),params[1])
        scale = julia.sqrt_bits(divide(numerator,denominator))
        s['xyz[0]'],s['xyz[1]'] = mul(sr,scale),mul(si,scale)
    else: raise ValueError('unknown original formula')
    s['current_slot'] = 0
    return s


def _program(name, params, state):
    ident, option, count, traits, flags, driver = FORMULAS[name]
    if len(params) != count: raise ValueError('wrong parameter count')
    p=[0]*2048
    p[:15]=[1,driver,6,0,0,0,0,state['max_iterations'],state['julia'],0,0,16384,4096,0,0]
    p[16],p[18],p[21],p[22]=state['rstop'],ONE,fp32(1<<20),ONE
    p[24:28]=[state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4):p[28+5*i]=ONE
    p[50]=state['hybrid_counts[0]']
    for slot in range(6):p[56+slot*332]=1
    base=56;p[base:base+12]=[1,1,ident,option,count,traits,flags,0,0,1,0,0]
    memory=bytearray(512);struct.pack_into('<Q',memory,248,0x3fe0000000000000)
    if name=='KaliLinCombSSE2sm' and params[1:]!=[0,HALF]: raise ScopeError('original fixed constants required')
    for i,((offset,kind),value) in enumerate(zip(LAYOUT[name],params)):
        if kind=='int32':
            if type(value) is not int or not -(1<<31)<=value<(1<<31):raise ScopeError('raw int32 required')
            struct.pack_into('<i',memory,256+offset,value)
            original,prepared=tail.binary64(value),fp32(value)
        else:
            original,prepared=tail.wide32(value),value
            struct.pack_into('<Q',memory,256+offset,original)
        p[base+12+i]=prepared
        p[base+76+2*i:base+78+2*i]=[original&0xffffffff,original>>32]
    p[base+204:base+268]=struct.unpack('<64I',memory[:256])
    # Typed original memory is preserved above. Unused cache cells are zero:
    # reinterpreting mixed int32/empty bytes as double has no original meaning.
    p[base+268]=HALF
    if name=='MsltoeSym2':p[base+269]=params[0]
    return p


def _parameters(name,program):
    params=program[68:68+FORMULAS[name][2]]
    if name=='KaliLinCombSSE2sm':
        power=tail.fraction64(program[132]|program[133]<<32)
        if power.denominator!=1 or not -(1<<31)<=power<(1<<31):raise ScopeError('invalid original int32 metadata')
        params[0]=int(power)
    return params


def _orbit(name,state,params):
    # Original doHybridPas body with caller-owned SSE transport from the
    # pinned NEW FPC i386 -CfSSE2 -O2 PE; no interpolation/smoothing.
    s=dict(state)
    for i in range(3):
        s[f'xyz[{i}]']=s[f'c[{i}]']
        s[f'j[{i}]']=s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]']=0;s['first']=s['iteration']=0
    s['rout']=s['otrap']=tail.radius(s)
    s['sse[0]']=add(mul(s['xyz[0]'],s['xyz[0]']),mul(s['xyz[1]'],s['xyz[1]']))
    s['sse[1]']=s['sse[3]']=0;s['sse[2]']=s['rout']
    remaining=count=s['hybrid_counts[0]'];s['tmp']=remaining
    for _ in range(s['max_iterations']):
        s['rold']=s['rout']
        if remaining==0:remaining=count
        s=_point(name,s,params)
        remaining-=1;s['tmp']=remaining;s['iteration']+=1
        s['rout']=tail.radius(s)
        if fraction(s['rout'])<fraction(s['otrap']):s['otrap']=s['rout']
        # Original PE compares maxIt before loading RStop into XMM0.low.
        s['sse[0]']=s['rout'] if s['iteration']>=s['max_iterations'] else s['rstop']
        s['sse[1]']=s['sse[3]']=0;s['sse[2]']=s['rout']
        if fraction(s['rout'])>fraction(s['rstop']):break
    return s


def evaluate(payload):
    if set(payload)!={'identity','kind','input_state_bits','parameter_bits'}:raise ValueError('exact semantic keys required')
    identity=payload['identity']
    if not isinstance(identity,list) or len(identity)!=3 or identity[0]!='external' or identity[2] not in FORMULAS or identity[1]!=FORMULAS[identity[2]][0]:raise ValueError('wrong symmetry identity')
    name=identity[2]
    def decode(v):
        if not isinstance(v,str) or not re.fullmatch(r'0x[0-9a-f]{8}',v):raise ValueError('hex32 required')
        return int(v,16)
    if set(payload['input_state_bits'])!=set(STATE) or len(payload['parameter_bits'])!=2048:raise ValueError('wrong ABI width')
    s={k:decode(v) for k,v in payload['input_state_bits'].items()};p=[decode(v) for v in payload['parameter_bits']]
    for k in STATE[:33]:tail.normal_or_zero(s[k])
    if payload['kind'] not in ('point','orbit') or s['julia'] not in (0,1) or s['inside'] or s['calc_smooth']:raise ScopeError('unsupported Hybrid3D caller')
    if not 1<=s['max_iterations']<=8 or not 1<=s['hybrid_counts[0]']<=3:raise ScopeError('work bound exceeded')
    params=_parameters(name,p)
    if p!=_program(name,params,s):raise ValueError('typed ABI/program mismatch')
    if payload['kind']=='orbit':
        if s['vary_scale']!=ONE or s['j[3]']!=s['original_j[3]']:raise ScopeError('unsupported caller entry')
        out=_orbit(name,s,params)
    else:out=_point(name,s,params)
    for k in STATE[:33]:tail.normal_or_zero(out[k])
    return {k:word(out[k]) for k in STATE}


def dependency_paths():return [Path(__file__).with_name(n).resolve() for n in DEPENDENCY_PINS]


def manifest():
    if hashlib.sha256((ROOT/CALLER_PROFILE['path']).read_bytes()).hexdigest() != CALLER_PROFILE['sha256']:
        raise ValueError('pinned new FPC caller proof changed')
    def verified(pins,directory=ROOT):
        out={}
        for name,pin in pins.items():
            p=directory/name;raw=p.read_bytes()
            if hashlib.sha256(raw.replace(b'\r\n',b'\n')).hexdigest()!=pin:raise ValueError('frozen dependency changed: '+name)
            out[str(p.relative_to(ROOT))]=hashlib.sha256(raw).hexdigest()
        return out
    sources,abi,deps=verified(SOURCE_PINS),verified(ABI_PINS),verified(DEPENDENCY_PINS,Path(__file__).parent)
    for name,audit in CODE_AUDIT.items():
        text=(ROOT/'mb3d/M3Formulas'/f'{name}.m3f').read_text()
        code=bytes.fromhex(re.sub(r'\s+','',re.search(r'\[CODE\](.*?)\[END\]',text,re.S)[1]))
        if len(code)!=audit['bytes'] or hashlib.sha256(code).hexdigest()!=audit['sha256']:raise ValueError('original CODE changed')
    parsed_kali_constant()
    return {'schema':'mb3d.native-group04-symmetry-reference.v2','caller_profile':dict(CALLER_PROFILE),'semantic_sources':sources,'abi_sources':abi,'dependencies':deps,'code_audit':CODE_AUDIT,
            'arithmetic':'Independent rational RN32 operations and sqrt midpoint proof; original uint32 logical shifts/sign tests and raw int32 parameter',
            'model_limits':[
                'Three original bodies; point and positive-count single-slot Hybrid3D state; all47 output lanes, no distance scalar/full-scene claim',
                'Normal/zero finite inputs and intermediates only; singular division, subnormals and nonfinite outputs are rejected',
                'Original operation-order RN32 sqrt/division targets, not x87 parity or arbitrary compiler reassociation/trig claims',
                'Orbit radii require a singleton over original grouping/FMA possibilities; no arbitrary hybrids/interpolation/smoothing',
                'Msltoe uses caller Rout, equality swaps Y/Z and negates X, and Z has no Julia addition',
                'Benesi uses strict rho<X and the sign bit of Jx, including negative zero',
                'Point bodies preserve SSE transport lanes; orbits use the pinned NEW FPC i386 SSE2 caller profile, not the unrecreated historical executable',
                'Positive counted iterations return Rout in SSE0 when maxIt is reached (including simultaneous bailout), otherwise RStop; SSE2=Rout, SSE1/3=zero',
                'Kali initializes first==0 only and carries Dfree1/2 and Deriv2/3',
                'Kali original parser rejects the literal trailing semicolon in 1e-30; and stores zero; source text remains unchanged',
                'Power covers signed int32 values but runs original bounded SHR/SUB/JS sequence, not generic exponentiation',
                'Prepared ABI fixture, not bit-exact host preparation: unused cache cells are zero rather than mixed-typed alias casts; original CODE reads only declared source-used cells, and raw typed words/param_bits remain exact'],
            'model_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def _state():return matrix._state()


def build_cases():
    sources=manifest()['semantic_sources'];cases=[]
    def vec(s,lane,values):
        for i,v in enumerate(values):s[f'{lane}[{i}]']=int(v,16) if isinstance(v,str) else fp32(v)
    def emit(name,variant,s,params,kind='point'):
        payload={'identity':['external',FORMULAS[name][0],name],'kind':kind,'input_state_bits':{k:word(s[k]) for k in STATE},'parameter_bits':[word(v) for v in _program(name,params,s)]}
        reads={f'xyz[{i}]' for i in range(3)}|{f'j[{i}]' for i in range(3)}
        writes={f'xyz[{i}]' for i in range(3)}|{'current_slot'}
        if name=='MsltoeSym2':reads.add('rout')
        if name=='KaliLinCombSSE2sm':reads|={'first','dfree1','dfree2','derivative[1]','derivative[2]'};writes|={'first','dfree1','dfree2','derivative[1]','derivative[2]'}
        if kind=='orbit':
            reads|={f'{v}[{i}]' for v in ('c','original_j') for i in range(4)}|{'julia','max_iterations','rstop','hybrid_counts[0]'}
            writes|={f'xyz[{i}]' for i in range(4)}|{f'j[{i}]' for i in range(3)}|{'rout','rold','otrap','first','iteration','tmp'}|{f'sse[{i}]' for i in range(4)}
        try:expected=evaluate(payload)
        except Exception as e:raise ValueError(f'{name}/{variant}: {e}') from e
        cases.append({'id':f'{FORMULAS[name][0]}-{variant}','identity':payload['identity'],'variant':variant,'input':payload,'expected_state_bits':expected,'read_lanes':[k for k in STATE if k in reads],'write_lanes':[k for k in STATE if k in writes],'source_hashes':sources})
    name='MsltoeSym2'
    for v,xyz,rout,mult,j in [
        ('positive_no_swap',(Fraction(3,8),Fraction(1,2),0),Fraction(25,64),1,(0,0,7)),
        ('positive_swap',(Fraction(3,8),0,Fraction(1,2)),Fraction(25,64),1,(0,0,7)),
        ('negative_y',(Fraction(3,8),Fraction(-1,2),0),1,1,(1,2,3)),
        ('negative_z_swap',(Fraction(3,8),0,Fraction(-1,2)),1,-2,(1,-2,3)),
        ('equal_absolute_opposite',(Fraction(3,8),Fraction(-1,2),Fraction(1,2)),Fraction(1,2),1,(0,0,3)),
        ('equal_absolute_reverse',(Fraction(3,8),Fraction(1,2),Fraction(-1,2)),Fraction(1,2),1,(0,0,3)),
        ('equal_values',(Fraction(3,8),Fraction(1,2),Fraction(1,2)),Fraction(1,2),1,(0,0,3)),
        ('caller_rout_sentinel',(Fraction(3,8),Fraction(1,2),Fraction(1,2)),2,1,(0,0,3)),
        ('zero_multiplier',(1,2,0),5,0,(0,3,4)),
        ('negative_multiplier',(1,2,0),5,-2,(0,3,4)),
        ('signed_zero',('0x80000000',1,'0x80000000'),1,1,('0x80000000',0,7))]:
        s=_state();vec(s,'xyz',xyz);vec(s,'j',j);s['rout']=fp32(rout);emit(name,v,s,[fp32(mult)])
    name='Benesi3Pow2'
    for v,xyz,j in [
        ('below_threshold',(Fraction(1,2),1,0),(0,1,2)),
        ('above_threshold',(2,1,0),(0,1,2)),
        ('equal_threshold',(1,1,0),('0x80000000',1,2)),
        ('negative_x',(-2,1,0),(0,1,2)),
        ('negative_Jx',(2,1,0),(-1,1,2)),
        ('negative_zero_Jx',(2,1,0),('0x80000000',1,2)),
        ('z_axis',(2,0,-1),(0,1,2)),
        ('pythagorean',(Fraction(5,4),3,4),(0,0,0)),
        ('irrational_sqrt',(1,1,1),(0,0,0)),
        ('zero_x',('0x80000000',-1,'0x80000000'),('0x80000000',0,0))]:
        s=_state();vec(s,'xyz',xyz);vec(s,'j',j);emit(name,v,s,[])
    name='KaliLinCombSSE2sm'
    powers=[0,1,2,3,4,5,6,7,8,9,10,16777215,16777216,16777217,2147483647,-2147483648,-2147483647,-1]
    for power in powers:
        s=_state();s['first']=0;vec(s,'xyz',(1,1,0));vec(s,'j',(Fraction(1,4),Fraction(1,4),0));emit(name,'power_'+str(power),s,[power,0,HALF])
    for first in (1,0x1000001,0x7fffffff,0x80000000,0xffffffff):
        s=_state();s['first']=first;s['dfree1']=s['dfree2']=ONE;s['derivative[1]']=s['derivative[2]']=0
        vec(s,'xyz',(9,8,7));vec(s,'j',(Fraction(1,4),Fraction(1,4),0));emit(name,'carried_first_'+str(first),s,[3,0,HALF])
    s=_state();s['first']=0;vec(s,'xyz',('0x80000000','0x80000000',1));vec(s,'j',('0x80000000',0,0));emit(name,'signed_zero_initialization',s,[3,0,HALF])
    # Point integer sentinels are unrelated to floating-point conversion.
    for name in FORMULAS:
        s=_state();s.update(first=0xffffffff,iteration=0x1000001,tmp=0x80000000)
        vec(s,'xyz',(1,1,0));vec(s,'j',(Fraction(1,4),Fraction(1,4),0));s['rout']=fp32(2)
        s['dfree1']=s['dfree2']=ONE;s['derivative[1]']=s['derivative[2]']=0
        emit(name,'integer_sentinels',s,[ONE] if name=='MsltoeSym2' else ([3,0,HALF] if name=='KaliLinCombSSE2sm' else []))
    for name in FORMULAS:
        for mode in ('mandel','julia','escape','equal_threshold','count_cycle'):
            s=_state();s['max_iterations']=3;s['hybrid_counts[0]']=2 if mode=='count_cycle' else 1
            s['julia']=int(mode=='julia');vec(s,'c',(1,1,0));vec(s,'original_j',(1,1,0));s['j[3]']=s['original_j[3]']
            if name=='MsltoeSym2':vec(s,'c',(Fraction(1,4),Fraction(1,4),0));vec(s,'original_j',(Fraction(1,4),Fraction(1,4),0))
            if mode=='julia':
                vec(s,'original_j',(Fraction(1,4),Fraction(-1,4),0) if name=='MsltoeSym2' else ((-1,1,0) if name=='Benesi3Pow2' else (Fraction(1,4),Fraction(1,4),0)))
            if mode=='escape':s['rstop']=fp32(Fraction(1,64))
            if mode=='equal_threshold':
                # Single exact first-step radius, then max>1 tests strict >.
                t=dict(s)
                for i in range(3):t[f'xyz[{i}]']=s[f'c[{i}]'];t[f'j[{i}]']=s[f'c[{i}]']
                t['rout']=tail.radius(t);t['first']=0
                params=[ONE] if name=='MsltoeSym2' else ([2,0,HALF] if name=='KaliLinCombSSE2sm' else [])
                s['rstop']=tail.radius(_point(name,t,params))
            emit(name,'orbit_'+mode,s,[ONE] if name=='MsltoeSym2' else ([2,0,HALF] if name=='KaliLinCombSSE2sm' else []),'orbit')
    return cases
