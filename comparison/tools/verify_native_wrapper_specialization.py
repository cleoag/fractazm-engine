"""Bounded real-device differential controls; not all-formula qualification."""
import argparse,random,struct
from pathlib import Path
from run_m3parameter_gpu import read,write,invoke,require,sha
ROOT=Path(__file__).resolve().parents[2]
def word(x):return struct.unpack('<I',struct.pack('<f',x))[0]
def run(base,candidate,out):
    require(not out.exists(),'Fresh output required');out.mkdir(parents=True)
    scenes={s['id']:s for s in read(candidate/'contract.json')['scenes']};rows=[]
    def probe(package,program,states,mode,slot,folder,policy=None):
        argv=[package/'build/mb3d_native_formula_probe','--program',program,'--states',states,
            '--library',package/'build/metal_native_scene.metallib','--output',folder/'report.json',
            '--mode',mode,'--slot',slot,'--repeats',3]
        if policy is not None:argv+=['--wrapper-policy',policy]
        e=invoke(argv,folder,package/'runtime',60)
        require(e['exit_code'] in (0,2) and not e['guard'],'Probe failed')
        return read(folder/'report.json')
    for sid in ('0f258a2abc1002ec','77923ebd629a2fb2'):
        seed=out/sid/'seed';scene=scenes[sid]
        e=invoke([base/'build/mb3d_native_formula_probe','--scene',base/scene['frozen'],
          '--resource-root',base/'runtime','--library',base/'build/metal_native_scene.metallib',
          '--output',seed/'report.json','--mode','step','--slot',0,'--repeats',3],seed,base/'runtime',60)
        require(e['exit_code']==0 and not e['guard'],'Seed probe failed');original=read(seed/'report.json')
        rng=random.Random(1847);states=[]
        for i in range(128):
            s=list(original['rows'][i%7]['input_words']);s[:33]=[word(rng.uniform(-2,2)) for _ in range(33)]
            s[26]=word(1000);s[27]=word(.01);s[33]=i%6;s[34]=(0,1,0x1000001,0x7fffffff,0x80000000,0xffffffff)[i%6]
            s[35]=0;s[36]=8;s[43]=i%2;s[44]=0;s[45]=i%2;s[46]=0
            if i%8==0:s[1]=0x80000000 if i%16==0 else 0
            if i==1:s[:4]=[0]*4
            states.append(s)
        sp=out/sid/'states.bin';sp.write_bytes(b''.join(struct.pack('<47I',*s) for s in states))
        for slot in range(len(scene['slots'])):
            offset=56+332*slot;ident=original['program_words'][offset+2]
            variants=['nominal','negative-scale','count','traits','option','flags','style','count59','local0','local-overflow']
            for variant in variants:
                program=list(original['program_words']);program[11]=4096
                if variant=='negative-scale':program[offset+12]=word(-1.5)
                if variant=='count':program[offset+4]-=1
                if variant=='traits':program[offset+5]^=2
                if variant=='option':program[offset+3]=2
                if variant=='flags':program[offset+6]^=1
                if variant=='style':program[offset+7]=1
                if variant=='count59':program[offset+4]=59
                if variant=='local0':program[12]=0
                if variant=='local-overflow':program[12]=(1<<20)+1
                pp=out/sid/(str(slot)+'-'+variant+'.program');pp.write_bytes(struct.pack('<2048I',*program))
                modes=['step','orbit','distance'] if variant=='nominal' else ['step']
                for mode in modes:
                    folder=out/sid/(str(slot)+'-'+variant+'-'+mode)
                    a=probe(base,pp,sp,mode,slot,folder/'frozen')
                    b=probe(candidate,pp,sp,mode,slot,folder/'generic',0)
                    c=probe(candidate,pp,sp,mode,slot,folder/'fixed',1)
                    require(a['rows']==b['rows']==c['rows'],f'59-word result difference {sid} {slot} {variant} {mode}')
                    # This is actual GPU eligibility from the helper used by
                    # the execution branch, separate from arithmetic records.
                    require(b['wrapper_audit_words'][slot*3:slot*3+3]==[1,0,0],'Generic policy not observed')
                    eligible=ident if ident in (9401,10350,5823) and variant in ('nominal','negative-scale','local0','local-overflow') else 0
                    require(c['wrapper_audit_words'][slot*3:slot*3+3]==[1,1,eligible],'Fixed eligibility not observed')
                    if variant in ('count59','local0','local-overflow'):
                        require(all(x['fault']==1 and x['work']==0 for x in c['rows']),'Invalid descriptor performed formula work')
                    rows.append({'scene':sid,'formula_id':ident,'slot':slot,'variant':variant,'mode':mode,
                      'inputs':128,'repeats':3,'compared_words':59,'faults':c['fault_count'],
                      'program_sha256':sha(pp),'states_sha256':sha(sp),'folder':str(folder)})
                    write(out/'partial.json',rows);print('EXACT',ident,variant,mode,flush=True)
    write(out/'qualification.json',{'status':'PASS_SCOPED_FULL_STATE','cases':rows,
      'comparisons':len(rows)*128,'word_comparisons':len(rows)*128*59,
      'scope':'four audited formulas; three diagnostic fixed metadata paths and generic MixPinski4; production excludes fixed Makin; not full467'})
if __name__=='__main__':
    p=argparse.ArgumentParser()
    for k in ('baseline','candidate','output'):p.add_argument('--'+k,type=Path,required=True)
    a=p.parse_args();run(a.baseline.resolve(),a.candidate.resolve(),a.output.resolve())
