#!/usr/bin/env python3
"""Extract the P1 combination tail around two independently owned DE queries."""
import hashlib
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import ORBIT_SHA,function

def checked(source):
 if hashlib.sha256(source.encode()).hexdigest()!=ORBIT_SHA:raise ValueError('Combined source changed; repeat P1 audit')
 return function(source,'native_distance_combined')

def generate_combined(source):
 v=checked(source);start=locate(v,['thread','auto','&','s','=','out','.','state']);tail=emit_text(v[start:])
 second=locate(v,['const','int','second_maximum','=']);second_end=v.index(';',second)+1
 second_code=emit_text(v[second:second_end]).replace('const int second_maximum =','second_maximum =')
 return f'''namespace mb3d_native {{ namespace continuation {{
inline NativeOrbitResult combined_finish(NativeOrbitResult first,NativeOrbitResult out,float stop,
 constant NativeFormulaProgram& p,constant NativeFormulaProgram& q,thread NativeFormulaState& lane,
 int max_iterations,int second_maximum,uint type){{{tail}}}
inline uint combined_phase(thread CombinedFrame& c,uint pc){{c.header.pc=pc;return pc;}}
inline uint combined_distance(thread CombinedFrame& c,thread DistanceFrame& d,int maximum,uint inside,uint pc){{
 d={{}};d.header.parent_tag=uint(Tag::Combined);d.header.parent_pc=pc;
 for(uint k=0;k<3;++k)d.point[k]=c.point[k];d.stop=c.stop;d.calc_smooth=c.calc_smooth;d.max_iterations=maximum;d.inside_mode=inside;
 return combined_phase(c,pc);
}}
template<uint EntryContext=0> inline uint combined_advance(thread CombinedFrame& c,thread DistanceFrame& d,thread OrbitReplayFrame& r,thread OrbitPassFrame& o,
 thread FormulaStepFrame& g,thread WrapperFrame& w,thread BodyFrame& b,
 constant NativeFormulaProgram& p,thread NativeResources& resources,thread NativeFormulaState& lane,thread uint* range_clamps){{
 const uint type=native_selected_formula_type();
 thread auto& out=c.result;thread auto& first=c.first;thread int& second_maximum=c.second_maximum;
 const int max_iterations=c.max_iterations;const float stop=c.stop;const bool temporary_maximum=bool(c.temporary_maximum);
 if(c.header.pc==0){{
  c.header.version=LayoutVersion;c.header.tag=uint(Tag::Combined);c.type=type;
  if(!type)return combined_distance(c,d,max_iterations,InsideFromLane,1);
  c.state=lane;return combined_distance(c,d,max_iterations,InsideDeferred,2);
 }}
 if(c.header.pc==1){{
  if(distance_advance<0,true,EntryContext>(d,r,o,g,w,b,p,resources,lane,range_clamps)!=99)return 1;
  out=d.result;return combined_phase(c,99);
 }}
 if(c.header.pc==2){{
  if(distance_advance<0,true,EntryContext>(d,r,o,g,w,b,p,resources,c.state,range_clamps)!=99)return 2;
  first=d.result;if(first.fault.code||!first.completed){{out=first;return combined_phase(c,99);}}
  return combined_phase(c,3);
 }}
 if(c.header.pc==3){{
  constant NativeFormulaProgram& q=(&p)[1];
  {second_code}
  return combined_distance(c,d,second_maximum,type==CombineMaximumInverse?InsideForced:InsideDeferred,4);
 }}
 if(c.header.pc==4){{
  constant NativeFormulaProgram& q=(&p)[1];
  if(distance_advance<1,true,EntryContext>(d,r,o,g,w,b,q,resources,c.state,range_clamps)!=99)return 4;
  out=d.result;out.fault.work+=first.fault.work;
  if(out.fault.code||!out.completed)return combined_phase(c,99);
  return combined_phase(c,90);
 }}
 if(c.header.pc==90){{
  constant NativeFormulaProgram& q=(&p)[1];
  c.result=combined_finish(c.first,c.result,stop,p,q,lane,max_iterations,second_maximum,type);
  return combined_phase(c,99);
 }}
 if(c.header.pc!=99)native_fail(out.fault,InvalidFormulaAbi,out.state);
 return combined_phase(c,99);
}}
}} }}
'''


def observer_numeric_regions(source):
 checked(source)
 v=function(source,'native_orbit_passes')
 sites=[i for i in range(len(v)-4)if v[i:i+4]==['if','(','!','native_step']]
 assert len(sites)==3
 for start in reversed(sites):
  end=v.index(';',start)+1;closing=matching(v,start+4,'(',')')
  args=emit_text(v[start+5:closing]).strip()
  assert args.endswith(', overflow');args=args[:-len(', overflow')]
  v[start:end]=tokens('{auto observed_step_result=atomic_step_lane('+args+');s=observed_step_result.lane;overflow=bool(observed_step_result.overflow);if(!observed_step_result.success)return 0.f;}')
 orbit=emit_text(v)
 replay=emit_text(function(source,'native_run_orbit')).replace('native_orbit_passes','combined_observer_orbit')
 distance=emit_text(function(source,'native_distance_controlled')).replace('native_run_orbit','combined_observer_replay')
 return f'''template<uint Part> inline float combined_observer_orbit(constant NativeFormulaProgram& p,thread NativeFormulaState& s,
 thread const NativeResources& resources,thread NativeFormulaFault& f,thread mb3d_native_gradient::RadiusRange* radius_range,
 uint pass_limit,thread uint& exit_pass,thread bool& inert){{{orbit}}}
template<uint Part> inline float combined_observer_replay(constant NativeFormulaProgram& p,thread NativeFormulaState& s,
 thread const NativeResources& resources,thread NativeFormulaFault& f,thread mb3d_native_gradient::RadiusRange* radius_range=nullptr,
 uint first_limit=0xffffffffu){{{replay}}}
template<uint Part> inline NativeOrbitResult combined_observer_query(float3 point,float stop,constant NativeFormulaProgram& p,
 thread NativeResources& resources,thread NativeFormulaState& lane,uint calc_smooth,int max_iterations,
 thread uint* range_clamps,uint inside_mode){{{distance}}}
template<uint Part=0,class Observer> inline NativeOrbitResult combined_observed_distance(float3 point,float stop,constant NativeFormulaProgram& p,
 thread NativeResources& resources,thread NativeFormulaState& lane,uint calc,int maximum,thread uint* clamps,uint inside,
 thread Observer& observer,uint site,NativeOrbitResult first){{
 observer.record(site,lane,NativeOrbitResult{{}},first,calc,maximum,inside);
 NativeOrbitResult result;
 if(site==2&&Part==0&&native_part_driver<Part>()==Interpolated4DAnalytic)
  result=combined_observer_query<Part>(point,stop,p,resources,lane,calc,maximum,clamps,inside);
 else result=native_distance_controlled<Part>(point,stop,p,resources,lane,calc,maximum,clamps,inside);
 observer.record(site+100,lane,result,first,calc,maximum,inside);return result;
}}
'''

def generate_combined_observer(source):
 v=checked(source);sites=[i for i,t in enumerate(v) if t=='native_distance_controlled'];assert len(sites)==3
 for site,start in reversed(list(enumerate(sites,1))):
  begin=v.index('(',start);end=matching(v,begin,'(',')');args=emit_text(v[begin+1:end]).strip()
  if site==1:args+=',InsideFromLane'
  part=1 if site==3 else 0
  v[start:end+1]=tokens(f'combined_observed_distance<{part}>({args},observer,{site},'+('first' if site==3 else 'NativeOrbitResult{}')+')')
 return f'''namespace mb3d_native {{ namespace continuation {{
{observer_numeric_regions(source)}
template<class Observer> inline NativeOrbitResult combined_original_observed(float3 point,float stop,constant NativeFormulaProgram& p,
 thread NativeResources& resources,thread NativeFormulaState& lane,uint calc_smooth,int max_iterations,
 bool temporary_maximum,thread uint* range_clamps,thread Observer& observer){{{emit_text(v)}}}
}} }}
'''
