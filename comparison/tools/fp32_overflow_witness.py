"""Exact rational overflow checks; no shader helper is used as the oracle."""
from fractions import Fraction
import gzip,hashlib,struct
from qualify_native_metal_all import require

MAX=Fraction((2**24-1)*2**104)
EPS=Fraction(struct.unpack('<f',struct.pack('<f',1e-40))[0])
NAMES=('program','scene','geometry','primary','surface','rays','normals','normal_results','inside','witness')


def number(word):
    require(type(word) is int and 0<=word<2**32 and word&0x7f800000!=0x7f800000,'Nonfinite witness input')
    return Fraction(struct.unpack('<f',struct.pack('<I',word))[0])


def genuine(words,program):
    require(len(words)==20 and not any(words[18:]),'Witness ABI/reserved words')
    kind,slot,iteration,power=words[:4]
    require(kind in (0,1,2),'Unknown overflow witness')
    if kind==0:
        require(not any(words),'Noncanonical empty witness');return False
    require(slot<6 and iteration<=2**20,'Witness slot/iteration bounds')
    x,y,z,w=map(number,words[4:8])
    if kind==1:
        require(power==0 and words[17] in (0,1) and not any(words[8:17]),'Radius witness shape')
        return x*x+y*y+z*z+(w*w if words[17] else 0)>MAX
    require(power==8 and words[17]==0,'Unimplemented formula proof')
    jx,jy,jz,_=map(number,words[8:12]);zm=number(words[16]);require(abs(zm)==1,'Unproved power8 z multiplier')
    require(any(v&0x7f800000==0x7f800000 for v in words[12:15]),'Power witness did not overflow XYZ')
    require(len(program) in (8192,16384),'Program capture size')
    matches=[]
    for off in range(0,len(program),8192):
        base=off+224+slot*1328
        ns,ident=struct.unpack_from('<II',program,base+4);p,enabled=struct.unpack_from('<II',program,base+32)
        zbits=struct.unpack_from('<I',program,base+1076)[0]
        matches.append(ns==0 and ident==0 and p==8 and enabled==1 and zbits==words[16])
    require(any(matches),'Witness formula/parameter differs from uploaded program')
    sx,sy,sz=x*x,y*y,z*z;R=sx+sy;require(R>0,'Power8 polar singularity is not overflow proof')
    rr,zz,xx,yy=R*R,sz*sz,sx*sx,sy*sy
    numerator=zz*(70*rr+zz)-28*sz*R*(zz+rr)
    xpoly=xx*xx+yy*(70*xx+yy)-28*sx*sy*(yy+xx)
    ypoly=8*x*y*(yy*(7*sx-sy)+xx*(sx-7*sy))
    # Metal may flush the literal subnormal epsilon. Prove both denominator
    # endpoints; each component is monotone between them, with unchanged sign.
    for polynomial,addition in ((xpoly,jx),(ypoly,jy)):
        lo=(1+numerator/(rr*rr))*polynomial+addition
        hi=(1+numerator/(rr*rr+EPS))*polynomial+addition
        if lo>MAX and hi>MAX or lo< -MAX and hi< -MAX:return True
    # Z=K*sqrt(R)+jz. Squaring this strict triangle bound is exact, and needs
    # neither rounded sqrt nor a numerical tolerance.
    K=-zm*8*z*(sz-R)*(zz-6*R*sz+rr)
    return K*K*R>(MAX+abs(jz))**2


def capture(path):
    result={'hashes':{},'lengths':{}};data={}
    with gzip.open(path,'rb') as f:
        header=f.read(20);require(len(header)==20,'Truncated capture header')
        magic,version,width,height,sections=struct.unpack('<5I',header)
        require(magic==0x4d425257 and version==1 and sections==10 and 0<width<=4096 and 0<height<=4096,'Capture ABI')
        count=width*height;result['size']=[width,height]
        for name in NAMES:
            b=f.read(8);require(len(b)==8,'Missing section length');length=struct.unpack('<Q',b)[0]
            require(0<length<=max(16384,count*384),'Capture section exceeds bound')
            if name=='program':require(length in (8192,16384),'Program capture size')
            if name=='scene':require(length==336,'Scene ABI')
            strides={'geometry':112,'primary':176,'surface':48,'rays':64,'normals':32,'normal_results':64,'inside':32,'witness':80}
            if name in strides:require(length==strides[name]*count,'Captured '+name+' ABI')
            h=hashlib.sha256();parts=[];remaining=length
            while remaining:
                chunk=f.read(min(remaining,1<<20));require(chunk,'Truncated capture section');remaining-=len(chunk);h.update(chunk)
                if name in ('program','geometry','witness'):parts.append(chunk)
            result['hashes'][name]=h.hexdigest();result['lengths'][name]=length
            if parts:data[name]=b''.join(parts)
        require(not f.read(1),'Trailing capture data')
    result['data']=data;return result


def compare(baseline_path,witness_path,*,require_threshold=True):
    a=capture(baseline_path);b=capture(witness_path)
    require(a['size']==b['size'] and a['lengths']==b['lengths'],'Capture shape differs')
    require(all(a['hashes'][k]==b['hashes'][k] for k in NAMES if k!='witness'),'Instrumentation changed ordinary GPU outputs')
    require(not any(a['data']['witness']),'Baseline unexpectedly wrote witnesses')
    n=a['size'][0]*a['size'][1];g=b['data']['geometry'];w=b['data']['witness'];proven=0;range_pixels=0;observed={1:0,2:0};valid={1:0,2:0}
    for i in range(n):
        version,fault=struct.unpack_from('<II',g,i*112);require(version==6 and not fault and not struct.unpack_from('<I',g,i*112+32)[0],'Faulted primary capture')
        exits=struct.unpack_from('<I',g,i*112+72)[0];range_pixels+=bool(exits)
        words=struct.unpack_from('<20I',w,i*80)
        if words[0]:require(exits>0,'Witness belongs to pixel without range exit');observed[words[0]]=observed.get(words[0],0)+1
        ok=genuine(words,b['data']['program']);proven+=ok
        if ok:valid[words[0]]+=1
    if require_threshold:require(proven*100>n,'Genuine overflow does not establish strict >1% policy')
    return {'size':a['size'],'geometry_pixels':n,'range_pixels':range_pixels,'proven_pixels':proven,
        'unattributed_range_pixels':range_pixels-proven,'observed_by_kind':observed,'proven_by_kind':valid,
        'primary_outputs_exact':True,'primary_section_hashes':{k:a['hashes'][k] for k in NAMES if k!='witness'},
        'threshold_percent':1,'threshold_proven':proven*100>n}
