"""Independent-review-selected dense rotation/cancellation holdout."""
import math,random,struct,argparse
from pathlib import Path
from run_m3parameter_gpu import read,write,invoke,require
from verify_native_wrapper_specialization import word
ROOT=Path(__file__).resolve().parents[2]
def run(base,candidate,evidence,out):
    require(not out.exists(),'Fresh output required');out.mkdir(parents=True)
    original=list(struct.unpack('<2048I',(evidence/'77923ebd629a2fb2/0-nominal.program').read_bytes()))
    raw=(evidence/'77923ebd629a2fb2/states.bin').read_bytes();states=[list(struct.unpack('<47I',raw[i:i+188])) for i in range(0,len(raw),188)]
    matrix=[[float(i==j) for j in range(4)] for i in range(4)]
    for a,b,angle in [(0,1,.37),(1,2,.53),(2,3,-.71),(0,3,.29)]:
        c,s=math.cos(angle),math.sin(angle);x=matrix[a][:];y=matrix[b][:]
        matrix[a]=[c*u-s*v for u,v in zip(x,y)];matrix[b]=[s*u+c*v for u,v in zip(x,y)]
    require(max(abs(sum(matrix[i][k]*matrix[j][k] for k in range(4))-float(i==j)) for i in range(4) for j in range(4))<1e-12,'Invalid rotation')
    rng=random.Random(99517)
    for i,s in enumerate(states):
        scale=2.0**((i%21)-10);coords=[rng.uniform(-scale,scale) for _ in range(4)]
        if i%2==0:coords[3]=-sum(matrix[0][j]*coords[j] for j in range(3))/matrix[0][3]
        s[:4]=map(word,coords)
    sp=out/'states.bin';sp.write_bytes(b''.join(struct.pack('<47I',*s) for s in states));rows=[]
    for scale in (-1.1875,.875):
        p=list(original)
        values={0:scale,**{5+i*4+j:matrix[i][j] for i in range(4) for j in range(4)},21:.125,22:-.3125,23:.625,24:-.75}
        for i,v in values.items():p[56+12+i]=word(v);p[56+76+2*i:56+78+2*i]=struct.unpack('<2I',struct.pack('<d',v))
        pp=out/(str(scale)+'.program');pp.write_bytes(struct.pack('<2048I',*p));reports=[]
        for name,package,policy in [('frozen',base,None),('generic',candidate,0),('fixed',candidate,1)]:
            d=out/(str(scale)+'-'+name);args=[package/'build/mb3d_native_formula_probe','--program',pp,'--states',sp,'--library',package/'build/metal_native_scene.metallib','--output',d/'report.json','--mode','step','--slot',0,'--repeats',3]
            if policy is not None:args+=['--wrapper-policy',policy]
            e=invoke(args,d,package/'runtime',60);require(e['exit_code'] in (0,2) and not e['guard'],'Holdout process failed');reports.append(read(d/'report.json'))
        require(reports[0]['rows']==reports[1]['rows']==reports[2]['rows'],'Dense rotation changed59-word results')
        rows.append({'scale':scale,'inputs':len(states),'exact':True})
    write(out/'qualification.json',{'status':'PASS_DENSE_ROTATION_HOLDOUT','rows':rows,'matrix':matrix,'scope':'Sierpinski4ex step only; independently suggested after fitting'})
if __name__=='__main__':
    p=argparse.ArgumentParser()
    for k in ('baseline','candidate','evidence','output'):p.add_argument('--'+k,type=Path,required=True)
    a=p.parse_args();run(a.baseline.resolve(),a.candidate.resolve(),a.evidence.resolve(),a.output.resolve())
