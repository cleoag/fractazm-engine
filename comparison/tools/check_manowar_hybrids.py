"""Discriminating real-driver Manowar orbits with independent Pascal buffers.

Finite FPC i386 SSE2 caller qualification only. Generic point status is unchanged.
Uses original doHybridPas/doInterpolHybridPas, original CODE, native parameters;
plain/trace/repeat results must agree. Other formulas' caller lanes are not an ABI.
"""
import argparse,hashlib,json,math,os,subprocess
from pathlib import Path
from pascal_formula_identity import validate_formula_load
ROOT=Path(__file__).resolve().parents[2]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def equal(x,y):return x==y or math.isnan(x)and math.isnan(y)
def near(x,y):return equal(x,y)or math.isfinite(x)and math.isfinite(y)and abs(x-y)<=2e-12*max(1,abs(y))
def run(argv,stdin,base,env=None,cwd=None):
    r=subprocess.run(list(map(str,argv)),input=stdin.encode(),capture_output=True,timeout=45,env=env,cwd=cwd,creationflags=subprocess.CREATE_NO_WINDOW)
    base.with_suffix('.stdout').write_bytes(r.stdout);base.with_suffix('.stderr').write_bytes(r.stderr)
    base.with_suffix('.command.json').write_text(json.dumps({'argv':list(map(str,argv)),'exit_code':r.returncode,'cwd':str(cwd)if cwd else None,'environment_overrides':{k:v for k,v in(env or{}).items()if k=='MB3D_FORMULA_BUFFER_DUMP'}},indent=2))
    r.check_returncode();return r
def rows(result):return [[math.nan if 'nan'in x.lower()else float(x)for x in l.split()]for l in result.stdout.decode().splitlines()]
def validate_rows(parsed,count):
    for name,data in parsed.items():
        if len(data)!=count:raise ValueError(name+': wrong row count')
        for row in data:
            if len(row)<15:raise ValueError(name+': truncated state')
            calls=row[14]
            if not math.isfinite(calls)or calls!=int(calls)or not 0<=calls<=128:raise ValueError(name+': invalid call count')
            expected=15+5*int(calls)
            if len(row)!=expected:raise ValueError(name+': wrong row length')
            if name.endswith('_plain')and calls!=0:raise ValueError(name+': unexpected plain captures')
            for i in range(int(calls)):
                if row[15+i*5]not in(0,1):raise ValueError(name+': invalid slot')
def incoming_mismatches(cpp,pas,names):
    if cpp[14]!=pas[14]:return ['call_count']
    bad=[]
    for i in range(int(pas[14])):
        j=15+i*5
        if cpp[j]!=pas[j]:bad.append('slot_order')
        elif names[int(pas[j])]=='ManowarBulb'and not all(near(v,w)for v,w in zip(cpp[j+1:j+5],pas[j+1:j+5])):
            bad.append('manowar_incoming_lanes')
    return bad
def consumed_parameter_differences(name,pascal,cpp):
    if len(pascal)!=512 or len(cpp)!=1024:raise ValueError('invalid parameter buffer length')
    if name not in ('ManowarBulb','MsltoeSym2','_Translate','_Rotate','MengerKoch','CommQuat','ABoxMod4d'):raise ValueError('unreviewed parameter consumer')
    # Full original CODE review: all negative options; ABoxMod4d also reads
    # the Double epsilon at PVar+0. Other positive constant bytes are unused.
    differences=[j-256 for j,(x,y)in enumerate(zip(pascal,cpp[256:768]))if x!=y]
    return differences,[v for v in differences if v<0 or(name=='ABoxMod4d'and 0<=v<8)]
def main():
    ap=argparse.ArgumentParser(description=__doc__)
    for n in ('cpp','oracle','pascal-loader','manifest','output'):ap.add_argument('--'+n,type=Path,required=True)
    a=ap.parse_args();a.output.mkdir(parents=True,exist_ok=False)
    xyz=[[.125,-.25,.5],[-.4,.3,.2],[.9,-.1,.2],[0.,0.,0.],[1e-8,-1e-8,1e-8],[1.5,-.5,.25]]
    inputs=[p+[.2,-.3,.4,j,l,s]for p in xyz for j in(0,1)for l in(1,2,5,12)for s in(4.,256.)]
    stdin=''.join(' '.join(format(v,'.17g')for v in p)+'\n'for p in inputs);(a.output/'inputs.txt').write_text(stdin)
    report={'scope':__doc__,'finished':False,'rows_per_case':len(inputs),'tolerance':2e-12,'signed_zero_nan_payloads':False,'identities':{str(p.resolve()):sha(p)for p in(a.cpp,a.oracle,a.pascal_loader,a.manifest,Path(__file__))},'results':[]}
    for c in json.loads(a.manifest.read_bytes())['cases']:
        d=a.output/c['id'];d.mkdir();scene=ROOT/c['scene'];assert sha(scene)==c['sha256']
        parsed={}
        for mode in('plain','trace'):parsed['cpp_'+mode]=rows(run([a.cpp.resolve(),scene,d.resolve(),mode],stdin,d/('cpp_'+mode)))
        rt=json.loads((d/'runtime.json').read_bytes());assert (rt['driver_id']>0)==bool(c['mode'])
        # Real original app independently prepares all active slot buffers once.
        env=dict(os.environ);env['MB3D_FORMULA_BUFFER_DUMP']=str((d/'pascal').resolve())
        loaded=run([a.pascal_loader.resolve(),'--render',scene,'--output',(d/'loader.png').resolve(),'--threads','1','--width','1','--height','1'],'',d/'loader',env,a.pascal_loader.resolve().parent)
        validated=validate_formula_load(loaded.stdout.decode(errors='replace'),scene,c['slots'])
        buffers=[];loader=[];code=[]
        for i,n in enumerate(c['names']):
            p=d/f'pascal_{i}.bin';cp=d/f'params{i}.bin';pb=p.read_bytes();cb=cp.read_bytes();assert len(pb)==512 and len(cb)==1024
            dif,consumed_dif=consumed_parameter_differences(n,pb,cb)
            loader.append({'slot':i,'name':n,'status':'PASS'if not consumed_dif else'FAIL','comparison':'negative PVar options plus ABoxMod4d epsilon at [0,8); other positive constants unused by reviewed exact CODE','raw_different_offsets':dif,'different_consumed_offsets':consumed_dif,'pascal_sha256':sha(p),'cpp_sha256':sha(cp)})
            # Only original-prepared central bytes are used by the original oracle.
            buf=d/f'original_params{i}.bin';buf.write_bytes(bytes(256)+pb+bytes(256));buffers.append(buf.resolve())
            asset=ROOT/'mb3d/M3Formulas'/(n+'.m3f');b=d/f'code{i}.bin';b.write_bytes(bytes.fromhex(asset.read_text().split('[CODE]')[1].split('[END]')[0]));code.append(b.resolve())
            report['identities'][str(asset)]=sha(asset)
        command=[a.oracle.resolve(),rt['driver_id'],rt['start'],rt['repeat'],rt['end'],*rt['counts'],code[0],buffers[0],code[1],buffers[1]]
        for mode in('plain','trace','repeat'):parsed['pas_'+mode]=rows(run(command+['plain'if mode=='plain'else'trace',rt['DEoption']],stdin,d/('pas_'+mode)))
        validate_rows(parsed,len(inputs))
        failures=[];exact=0;state_bad=0;lane_bad=0;calls=0;invariant=True
        for i,(x,y)in enumerate(zip(parsed['cpp_trace'],parsed['pas_trace'])):
            assert len(x)==15+int(x[14])*5 and len(y)==15+int(y[14])*5
            calls+=int(y[14]);bad=[]
            if not all(near(v,w)if k<12 else equal(v,w)for k,(v,w)in enumerate(zip(x[:14],y[:14]))):state_bad+=1;bad.append('orbit')
            elif all(equal(v,w)for v,w in zip(x[:14],y[:14])):exact+=1
            incoming=incoming_mismatches(x,y,c['names']);bad.extend(incoming)
            lane_bad+=incoming.count('manowar_incoming_lanes')
            for lang in('cpp','pas'):
                if not all(equal(v,w)for v,w in zip(parsed[lang+'_plain'][i][:14],parsed[lang+'_trace'][i][:14])):bad.append(lang+'_instrumentation');invariant=False
            if len(parsed['pas_repeat'][i])!=len(y)or not all(equal(v,w)for v,w in zip(parsed['pas_repeat'][i],y)):bad.append('repeat');invariant=False
            if bad:failures.append({'row':i,'kinds':bad,'input':inputs[i],'cpp':list(map(str,x[:15])),'pascal':list(map(str,y[:15]))})
        assert all(len(v)==len(inputs)for v in parsed.values())
        row={'id':c['id'],'scene_sha256':sha(scene),'runtime':rt,'independent_loader':loader,'loader_identity':validated,'rows':len(inputs),'captured_calls':calls,'exact_orbits':exact,'different_orbits':state_bad,'different_manowar_incoming_lanes':lane_bad,'plain_trace_repeat_invariant':invariant,'status':'FAIL'if failures or any(r['status']!='PASS'for r in loader)else'PASS','failures':failures[:12]}
        report['results'].append(row);(a.output/'summary.json').write_text(json.dumps(report,indent=2));print(c['id'],row['status'],'orbit differences',state_bad,'lane differences',lane_bad,flush=True)
    report['finished']=True;report['artifact_sha256']={str(p.relative_to(a.output)):sha(p)for p in a.output.rglob('*')if p.is_file()and p.name!='summary.json'}
    (a.output/'summary.json').write_text(json.dumps(report,indent=2));return int(any(r['status']!='PASS'for r in report['results']))
if __name__=='__main__':raise SystemExit(main())
