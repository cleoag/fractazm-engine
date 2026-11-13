#!/usr/bin/env python3
"""Source-bound primary camera attempt for the audited P1 preview capability."""
import hashlib,re
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import function
from gen_native_entry_continuations import GEOMETRY_SHA

def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=GEOMETRY_SHA:raise ValueError('Geometry source changed; repeat P1 primary audit')
 v=function(source,'general_primary_camera_attempt')
 def at(v,s):return locate(v,tokens(s))
 def block(v,i):
  a=v.index('{',i);b=matching(v,a);return v[a+1:b],a,b
 initial=at(v,'if(!camera.cut_miss)');initial_end=v.index(';',initial)+1
 hit=at(v,'if(iteration>=general_maximum(combined_maximum,program_maximum)||de<stop)')
 inside,a,b=block(v,hit);march,ma,mb=block(v,b+1)
 loop=at(march,'for(uint step_index=0;step_index<scene.max_steps;++step_index)');body,la,lb=block(march,loop)
 backup=at(body,'if(iteration>=general_maximum(combined_maximum,program_maximum))');back,ba,bb=block(body,backup)
 moving=at(body,'if(iteration<scene.min_iterations||');move,xa,xb=block(body,moving);contact,ca,cb=block(body,xb+1)
 def split_call(v):
  a=at(v,'result=general_de(');b=v.index(';',a)+1;return v[:a],v[a:b],v[b:]
 back_pre,back_call,back_post=split_call(back);move_pre,move_call,move_post=split_call(move)
 refine=at(contact,'if(scene.refinement_steps)');_,ra,rb=block(contact,refine)
 remainder=v[mb+1:];normal=at(remainder,'if(out.rm_result&&!out.start_inside&&!record.fault)');normal_body,na,nb=block(remainder,normal)
 ncall=at(normal_body,'normal_result=general_calculate_normals(');nend=normal_body.index(';',ncall)+1
 return dict(prefix=v[:initial],initial=v[initial:initial_end],after_initial=v[initial_end:hit],inside=inside,march_init=march[:loop],
  loop_begin=body[:backup],backup_before=back_pre,backup_call=back_call,backup_after=back_post,
  move_condition=body[moving:xa],move_before=move_pre,move_call=move_call,move_after=move_post,
  contact_before=contact[:refine],contact_after=contact[rb+1:],after_loop=march[lb+1:],
  pack=remainder[:normal],normal_condition=remainder[normal:na],normal_call=normal_body[ncall:nend],normal_after=normal_body[nend:],finish=remainder[nb+1:])

def generate_primary(source):
 r=regions(source)
 def text(name):
  return emit_text(r[name]).replace('return out;', 'return primary_phase(f,99,point,active);').replace('break;', 'return primary_phase(f,60,point,active);')
 prefix=text('prefix').replace('ExactPixel out {\n}\n;', 'out={};')
 # Declarations become assignments to their existing typed live storage.
 for declaration in ['const float axial_multiplier','const float base_stop','const float z_div','float depth','float stop','float3 point','float count']:
  prefix=prefix.replace(declaration+' =',declaration.split()[-1]+' =')
 prefix=re.sub(r'ExactPixel out\s*\{\s*\}\s*;', 'out={};',prefix)
 prefix=re.sub(r'NativeOrbitResult result\s*\{\s*\}\s*;', 'result={};',prefix)
 for name in ['ray','start']:
  pattern=r'const float3 '+name+r'\s*\([^;]+;';prefix,n=re.subn(pattern,'',prefix);assert n==1
 # Reject unaudited capabilities before the first numerical child. Inputs are
 # immutable, so the admission remains true throughout this attempt.
 prefix=prefix.replace('out={};', 'out={};if(normal_params.smoothing||scene.color_on_it||scene.fog_iteration==kNativeVolumeFog||native_primary_random()){record.fault=out.fault=1;return primary_phase(f,99,point,active);}')
 after=text('after_initial')
 for declaration in ['float de','uint iteration','const uint program_maximum','uint combined_maximum']:
  after=after.replace(declaration+' =',declaration.split()[-1]+' =')
 # The original volume validation remains in after_initial; volume is refused
 # before dispatch/entry in this capability and the immutable map predicate is false.
 after=after.replace('const bool volume_map = volume && resources . volume -> side_count;', '')
 march=text('march_init').replace('float last_step =','last_step =').replace('bool ended =','ended =')
 backup=text('backup_before').replace('const float step =','step =')
 contact=text('contact_before').replace('const bool de_limited =','de_limited =')
 scalar='axial_multiplier base_stop stop_factor z_div limiter depth stop count slope final_de axial de last_step last_de iteration program_maximum combined_maximum step_index remaining ended de_limited'.split()
 aliases='\n'.join(f'thread auto& {name}=n.{name};'for name in scalar)
 args='f.de,f.combined,f.distance,f.replay,f.orbit,f.step,f.wrapper,f.body,program,resources,lane,record'
 return f'''namespace mb3d_native {{ namespace continuation {{
inline uint primary_phase(thread RayFrame& f,uint pc,float3 point,float3 active){{
 for(uint k=0;k<3;++k){{f.primary.point[k]=point[k];f.primary.active[k]=active[k];}}f.primary.header.pc=pc;return pc;
}}
inline void primary_entry_child(thread RayFrame& f,float3 point,float stop,uint pc){{
 f.de={{}};f.de.header.parent_tag=uint(Tag::Primary);f.de.header.parent_pc=pc;
 for(uint k=0;k<3;++k)f.de.point[k]=point[k];f.de.stop=stop;
}}
inline uint primary_advance(thread RayFrame& f,constant ExactScene& scene,constant NativeFormulaProgram& program,thread NativeResources& resources){{
 thread auto& n=f.primary;thread auto& lane=f.lane;thread auto& record=f.geometry;thread auto& out=f.pixel;
 thread auto& normal_record=f.normal_record;thread auto& normal_result=f.normal_result;thread auto& result=n.child;
 const auto camera=f.camera;const uint x=f.owner.x,y=f.owner.y;const int inside_word=n.inside_word;
 auto normal_params=f.normal_params;normal_params.inside=uint(inside_word!=0);
 const float3 ray(camera.direction[0],camera.direction[1],camera.direction[2]),start(camera.origin[0],camera.origin[1],camera.origin[2]);
 float3 point=n.header.pc?float3(n.point[0],n.point[1],n.point[2]):float3(0.f);
 float3 active=n.header.pc?float3(n.active[0],n.active[1],n.active[2]):float3(0.f);
 thread auto& step=n.refine_step;
 thread PrimaryRandomState* random=nullptr;const bool volume_map=false;
 {aliases}
 if(n.header.pc==0){{
  n.header.version=LayoutVersion;n.header.tag=uint(Tag::Primary);
  {prefix}
  for(uint k=0;k<3;++k){{n.ray[k]=ray[k];n.start[k]=start[k];}}
  if(!camera.cut_miss){{primary_entry_child(f,point,stop,1);return primary_phase(f,1,point,active);}}
  return primary_phase(f,2,point,active);
 }}
 if(n.header.pc==1||n.header.pc==11||n.header.pc==13){{
  if(entry_advance<7>({args})!=99)return n.header.pc;
  result=f.de.child;
  if(n.header.pc==1)return primary_phase(f,2,point,active);
  if(n.header.pc==11){{{text('backup_after')}return primary_phase(f,12,point,active);}}
  {text('move_after')}++step_index;return primary_phase(f,10,point,active);
 }}
 if(n.header.pc==2){{
  {after}
  if(iteration>=general_maximum(combined_maximum,program_maximum)||de<stop){{{text('inside')}return primary_phase(f,70,point,active);}}
  {march}step_index=0;return primary_phase(f,10,point,active);
 }}
 if(n.header.pc==10){{
  if(!(step_index<scene.max_steps))return primary_phase(f,60,point,active);
  {text('loop_begin')}
  if(iteration>=general_maximum(combined_maximum,program_maximum)){{{backup}primary_entry_child(f,point,stop,11);return primary_phase(f,11,point,active);}}
  return primary_phase(f,12,point,active);
 }}
 if(n.header.pc==12){{
  {text('move_condition')}{{{text('move_before')}primary_entry_child(f,point,stop,13);return primary_phase(f,13,point,active);}}
  {contact}
  if(scene.refinement_steps){{
   if(de_limited)return primary_phase(f,40,point,active);
   f.refinement={{}};f.refinement.header.parent_tag=uint(Tag::Primary);f.refinement.header.parent_pc=20;
   for(uint k=0;k<3;++k){{f.refinement.point[k]=point[k];f.refinement.ray[k]=ray[k];}}
   f.refinement.depth=depth;f.refinement.stop=stop;f.refinement.iteration=iteration;f.refinement.refinement_steps=scene.refinement_steps;
   return primary_phase(f,20,point,active);
  }}
  return primary_phase(f,30,point,active);
 }}
 if(n.header.pc==20){{
  if(refinement_advance(f.refinement,{args.replace('program,resources','scene,program,resources')})!=99)return 20;
  point=float3(f.refinement.point[0],f.refinement.point[1],f.refinement.point[2]);depth=f.refinement.depth;iteration=f.refinement.iteration;
  if(!refinement_success(f.refinement,record))return primary_phase(f,60,point,active);
  return primary_phase(f,30,point,active);
 }}
 if(n.header.pc>=40&&n.header.pc<=43)return de_refinement_advance(n,{args.replace('program,resources','scene,program,resources')});
 if(n.header.pc==49)return primary_phase(f,30,point,active);
 if(n.header.pc==30){{{text('contact_after')}}}
 if(n.header.pc==60){{{text('after_loop')}return primary_phase(f,70,point,active);}}
 if(n.header.pc==70){{
  {text('pack')}
  {text('normal_condition')}{{
   f.normal={{}};f.normal.header.parent_tag=uint(Tag::Primary);f.normal.header.parent_pc=80;
   for(uint k=0;k<3;++k)f.normal.center[k]=active[k];f.normal.depth=depth;f.normal.stop=stop;
   return primary_phase(f,80,point,active);
  }}
  return primary_phase(f,90,point,active);
 }}
 if(n.header.pc==80){{
  if(normal_advance(f.normal,f.sample,{args.replace('program,resources','scene,program,normal_params,resources')},normal_record)!=99)return 80;
  normal_result=f.normal.result;{text('normal_after')}
  return primary_phase(f,90,point,active);
 }}
 if(n.header.pc==90){{{text('finish')}}}
 return n.header.pc;
}}
}} }}
'''

if __name__=='__main__':
 import argparse
 from pathlib import Path
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-root',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();out=a.output.resolve();production=Path(__file__).resolve().parents[1]/'src/render/native_metal/generated'
 if out==production or out.is_relative_to(production):ap.error('Separate experimental output required')
 out.mkdir(parents=True,exist_ok=True);source=(a.source_root/'src/render/native_metal/general_geometry.metal.h').read_text()
 (out/'primary_regions.metal.h').write_text(generate_primary(source))
