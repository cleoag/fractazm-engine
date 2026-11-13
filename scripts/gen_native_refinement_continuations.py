#!/usr/bin/env python3
"""Source-bound iteration refinement; no arithmetic rewriting or renderer admission."""
import hashlib,re
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_entry_continuations import GEOMETRY_SHA


def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=GEOMETRY_SHA:raise ValueError('Geometry source changed; repeat P1 caller audit')
 v=function(source,'general_refine_iteration_count')
 loop=locate(v,tokens('for(uint pass=0;pass<=refinement_steps;++pass)'))
 opening=v.index('{',loop);body=v[opening+1:matching(v,opening)]
 call1=locate(body,tokens('auto value=general_de('));end1=body.index(';',call1)+1
 branch=locate(body,tokens('if(smooth>float(temporary_max)-.1f)'))
 alternate=body.index('else',branch);opening=alternate+1;closing=matching(body,opening)
 call2=end1+locate(body[end1:],tokens('value=general_de('));end2=body.index(';',call2)+1
 return dict(prefix=v[:loop],move_first=body[:call1],call_first=body[call1:end1],
  after_first=body[end1:branch],high=body[branch:alternate],move_second=body[opening+1:call2],
  call_second=body[call2:end2],after_second=body[end2:closing],finish=body[closing+1:])


def generate_refinement(source):
 r=regions(source)
 def text(name):return emit_text(r[name]).replace('return false;','return refinement_phase(n,99,point);')
 prefix=text('prefix')
 for declaration in ['const float target','const int temporary_max','float step','bool first']:
  prefix=prefix.replace(declaration+' =',declaration.split()[-1]+' =')
 # The source's comma declarations become assignments to distinct live fields.
 prefix=prefix.replace(', multiplier =','; multiplier =').replace(', last =','; last =').replace(', last_difference =','; last_difference =').replace(', smooth =','; smooth =')
 return f'''namespace mb3d_native {{ namespace continuation {{
inline uint refinement_phase(thread RefinementFrame& n,uint pc,float3 point){{
 for(uint k=0;k<3;++k)n.point[k]=point[k];n.header.pc=pc;return pc;
}}
inline void refinement_child(thread DistanceEntryFrame& e,float3 point,float stop,int maximum,uint parent_pc){{
 e={{}};e.header.parent_tag=uint(Tag::Refinement);e.header.parent_pc=parent_pc;
 for(uint k=0;k<3;++k)e.point[k]=point[k];e.stop=stop;e.smooth=1;e.max_iterations=maximum;e.temporary_maximum=1;
}}
inline bool refinement_success(thread const RefinementFrame& n,thread const NativeGeometryRecord& record){{return n.header.pc==99&&!record.fault;}}
inline uint refinement_advance(thread RefinementFrame& n,thread DistanceEntryFrame& e,
 thread CombinedFrame& c,thread DistanceFrame& d,thread OrbitReplayFrame& r,thread OrbitPassFrame& o,
 thread FormulaStepFrame& step_frame,thread WrapperFrame& w,thread BodyFrame& b,
 constant ExactScene& scene,constant NativeFormulaProgram& program,thread NativeResources& resources,
 thread NativeFormulaState& lane,thread NativeGeometryRecord& record){{
 float3 point(n.point[0],n.point[1],n.point[2]);const float3 ray(n.ray[0],n.ray[1],n.ray[2]);const float stop=n.stop;
 thread auto& depth=n.depth;thread auto& target=n.target;thread auto& temporary_max=n.temporary_max;
 thread auto& step=n.step;thread auto& multiplier=n.multiplier;thread auto& last=n.last;
 thread auto& last_difference=n.last_difference;thread auto& smooth=n.smooth;thread auto& first=n.first;
 thread auto& pass=n.pass;thread auto& refinement_steps=n.refinement_steps;thread auto& iteration=n.iteration;
 if(n.header.pc==0){{
  n.header.version=LayoutVersion;n.header.tag=uint(Tag::Refinement);
  {prefix}
  pass=0;return refinement_phase(n,1,point);
 }}
 if(n.header.pc==1){{
  if(pass>refinement_steps)return refinement_phase(n,99,point);
  {text('move_first')}
  refinement_child(e,point,stop,temporary_max,2);return refinement_phase(n,2,point);
 }}
 if(n.header.pc==2||n.header.pc==4){{
  // Context 5 is reserved for the iteration-refinement caller.
  if(entry_advance<5>(e,c,d,r,o,step_frame,w,b,program,resources,lane,record)!=99)return n.header.pc;
  n.child=e.child;return refinement_phase(n,n.header.pc==2?3:5,point);
 }}
 if(n.header.pc==3){{
  const auto value=n.child;{text('after_first')}
  {text('high')}else{{
   {text('move_second')}
   refinement_child(e,point,stop,temporary_max,4);return refinement_phase(n,4,point);
  }}
  return refinement_phase(n,6,point);
 }}
 if(n.header.pc==5){{
  const auto value=n.child;{text('after_second')}
  return refinement_phase(n,6,point);
 }}
 if(n.header.pc==6){{
  {text('finish')}
  ++pass;return refinement_phase(n,1,point);
 }}
 return n.header.pc;
}}
}} }}
'''

if __name__=='__main__':
 import argparse
 from pathlib import Path
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-root',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 out=a.output.resolve();production=Path(__file__).resolve().parents[1]/'src/render/native_metal/generated'
 if out==production or out.is_relative_to(production):ap.error('Separate experimental output required')
 out.mkdir(parents=True,exist_ok=True)
 (out/'refinement_regions.metal.h').write_text(generate_refinement((a.source_root/'src/render/native_metal/general_geometry.metal.h').read_text()))
