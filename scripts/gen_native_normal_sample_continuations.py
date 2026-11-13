#!/usr/bin/env python3
"""Lower the selected P1 normal sample around DE/direct-orbit child calls."""
import hashlib,re
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import function
NORMAL_SHA='307cfdf25175658d03838e9ccdbd13ec5d0b280c0f97c02d85cf434d53278e2e'

def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=NORMAL_SHA:raise ValueError('Normal source changed; repeat P1 audit')
 v=function(source,'general_normal_sample');a=locate(v,['if','(','!','direct',')','{']);b=matching(v,a+5)
 direct=v[b+1:];bind=locate(direct,['thread','NativeSmoothingCounters','*','previous_smoothing']);call=locate(direct,['(','void',')','native_run_orbit']);end=direct.index(';',call)+1
 tail=locate(direct,['g','.','formula_work','+='])
 branch=v[a+6:b];de_call=locate(branch,['const','auto','value','=']);de_end=branch.index(';',de_call)+1
 return [emit_text(x) for x in (v[:a],branch[:de_call],branch[de_end:],direct[:bind],direct[bind:call],direct[end:tail],direct[tail:])]

def generate_sample(source):
 prefix,de_begin,de_finish,direct_begin,bind,restore,tail=regions(source)
 prefix=re.sub(r'GeneralNormalSample out\s*\{\s*\}\s*;','',prefix);prefix=re.sub(r'return\s+out\s*;','return false;',prefix)
 direct_begin=re.sub(r'NativeFormulaFault fault\s*\{\s*\}\s*;','fault={};',direct_begin);direct_begin=re.sub(r'return\s+out\s*;','return false;',direct_begin)
 return f'''namespace mb3d_native {{ namespace continuation {{
inline bool normal_sample_begin(float3 point,float stop,thread NativeGeometryRecord& g,thread NativeNormalRecord& n){{{prefix}return true;}}
inline bool normal_sample_direct_begin(float3 point,constant NativeFormulaProgram& p,thread NativeFormulaState& lane,
 thread NativeGeometryRecord& g,thread NativeNormalRecord& n,thread NativeFormulaFault& fault){{{direct_begin}return true;}}
inline GeneralNormalSample normal_sample_de_finish(NativeOrbitResult value,thread NativeGeometryRecord& g,thread NativeNormalRecord& n){{GeneralNormalSample out{{}};{de_finish}}}
inline GeneralNormalSample normal_sample_direct_finish(NativeFormulaFault fault,thread NativeFormulaState& lane,
 thread NativeGeometryRecord& g,thread NativeNormalRecord& n){{GeneralNormalSample out{{}};{tail}}}
inline uint normal_sample_phase(thread NormalSampleFrame& s,uint pc){{s.header.pc=pc;return pc;}}
template<bool OrderedFirst=false> inline uint normal_sample_advance(thread NormalSampleFrame& s,thread DistanceEntryFrame& e,thread CombinedFrame& c,thread DistanceFrame& d,
 thread OrbitReplayFrame& r,thread OrbitPassFrame& o,thread FormulaStepFrame& step,thread WrapperFrame& w,thread BodyFrame& b,
 constant NativeFormulaProgram& p,thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& g,thread NativeNormalRecord& n){{
 if(s.header.pc==0){{
  s.header.version=LayoutVersion;s.header.tag=uint(Tag::NormalSample);s.value=0.f;s.smooth=0.f;s.child={{}};s.fault={{}};
  const float3 point(s.point[0],s.point[1],s.point[2]);
  if(!normal_sample_begin(point,s.stop,g,n))return normal_sample_phase(s,99);
  if(!s.direct){{
   {de_begin}
   e={{}};e.header.parent_tag=uint(Tag::NormalSample);e.header.parent_pc=1;
   for(uint k=0;k<3;++k)e.point[k]=s.point[k];e.stop=s.stop;e.smooth=s.want_smooth;
   return normal_sample_phase(s,1);
  }}
  if(!normal_sample_direct_begin(point,p,lane,g,n,s.fault))return normal_sample_phase(s,99);
  r={{}};r.header.parent_tag=uint(Tag::NormalSample);r.header.parent_pc=2;
  return normal_sample_phase(s,2);
 }}
 if(s.header.pc==1){{
  if(entry_advance<2>(e,c,d,r,o,step,w,b,p,resources,lane,g)!=99)return 1;
  s.child=e.child;return normal_sample_phase(s,4);
 }}
 if(s.header.pc==2){{
  {bind}
  const uint driver=native_selected_driver();
  const uint child=OrderedFirst&&driver==Hybrid3D?
   orbit_replay_advance<0,false,false,false,3>(r,o,step,w,b,p,lane,resources,s.fault,nullptr,0xffffffffu):
   OrderedFirst&&driver==Hybrid4D&&native_formula_id0==(kInternalFormulaSelector|2u)?
   orbit_replay_advance<0,false,false,false,4>(r,o,step,w,b,p,lane,resources,s.fault,nullptr,0xffffffffu):
   driver==Interpolated4D?
   orbit_replay_advance<0,false,true,false,2>(r,o,step,w,b,p,lane,resources,s.fault,nullptr,0xffffffffu):
   driver==Interpolated3D?
   orbit_replay_advance<0,true,false,false,2>(r,o,step,w,b,p,lane,resources,s.fault,nullptr,0xffffffffu):
   orbit_replay_advance<0,false>(r,o,step,w,b,p,lane,resources,s.fault,nullptr,0xffffffffu);
  {restore}
  if(child!=99)return 2;
  return normal_sample_phase(s,3);
 }}
 if(s.header.pc==3||s.header.pc==4){{
  const auto result=s.header.pc==3?normal_sample_direct_finish(s.fault,lane,g,n):normal_sample_de_finish(s.child,g,n);
  s.value=result.value;s.smooth=result.smooth;return normal_sample_phase(s,99);
 }}
 return s.header.pc;
}}
}} }}
'''

def generate_observer(source):
 regions(source);v=function(source,'general_normal_sample')
 # Observe original DE and direct orbit calls, never a continuation evaluator.
 start=locate(v,['const','auto','value','=']);end=v.index(';',start)+1
 v[start:end]=tokens('observer.record(1,lane,g,n,NativeOrbitResult{},NativeFormulaFault{});')+v[start:end]+tokens('observer.record(4,lane,g,n,value,NativeFormulaFault{});')
 start=locate(v,['(','void',')','native_run_orbit']);end=v.index(';',start)+1
 v[start:end]=tokens('observer.record(2,lane,g,n,NativeOrbitResult{},fault);')+v[start:end]+tokens('observer.record(3,lane,g,n,NativeOrbitResult{},fault);')
 return '''namespace mb3d_native {
template<class Observer> inline GeneralNormalSample normal_sample_original_observed(float3 point,float stop,bool direct,bool smooth,
 constant NativeFormulaProgram& p,thread NativeResources& resources,thread NativeFormulaState& lane,
 thread NativeGeometryRecord& g,thread NativeNormalRecord& n,thread Observer& observer){'''+emit_text(v)+'\n}\n}\n'

if __name__=='__main__':
 import argparse
 from pathlib import Path
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-root',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 output=a.output.resolve();production=Path(__file__).resolve().parents[1]/'src/render/native_metal/generated'
 if output==production or output.is_relative_to(production):ap.error('Separate experimental output required')
 source=(a.source_root/'src/render/native_metal/general_normals.metal.h').read_text();output.mkdir(parents=True,exist_ok=True)
 for name,generate in [('regions',generate_sample),('observer',generate_observer)]:
  (output/f'normal_sample_{name}.metal.h').write_text(generate(source))
