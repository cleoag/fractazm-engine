"""Original MapTranslate CODE/Pascal oracle with real, pinned RGB8 resources.

No device observation supplies expected values. Constant-colour spline
footprints use partition of unity and a derived FP32 error bound; only inputs
whose entire final rounding interval gives one word are accepted. This is a
finite native RN32 target, not arbitrary sampling or x87 image parity.
"""
from fractions import Fraction as F
from pathlib import Path
import hashlib
import re
import struct
import zlib

import native_formula_reference as affine
from gpu_pipeline_proofs import STATE
ROOT=Path(__file__).resolve().parents[2]
fp32, fraction, word=affine.binary32, affine.fraction, affine.word
ONE=0x3f800000
NEG_ZERO=0x80000000
PRIMITIVE_LF_SHA256='a0964784744671549916c1b53b227ea085be6e8eef18d6fe84bd79994295d94e'
FORMULAS={'_MapTranslate':(10006,-1,7,0,1,1)}
SOURCE_PINS={'mb3d/Calc.pas': 'eb290ad9001a80ad185df553401ec55bc1bf662141ee3f7169b78dff6714c419',
 'mb3d/CustomFormulas.pas': '9ed1c8289ebfb99202795af8daca6bef92650ba250a28c12435eb5beb4264c61',
 'mb3d/ImageProcess.pas': '3a2ccc5790fa8a61715b8b7838507080bc640ef72f9c87b6e3a7e66118303527',
 'mb3d/M3Formulas/_MapTranslate.m3f': 'c900cb71cf11a7e3f2faa9bbbdb7dd672843daf3e6376f3b7fdbf7a9c95c2c0e',
 'mb3d/Math3D.pas': '0b03d5d982be1285839831dfc36b2532725f5d679e2891b474d00d750b7b6727',
 'mb3d/TypeDefinitions.pas': '5ef479844d3d5ffac309b141c9f80b62ebf0f9bf8b8782b706d9f451ef096747',
 'mb3d/formulas.pas': '3fefcd04fe6726831f3a7904f151cc3cc192610f4532f2889b9771e5953775bd',
 'mb3d/maps/MB3DMaps.pas': '8a1f9d1707595d4b73360bf6d270ab2b9a9ae2a206d7362873eea522d6a464ca'}
ABI_PINS={'src/core/external_formulas.h': 'b36cba7e6b37c6426a8d052f2434866397f6edc7edb8d799efd0ffccc751f3b6',
 'src/render/native_metal/formula_abi.h': 'd69725ad200b889c8e7e2c3267d8fee96e37188b54e5cdc145a6cde3afb6350a'}
ASSET_PINS={'comparison/fixtures/native-formula-maps-v1/large-id-runtime/M3Maps/16777216.png': '0b85cf89462c255ccc62b7a3ef4617cabf1956eaf6978c51a6940bbf88f91615',
 'comparison/fixtures/native-formula-maps-v1/large-id-runtime/M3Maps/16777217.png': '15bf0e4d09477448464f22aeaed94dac7277dd6ad658fb62346748e396a476f4',
 'comparison/fixtures/native-formula-maps-v1/runtime/M3Maps/1.png': 'e52b93534f1a5e9927bfb6e051d04841d83e76db500893c2345cdf7a504eafa0',
 'comparison/fixtures/native-formula-maps-v1/runtime/M3Maps/2.png': 'c95a87eefd1913a003f703ace18dfe063dc88eae19516705aaab0e3ebcb4edd4',
 'comparison/fixtures/native-formula-maps-v1/runtime/M3Maps/3.png': '15bf0e4d09477448464f22aeaed94dac7277dd6ad658fb62346748e396a476f4'}
PATTERNS={1: ((255, 0, 0), (0, 255, 0), (0, 0, 255), (255, 255, 0)), 2: ((0, 255, 255), (255, 0, 255), (255, 255, 0), (0, 0, 255)), 3: ((255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255)), 16777216: ((0, 0, 0), (0, 0, 0), (0, 0, 0), (0, 0, 0)), 16777217: ((255, 255, 255), (255, 255, 255), (255, 255, 255), (255, 255, 255))}
CODE_AUDIT={'_MapTranslate': {'sha256': 'ea8b71660a1a8c7ed27dcc769400fb77579661f99aebded23aa3c122b0449109', 'bytes': 80, 'typed_selector_offset': -12, 'singles_offsets': [-16, -20, -24, -28, -32, -36], 'callback_offset': 352, 'writes': ['xyz[0]', 'xyz[1]', 'xyz[2]']}}

class ScopeError(ValueError):
    """Outside independently proved finite domain."""


def normal(bits):
    value=fraction(bits)
    if value and bits & 0x7f800000 == 0:
        raise ScopeError('subnormal outside model')
    return value


def spline_error_proof():
    """Forward error, derived from Math3D.MakeSplineCoeff and Calc.SplineIpolMap.

    Pair (M,e) bounds mathematical absolute value and absolute arithmetic
    error. Every source elementary operation below has |actual|<8, hence
    RN32 error <=2^-21 (including tiny/flush-to-zero results). FMA removes
    an intermediate error and is contained by these positive error bounds.
    Scalars 1/6 and 1/255 each conservatively get the same error allowance.
    Coefficient mathematical values are nonnegative and sum exactly to one.
    """
    E=F(1,1<<21)
    def add(a,b):
        out=(a[0]+b[0],a[1]+b[1]+E)
        if sum(out)>=8: raise AssertionError('rounding bound magnitude escaped')
        return out
    def mul(a,b):
        out=(a[0]*b[0],a[0]*b[1]+b[0]*a[1]+a[1]*b[1]+E)
        if sum(out)>=8: raise AssertionError('rounding bound magnitude escaped')
        return out
    one=(F(1),F(0));x=one;half=(F(1,2),F(0));sixth=(F(1,6),E)
    c3=mul(mul(mul(x,x),x),sixth)
    # 1-x lies in [0,1], despite the dependency-blind addition bound2.
    one_minus_x=(F(1),E)
    c0=add(add(sixth,mul(mul(one_minus_x,x),half)),c3)
    c2=add(add(add(x,c0),c3),c3)
    c1=add(add(add(one,c2),c3),c0)
    errors=[c0[1],c1[1],c2[1],c3[1]]
    # Exact B-spline coefficients lie in [0,1]. A row of constant channel
    # 255 is sum(255*(wi/255)); bound the unnormalised pixel multiplication
    # explicitly, rather than incorrectly treating 255 as <=8.
    weighted=[]
    for error in errors:
        normalized=mul((F(1),error),(F(1,255),E))
        product_error=255*normalized[1]+E
        if 1+product_error>=8: raise AssertionError('pixel product escaped')
        weighted.append(product_error)
    row_error=sum(weighted)+3*E
    # All four rows have the same true value 0 or1. Use sum(wy)=1 for
    # propagation of row errors, not a guessed uniform interpolation epsilon.
    sample_error=row_error*(1+sum(errors))+sum(errors)+7*E
    if sample_error>=F(1,128): raise AssertionError('bound too loose for corpus')
    return {'round_error':E,'coefficient_errors':errors,'row_error':row_error,
            'sample_error':sample_error}


def png_pixels(path):
    """Decode only the authored canonical RGB8 PNG; no image-library oracle."""
    data=Path(path).read_bytes()
    if data[:8]!=b'\x89PNG\r\n\x1a\n': raise ValueError('PNG signature')
    parts=[];pos=8
    while pos<len(data):
        if pos+12>len(data): raise ValueError('truncated PNG')
        n=struct.unpack_from('>I',data,pos)[0];end=pos+12+n
        if end>len(data): raise ValueError('truncated PNG payload')
        tag=data[pos+4:pos+8];payload=data[pos+8:end-4]
        if zlib.crc32(tag+payload)!=struct.unpack_from('>I',data,end-4)[0]:
            raise ValueError('PNG CRC')
        parts.append((tag,payload));pos=end
    if [t for t,_ in parts]!=[b'IHDR',b'IDAT',b'IEND'] or parts[2][1]:
        raise ValueError('PNG chunks')
    if struct.unpack('>IIBBBBB',parts[0][1])!=(64,32,8,2,0,0,0):
        raise ScopeError('only pinned RGB8 64x32 maps')
    dec=zlib.decompressobj();raw=dec.decompress(parts[1][1],6177)
    if len(raw)!=6176 or not dec.eof or dec.unused_data or dec.unconsumed_tail:
        raise ValueError('PNG complete stream')
    if any(raw[i*193] for i in range(32)): raise ValueError('PNG filter')
    return [tuple(raw[y*193+1+x*3:y*193+4+x*3]) for y in range(32) for x in range(64)]


def resources(large=False):
    ids=[16777216,16777217] if large else [1,2,3]
    runtime='comparison/fixtures/native-formula-maps-v1/'+('large-id-runtime' if large else 'runtime')
    files=[];descriptors=[];words=[]
    for identity in ids:
        rel=runtime+f'/M3Maps/{identity}.png';data=(ROOT/rel).read_bytes()
        sha=hashlib.sha256(data).hexdigest()
        if sha!=ASSET_PINS[rel]: raise ValueError('authored PNG changed')
        pixels=png_pixels(ROOT/rel)
        expected=[PATTERNS[identity][x//16] for y in range(32) for x in range(64)]
        if pixels!=expected: raise ValueError('authored pixel pattern differs')
        files.append({'path':rel,'sha256':sha})
        descriptors.append([identity,64,32,3,len(words),65*33*3,0,0])
        # Original MB3DMaps copies first column and first row; RGB order is
        # source scalar flip-RB and independently traced SSE2 output lanes.
        words.extend(fp32(v) for y in range(33) for x in range(65)
                     for v in pixels[(y%32)*64+x%64])
    entries=list((ROOT/runtime).rglob('*'))
    if any(p.is_symlink() for p in entries) or {str(p.relative_to(ROOT)) for p in entries if p.is_file()}!={r['path'] for r in files} or any(not p.is_file() and p!=ROOT/runtime/'M3Maps' for p in entries):
        raise ValueError('unbound runtime candidate')
    return {'schema':'mb3d.native-formula-map-input.v1','root':runtime,'map_ids':ids,
            'files':files,'descriptor_words':descriptors,'pixel_count':len(words),
            'pixel_words_sha256':hashlib.sha256(struct.pack('<'+str(len(words))+'I',*words)).hexdigest(),
            'controls':'active'}


def wide(value):
    return struct.unpack('<Q',struct.pack('<d',float(value)))[0]


def narrow64(bits):
    sign=(bits>>32)&NEG_ZERO;exp=(bits>>52)&2047;mant=bits&((1<<52)-1)
    if exp==2047: return sign|0x7f800000|((0x400000|(mant>>29)) if mant else 0)
    if exp: mant|=1<<52
    shift=exp-1075 if exp else -1074
    val=F(mant*(1<<shift)) if shift>=0 else F(mant,1<<-shift)
    return fp32(-val if sign else val,bool(sign))


def _program(selector,params,state):
    if type(selector) is not int or not 0<selector<1<<31 or len(params)!=6:
        raise ScopeError('original int32 selector and six Singles required')
    p=[0]*2048
    p[:15]=[1,1,6,0,0,0,2,state['max_iterations'],state['julia'],0,0,16384,4096,0,0]
    p[16],p[18],p[21],p[22]=state['rstop'],ONE,fp32(1<<20),ONE
    p[24:28]=[state[f'original_j[{i}]'] for i in range(4)]
    for i in range(4):p[28+5*i]=ONE
    p[50]=state['hybrid_counts[0]']
    for i in range(6):p[56+332*i]=1
    p[56:68]=[1,1,10006,0xffffffff,7,0,1,0,0,1,0,0]
    mem=bytearray(512);struct.pack_into('<Q',mem,248,0x3fe0000000000000)
    struct.pack_into('<i',mem,244,selector)
    for i,v in enumerate(params):struct.pack_into('<I',mem,240-4*i,v)
    values=[F(selector)]+[normal(v) for v in params]
    for i,v in enumerate(values):
        p[68+i]=fp32(v, bool(i and params[i-1]&NEG_ZERO))
        original=wide(v) if v else (1<<63 if i and params[i-1]&NEG_ZERO else 0)
        p[132+2*i:134+2*i]=[original&0xffffffff,original>>32]
    p[260:324]=struct.unpack('<64I',mem[:256])
    for i in range(32):
        p[324+i]=narrow64(struct.unpack_from('<Q',mem,248-8*i)[0])
        p[356+i]=narrow64(struct.unpack_from('<Q',mem,256+8*i)[0])
    return p


def footprint_proof():
    """Rational enclosure independent of trigonometric implementations."""
    # sqrt(3) lies strictly here. RN32 sqrt/division errors near1 are less
    # than2^-20, so both original equal-coordinate normalisation and its
    # scaled implementation fit the deliberately wider [0.57,0.59] interval.
    low,high=F(1732,1000),F(1733,1000)
    assert low*low<3<high*high
    delta=F(1,1<<20)
    assert F(57,100)<1/(high+delta)-delta
    assert 1/(low-delta)+delta<F(59,100)
    # Original atan2 arguments x,y have same magnitude; even independent
    # perturbations anywhere in this box deviate from +/-pi/4 or +/-3pi/4
    # by less than0.04, since derivative of arctan <=1.
    ratio_error=F(59,57)-1
    assert ratio_error<F(4,100)
    # 3<pi<22/7 suffices: angle error/(2*pi) plus2^-12 for precise atan2
    # (<=6ULP at magnitude<4), rounded inverse2pi, multiply/add <1/64.
    E=F(1,1<<21)
    reciprocal_error=E/(2*3*(3-E))+E
    projection_rounding=4*reciprocal_error+6*E*(F(1,6)+reciprocal_error)+2*E
    assert projection_rounding<F(1,1<<12)
    u_error=F(4,100)/6+F(1,1<<12)
    assert u_error<F(1,64)
    # Centres8,24,40,56, floor within centre-1..centre+1; spline accesses
    # floor-1 through floor+2, so all columns are strictly in their stripe.
    for centre in (8,24,40,56):
        assert centre-2>=16*(centre//16)
        assert centre+3<16*(centre//16+1)
    return {'normalised_abs_bounds':[F(57,100),F(59,100)],
            'longitude_error':u_error,'projection_rounding_error':projection_rounding,
            'allowed_longitude_error':F(1,64)}


def footprint_colour(xyz,selector):
    """Proven interior sphere footprints, not host transcendental evaluation.

    Equal absolute coordinates give atan2 quadrants at +/-pi/4,+/-3pi/4.
    Normalisation preserves the x/y ratio. Even allowing normalised absolute
    components [0.57,0.59], atan2 deviation is <0.04 radians (arctan is
    1-Lipschitz); 6ULP atan2 and all projection operations contribute <2^-12
    to u. Thus u lies within centre +/-1/64. Pixel footprint is within one
    16-column stripe. Every row is identical, so latitude and vertical
    orientation cannot affect colour; asin remains finite for |z/n|<0.59.
    Solid maps need no footprint claim and accept a single nonzero axis.
    """
    footprint_proof()
    if selector not in PATTERNS:raise ScopeError('unproved selector')
    vals=[normal(v) for v in xyz];pattern=PATTERNS[selector]
    if len(set(pattern))==1:
        if not any(vals):raise ScopeError('zero sphere vector unproved')
        return [F(v,255) for v in pattern[0]]
    if any(abs(v)!=1<<20 for v in vals):raise ScopeError('outside interior footprint proof')
    x,y,_=vals
    stripe=(1 if y>0 else 0) if x>0 else (2 if y>0 else 3)
    return [F(v,255) for v in pattern[stripe]]


def _point(s,selector,params):
    colours=footprint_colour([s[f'xyz[{i}]'] for i in range(3)],selector)
    out=dict(s);E=spline_error_proof()['round_error'];B=spline_error_proof()['sample_error']
    for i in range(3):
        coord=normal(s[f'xyz[{i}]']);off=normal(params[i]);scale=normal(params[i+3])
        if abs(off)>1 or abs(scale)>2:raise ScopeError('outside bounded affine sample use')
        if scale==0:
            # Positive zero result is added to the coordinate; avoid claiming
            # an arbitrary signed-zero multiplication when sample is uncertain.
            if coord==0 and s[f'xyz[{i}]']&NEG_ZERO:raise ScopeError('negative zero coordinate unproved')
            out[f'xyz[{i}]']=fp32(coord);continue
        if abs(coord)<1<<20 or abs(coord)>(1<<20)+8:raise ScopeError('outside final rounding cells')
        target=coord+(colours[i]+off)*scale
        error=(B+E)*abs(scale)+E
        lo,hi=fp32(target-error),fp32(target+error)
        if lo!=hi or lo!=fp32(target):raise ScopeError('sample interval crosses rounding cell')
        out[f'xyz[{i}]']=lo
    out['current_slot']=0
    return out


def radius(s):
    vals=[normal(s[f'xyz[{i}]']) for i in range(3)]
    if sum(v!=0 for v in vals)!=1:raise ScopeError('orbit radius needs one nonzero axis')
    # One product and zero products: all associations/FMA evaluate RN32(x*x).
    return fp32(sum(v*v for v in vals))


def _orbit(s,selector,params):
    s=dict(s)
    for i in range(3):
        s[f'xyz[{i}]']=s[f'c[{i}]'];s[f'j[{i}]']=s[f'original_j[{i}]'] if s['julia'] else s[f'c[{i}]']
    s['xyz[3]']=ONE;s['first']=s['iteration']=0;s['rout']=s['otrap']=radius(s)
    remaining=count=s['hybrid_counts[0]'];s['tmp']=remaining
    for _ in range(s['max_iterations']):
        s['rold']=s['rout']
        if remaining==0:remaining=count
        s=_point(s,selector,params);remaining-=1;s['tmp']=remaining;s['iteration']+=1;s['rout']=radius(s)
        if fraction(s['rout'])<fraction(s['otrap']):s['otrap']=s['rout']
        if fraction(s['rout'])>fraction(s['rstop']):break
    return s


def evaluate(payload):
    if set(payload)!={'identity','kind','input_state_bits','parameter_bits','resources'} or payload['identity']!=['external',10006,'_MapTranslate']:
        raise ValueError('exact map semantic fields required')
    def decode(v):
        if not isinstance(v,str) or not re.fullmatch('0x[0-9a-f]{8}',v):raise ValueError('hex32 required')
        return int(v,16)
    if set(payload['input_state_bits'])!=set(STATE) or len(payload['parameter_bits'])!=2048:raise ValueError('ABI width')
    s={k:decode(v) for k,v in payload['input_state_bits'].items()};p=list(map(decode,payload['parameter_bits']))
    for lane in STATE[:33]:normal(s[lane])
    if payload['kind'] not in ('point','orbit') or s['julia'] not in (0,1) or s['inside'] or s['calc_smooth'] or not 1<=s['max_iterations']<=4 or not 1<=s['hybrid_counts[0]']<=3:
        raise ScopeError('bounded analytic3D caller only')
    raw=struct.pack('<64I',*p[260:324]);selector=struct.unpack_from('<i',raw,244)[0];params=p[69:75]
    if p!=_program(selector,params,s):raise ValueError('typed parameter ABI differs')
    if payload['resources']!=resources(selector>=1<<24):raise ValueError('resource binding differs')
    if selector not in payload['resources']['map_ids']:raise ValueError('unloaded selector')
    if payload['kind']=='orbit':
        if s['vary_scale']!=ONE or s['j[3]']!=s['original_j[3]']:raise ScopeError('analytic3D entry')
        out=_orbit(s,selector,params)
    else:out=_point(s,selector,params)
    for lane in STATE[:33]:normal(out[lane])
    return {k:word(out[k]) for k in STATE}


def dependency_paths():return [Path(affine.__file__).resolve()]


def primitive_identity(data):
    if hashlib.sha256(data.replace(b'\r\n',b'\n')).hexdigest()!=PRIMITIVE_LF_SHA256:raise ValueError('frozen arithmetic changed')
    return hashlib.sha256(data).hexdigest()


def manifest():
    def pins(values):
        result={}
        for path,expected in values.items():
            data=(ROOT/path).read_bytes()
            if hashlib.sha256(data.replace(b'\r\n',b'\n')).hexdigest()!=expected:raise ValueError('pinned source changed: '+path)
            result[path]=hashlib.sha256(data).hexdigest()
        return result
    code=bytes.fromhex(re.sub(r'\s+','',re.search(r'\[CODE\](.*?)\[END\]',(ROOT/'mb3d/M3Formulas/_MapTranslate.m3f').read_text(),re.S)[1]))
    if hashlib.sha256(code).hexdigest()!=CODE_AUDIT['_MapTranslate']['sha256'] or len(code)!=80:raise ValueError('original CODE changed')
    resources();resources(True)
    proof=spline_error_proof(); footprint=footprint_proof()
    return {'schema':'mb3d.native-group03-map-reference.v1','semantic_sources':pins(SOURCE_PINS),'abi_sources':pins(ABI_PINS),
            'code_audit':CODE_AUDIT,'dependencies':{str(dependency_paths()[0].relative_to(ROOT)):primitive_identity(dependency_paths()[0].read_bytes())},
            'assets':ASSET_PINS,'footprint_proof':{k:([str(x) for x in v] if isinstance(v,list) else str(v)) for k,v in footprint.items()},'spline_error_proof':{k:([str(x) for x in v] if isinstance(v,list) else str(v)) for k,v in proof.items()},
            'arithmetic':'Original typed int32/Single ABI; constant-footprint rational partition-of-unity; bounded final RN32 interval',
            'model_limits':['Only pinned positive-ID RGB8 64x32 real PNGs; all rows equal, so no vertical-orientation claim',
              'No absent/zero map qualification; actual descriptor/pixel bytes and negative controls required by observer',
              'Interior striped points have xyz absolute2^20; solid maps allow bounded one-axis orbits',
              'Explicit native RN-even target, not universal MSL rounding guarantee or original x87 parity',
              'Basic operation bound2^-21 for actual magnitudes<8; precise atan2<=6ULP used only to bound footprint',
              'Not arbitrary texture interpolation, nonconstant spline boundaries, RGB16, distance scalars or full scenes'],
            'model_sha256':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()}


def _state():
    s={k:fp32(F(i+1,32)) for i,k in enumerate(STATE)}
    s.update(iteration=19,first=17,tmp=23,max_iterations=4,julia=0,inside=0,calc_smooth=0,current_slot=5)
    for i in range(6):s[f'hybrid_counts[{i}]']=1 if i==0 else 0
    for i in range(4):
        s[f'xyz[{i}]']=fp32(1<<20);s[f'c[{i}]']=fp32(1<<20);s[f'j[{i}]']=fp32(F(i+1,4));s[f'original_j[{i}]']=fp32(-F(i+1,4))
    s['j[3]']=s['original_j[3]'];s['rstop']=fp32(1<<44);s['vary_scale']=ONE
    return s


def build_cases():
    source=manifest()['semantic_sources'];cases=[]
    def emit(label,selector,state,values,kind='point'):
        params=[fp32(v) for v in values]
        payload={'identity':['external',10006,'_MapTranslate'],'kind':kind,'input_state_bits':{k:word(state[k]) for k in STATE},
                 'parameter_bits':[word(v) for v in _program(selector,params,state)],'resources':resources(selector>=1<<24)}
        reads={f'xyz[{i}]' for i in range(3)};writes=reads|{'current_slot'}
        if kind=='orbit':
            writes|={f'xyz[{i}]' for i in range(4)}|{f'j[{i}]' for i in range(3)}|{'rout','rold','otrap','first','iteration','tmp'}
            reads|={f'{v}[{i}]' for v in ('c','original_j') for i in range(4)}|{'julia','max_iterations','rstop','hybrid_counts[0]'}
        cases.append({'id':'10006-'+label,'identity':payload['identity'],'variant':label,'input':payload,'expected_state_bits':evaluate(payload),
                      'read_lanes':[k for k in STATE if k in reads],'write_lanes':[k for k in STATE if k in writes],'source_hashes':source})
    for selector in (1,2):
        for signs in ((1,1,1),(1,-1,1),(-1,1,1),(-1,-1,1),(1,1,-1),(1,-1,-1),(-1,1,-1),(-1,-1,-1)):
            s=_state()
            for i,sign in enumerate(signs):s[f'xyz[{i}]']=fp32(sign*(1<<20))
            emit(f'map{selector}_octant_'+''.join('p' if v>0 else 'n' for v in signs),selector,s,[0,0,0,1,1,1])
    emit('offsets_scales',1,_state(),[F(-1,2),F(1,2),-1,2,-1,F(1,2)])
    for selector in (16777216,16777217):emit(f'typed_id_{selector}',selector,_state(),[0,0,0,1,1,1])
    for axis in range(3):
        for julia in (0,1):
            for escape in (False,True):
                s=_state();s['julia']=julia;s['hybrid_counts[0]']=2
                for i in range(3):s[f'c[{i}]']=fp32(1<<20) if i==axis else 0
                scales=[0,0,0];scales[axis]=1
                if escape:s['rstop']=fp32(F((1<<20)+1)**2)
                emit(f'orbit_axis{axis}_j{julia}_'+('escape' if escape else 'repeat'),3,s,[0,0,0]+scales,'orbit')
    return cases
