#!/usr/bin/env python3
"""Extract the P1 primary DE-refinement block, retaining every numeric expression."""
import hashlib
from gen_native_orbit_continuations import function
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_entry_continuations import GEOMETRY_SHA


def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=GEOMETRY_SHA:raise ValueError('Geometry source changed; repeat P1 caller audit')
 v=function(source,'general_primary_camera_attempt')
 loop=tokens('for(uint remaining=scene.refinement_steps;remaining>0&&abs(de-stop)>.001f;--remaining)')
 i=locate(v,loop);init=tokens('float step=last_step*-.5f;')
 if v[i-len(init):i]!=init:raise ValueError('DE refinement initializer changed')
 opening=i+len(loop);body=v[opening+1:matching(v,opening)]
 call=locate(body,tokens('result=general_de('));end=body.index(';',call)+1
 return dict(init=init,before=body[:call],call=body[call:end],after=body[end:],loop=loop)


def generate_de_refinement(source):
 r=regions(source);before=emit_text(r['before']).replace('break;', 'return de_refinement_phase(n,49,point);')
 after=emit_text(r['after']).replace('break;', 'return de_refinement_phase(n,49,point);')
 init=emit_text(r['init']).replace('float step =','step =')
 return f'''namespace mb3d_native {{ namespace continuation {{
// PCs 40..49 are the DE-refinement subregion of the primary frame, not a new stack frame.
inline uint de_refinement_phase(thread PrimaryFrame& n,uint pc,float3 point){{for(uint k=0;k<3;++k)n.point[k]=point[k];n.header.pc=pc;return pc;}}
inline void de_refinement_child(thread DistanceEntryFrame& e,float3 point,float stop){{
 e={{}};e.header.parent_tag=uint(Tag::Primary);e.header.parent_pc=42;
 for(uint k=0;k<3;++k)e.point[k]=point[k];e.stop=stop;
}}
inline uint de_refinement_advance(thread PrimaryFrame& n,thread DistanceEntryFrame& e,
 thread CombinedFrame& c,thread DistanceFrame& d,thread OrbitReplayFrame& r,thread OrbitPassFrame& o,
 thread FormulaStepFrame& step_frame,thread WrapperFrame& w,thread BodyFrame& b,
 constant ExactScene& scene,constant NativeFormulaProgram& program,thread NativeResources& resources,
 thread NativeFormulaState& lane,thread NativeGeometryRecord& record){{
 float3 point(n.point[0],n.point[1],n.point[2]);const float3 ray(n.ray[0],n.ray[1],n.ray[2]);
 thread auto& depth=n.depth;thread auto& stop=n.stop;thread auto& de=n.de;thread auto& iteration=n.iteration;
 const float base_stop=n.base_stop,stop_factor=n.stop_factor,last_step=n.last_step;
 thread auto& step=n.refine_step;thread auto& remaining=n.remaining;
 if(n.header.pc==40){{
  n.header.version=LayoutVersion;n.header.tag=uint(Tag::Primary);
  {init}remaining=scene.refinement_steps;return de_refinement_phase(n,41,point);
 }}
 if(n.header.pc==41){{
  if(!(remaining>0&&abs(de-stop)>.001f))return de_refinement_phase(n,49,point);
  {before}
  de_refinement_child(e,point,stop);return de_refinement_phase(n,42,point);
 }}
 if(n.header.pc==42){{
  // Context 6 is the primary DE-refinement call site.
  if(entry_advance<6>(e,c,d,r,o,step_frame,w,b,program,resources,lane,record)!=99)return 42;
  n.child=e.child;return de_refinement_phase(n,43,point);
 }}
 if(n.header.pc==43){{
  const auto result=n.child;{after}
  --remaining;return de_refinement_phase(n,41,point);
 }}
 return n.header.pc;
}}
}} }}
'''


def generate_control(source):
 r=regions(source)
 signature='thread float3& point,float3 ray,thread float& depth,thread float& stop,thread float& de,float last_step,float base_stop,float stop_factor,thread uint& iteration,constant ExactScene& scene,constant NativeFormulaProgram& program,thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record'
 body=emit_text(r['init']+r['loop']+['{']+r['before']+r['call']+r['after']+['}'])
 return 'inline void de_refinement_control('+signature+'){NativeOrbitResult result{};'+body+'}\n'

if __name__=='__main__':
 import argparse
 from pathlib import Path
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-root',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();out=a.output.resolve();production=Path(__file__).resolve().parents[1]/'src/render/native_metal/generated'
 if out==production or out.is_relative_to(production):ap.error('Separate experimental output required')
 source=(a.source_root/'src/render/native_metal/general_geometry.metal.h').read_text();out.mkdir(parents=True,exist_ok=True)
 (out/'de_refinement_regions.metal.h').write_text(generate_de_refinement(source));(out/'de_refinement_control.metal.h').write_text(generate_control(source))
