#!/usr/bin/env python3
"""Test-only copies of live primary caller state around six child boundaries."""
from gen_native_primary_continuations import generate_primary,regions
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate

# Initial DE precedes declaration of de/iteration/maxima; march-only locals
# cease to exist before normal packing. Inactive fields are canonical zeroes.
def capture(site,phase):
 scalar='axial_multiplier base_stop stop_factor z_div limiter depth stop count slope final_de axial'.split()
 scalar+=['de' if site!=1 else '0.f']
 scalar+=['last_step' if site in [2,3,4,5] else '0.f','last_de' if site in [2,3,4,5] else '0.f','step' if site==2 else '0.f','0.f']
 ints=[name if site!=1 else '0u' for name in ['iteration','program_maximum','combined_maximum']]
 ints+=['step_index' if site in [2,3,4,5] else '0u','uint(ended)' if site in [2,3,4,5] else '0u','uint(de_limited)' if site in [4,5] else '0u','0u','0u']
 mask=1|(2 if site!=1 else 0)|(4 if site in [2,3,4,5]else 0)|(8 if site==2 else 0)|(16 if site in [4,5]else 0)
 child='result' if site in [1,2,3]and phase==2 else 'NativeOrbitResult{}'
 return f'observer.record({site}u,{phase}u,{mask}u,ray,start,point,active,'+','.join(scalar+ints)+f',out,lane,record,normal_record,normal_result,{child});'

def original_observed(source):
 regions(source);v=function(source,'general_primary_camera_attempt');patches=[]
 calls=[]
 needle=tokens('result=general_de(')
 for i in range(len(v)-len(needle)+1):
  if v[i:i+len(needle)]==needle:calls.append(i)
 if len(calls)!=4:raise ValueError('Primary DE sites changed')
 for site,i in enumerate(calls[:3],1):
  end=v.index(';',i)+1;before=tokens(capture(site,1));after=tokens(capture(site,2));replacement=before+v[i:end]+after
  if site==1:replacement=['{']+replacement+['}']
  patches.append((i,end,replacement))
 loop=tokens('for(uint remaining=scene.refinement_steps;remaining>0&&abs(de-stop)>.001f;--remaining)');i=locate(v,loop);opening=i+len(loop);end=matching(v,opening)+1
 begin=i-len(tokens('float step=last_step*-.5f;'));patches.append((begin,end,tokens(capture(4,1))+v[begin:end]+tokens(capture(4,2))))
 i=locate(v,tokens('else if(!general_refine_iteration('));call=v.index('general_refine_iteration',i);end=matching(v,call+1,'(',')');assert v[end+1:end+4]==[')','break',';']
 replacement=tokens('else{'+capture(5,1)+'const bool primary_refined=')+v[call:end+1]+tokens(';'+capture(5,2)+'if(!primary_refined)break;}')
 patches.append((i,end+4,replacement))
 i=locate(v,tokens('normal_result=general_calculate_normals('));end=v.index(';',i)+1;patches.append((i,end,tokens(capture(6,1))+v[i:end]+tokens(capture(6,2))))
 for a,b,replacement in sorted(patches,reverse=True):v[a:b]=replacement
 return emit_text(v)

def candidate_observed(source):
 code=generate_primary(source)
 def replace(old,new):
  nonlocal code
  if code.count(old)!=1:raise ValueError('Missing/ambiguous primary trace binding: '+old)
  code=code.replace(old,new)
 replace('inline uint primary_advance(', 'template<class Observer> inline uint primary_advance(')
 replace('thread NativeResources& resources){', 'thread NativeResources& resources,thread Observer& observer){')
 for site,pc in [(1,1),(2,11),(3,13)]:
  replace(f'primary_entry_child(f,point,stop,{pc});',capture(site,1)+f'primary_entry_child(f,point,stop,{pc});')
 replace('if(n.header.pc==1)return primary_phase(f,2,point,active);','if(n.header.pc==1){'+capture(1,2)+'return primary_phase(f,2,point,active);}')
 replace('if(n.header.pc==11){', 'if(n.header.pc==11){'+capture(2,2))
 anchor='return primary_phase(f,12,point,active);}\n  '
 replace(anchor,anchor+capture(3,2))
 replace('if(de_limited)return primary_phase(f,40,point,active);','if(de_limited){'+capture(4,1)+'return primary_phase(f,40,point,active);}')
 replace('f.refinement={};',capture(5,1)+'f.refinement={};')
 anchor='depth=f.refinement.depth;iteration=f.refinement.iteration;'
 replace(anchor,anchor+capture(5,2))
 replace('if(n.header.pc==49)return primary_phase(f,30,point,active);','if(n.header.pc==49){'+capture(4,2)+'return primary_phase(f,30,point,active);}')
 replace('f.normal={};',capture(6,1)+'f.normal={};')
 replace('normal_result=f.normal.result;', 'normal_result=f.normal.result;'+capture(6,2))
 return code
