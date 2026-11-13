"""Poison retained state and compare observable orbit results on the real GPU."""
import argparse,struct
from pathlib import Path
from run_m3parameter_gpu import read,write,require,invoke,sha
IDS=['a203b667b15f1e47','5f2747de7764cf5f']
def run(p,out):
 out.mkdir();scenes={s['id']:s for s in read(p/'contract.json')['scenes']};results=[]
 def call(args,dest):
  event=invoke([p/'build/mb3d_native_formula_probe',*args,'--library',p/'build/metal_native_scene.metallib','--mode','distance','--repeats','3','--output',dest/'result.json'],dest,p/'runtime',180)
  require(event['exit_code']==0 and not event['guard'],'Orbit probe failed');v=read(dest/'result.json');require(v['fault_count']==0 and v['exact_state_repeats'],'Orbit fault/repeat');return v
 for sid in IDS:
  d=out/sid;d.mkdir();base=call(['--scene',p/scenes[sid]['frozen'],'--resource-root',p/'runtime'],d/'baseline')
  require(base['formula_abi']==[1,8192,188,236,32],'Wrong formula ABI')
  (d/'program.bin').write_bytes(struct.pack('<2048I',*base['program_words']));states=[]
  # xyz, j.xyz, radius/trap/first/tmp are reset. Other poisoned lanes are
  # retained but proven unused by these chains; their output words may differ.
  poisoned=[*range(4),*range(8,12),*range(16,26),28,29,30,31,33,34,35]
  for row in base['rows']:
   for salt in range(1,6):
    words=list(row['input_words'])
    for i in poisoned:
     words[i]=struct.unpack('<I',struct.pack('<f',(-1 if salt%2 else 1)*(1000*salt+i)))[0] if i<33 else 1234567*salt
    states.extend(words)
  (d/'states.bin').write_bytes(struct.pack('<'+'I'*len(states),*states));v=call(['--program',d/'program.bin','--states',d/'states.bin'],d/'poisoned')
  compared=[*range(7),*range(8,11),*range(24,30),*range(32,59)]
  for i,row in enumerate(v['rows']):
   expected=base['rows'][i//5]['result_words'];actual=row['result_words']
   require(all(actual[k]==expected[k] for k in compared),'Retained state affected observable orbit '+sid)
  results.append({'id':sid,'baseline_points':len(base['rows']),'poisoned_states':len(v['rows']),'exact_repeats':3,'compared_word_indices':compared,'retained_unused_words_not_compared':[7,11,*range(12,24),30,31],'baseline':str(d/'baseline/result.json'),'poisoned':str(d/'poisoned/result.json')});print('STATE INDEPENDENCE PASS',sid,flush=True)
 write(out/'result.json',{'status':'PASS','method':'Source-audited state invariance on real device; not an independent Pascal oracle','rows':results,'runner_sha256':sha(__file__)})
if __name__=='__main__':
 a=argparse.ArgumentParser();a.add_argument('--package',type=Path,required=True);a.add_argument('--output',type=Path,required=True);v=a.parse_args();run(v.package.resolve(),v.output.resolve())
