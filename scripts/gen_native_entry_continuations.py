#!/usr/bin/env python3
"""Source-bound P1 geometry DE entry, with scoped observer rebinding."""
import hashlib,re
from gen_native_metal_formulas import emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import function
GEOMETRY_SHA='8f26f24aeb004014f806a2447d51ddf60e0b3cd48e206c4e31f6ba19b55aa819'

def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=GEOMETRY_SHA:raise ValueError('Geometry source changed; repeat P1 audit')
 v=function(source,'general_de_inline')
 bind=locate(v,['thread','NativeSmoothingCounters','*','previous_smoothing'])
 call=locate(v,['result','=','native_distance_combined','(']);end=v.index(';',call)+1
 restore=locate(v,['record','.','formula_work','+='])
 return tuple(emit_text(x) for x in (v[:bind],v[bind:call],v[call:end],v[end:restore],v[restore:]))

SIGNATURE='''float3 point,float stop,constant NativeFormulaProgram& program,
 thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record,
 bool smooth,int max_iterations,bool temporary_maximum'''

def generate_original(source):
 return 'namespace mb3d_native {\ninline NativeOrbitResult entry_original('+SIGNATURE+') {\n'+'\n'.join(regions(source))+'\n}\n}\n'

def generate_entry(source):
 prefix,bind,call,restore,tail=regions(source)
 begin=re.sub(r'NativeOrbitResult result\s*\{\s*\}\s*;','',prefix)
 begin=re.sub(r'return\s+result\s*;','return false;',begin)
 finish=re.sub(r'return\s+result\s*;','',tail)
 bind=re.sub(r'uint range_clamps = 0\s*;','thread uint& range_clamps=e.range_clamps;',bind)
 return f'''namespace mb3d_native {{ namespace continuation {{
inline bool entry_begin(thread NativeGeometryRecord& record){{{begin}return true;}}
inline void entry_finish(NativeOrbitResult result,thread NativeGeometryRecord& record,uint range_clamps){{{finish}}}
inline uint entry_phase(thread DistanceEntryFrame& e,uint pc){{e.header.pc=pc;return pc;}}
template<uint ParentContext=1> inline uint entry_advance(thread DistanceEntryFrame& e,thread CombinedFrame& c,thread DistanceFrame& d,
 thread OrbitReplayFrame& r,thread OrbitPassFrame& o,thread FormulaStepFrame& g,thread WrapperFrame& w,thread BodyFrame& b,
 constant NativeFormulaProgram& program,thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& record){{
 if(e.header.pc==0){{
  e.header.version=LayoutVersion;e.header.tag=uint(Tag::DistanceEntry);e.child={{}};
  if(!entry_begin(record))return entry_phase(e,99);
  c={{}};c.header.parent_tag=uint(Tag::DistanceEntry);c.header.parent_pc=1;
  for(uint k=0;k<3;++k)c.point[k]=e.point[k];c.stop=e.stop;c.calc_smooth=uint(bool(e.smooth));
  c.max_iterations=e.max_iterations?e.max_iterations:program.max_iterations;c.temporary_maximum=e.temporary_maximum;e.range_clamps=0;
  return entry_phase(e,1);
 }}
 if(e.header.pc==1){{
  {bind}
  const uint child=combined_advance<ParentContext>(c,d,r,o,g,w,b,program,resources,lane,&range_clamps);
  {restore}
  if(child!=99)return 1;
  e.child=c.result;return entry_phase(e,2);
 }}
 if(e.header.pc==2){{entry_finish(e.child,record,e.range_clamps);return entry_phase(e,99);}}
 return e.header.pc;
}}
}} }}
'''

def generate_observer(source):
 prefix,bind,call,restore,tail=regions(source)
 observe='observer.record({phase},lane,result,record,uint(smooth),max_iterations?max_iterations:program.max_iterations,uint(temporary_maximum),resources.radius_stop,range_clamps);'
 call=call.replace('native_distance_combined','entry_observer_child')
 helper='''#include "combined_observer.metal.h"
namespace mb3d_native {
struct EntryChildObserver {
 void record(uint,thread const NativeFormulaState&,thread const NativeOrbitResult&,thread const NativeOrbitResult&,uint,int,uint) thread {}
};
inline NativeOrbitResult entry_observer_child(float3 point,float stop,constant NativeFormulaProgram& program,
 thread NativeResources& resources,thread NativeFormulaState& lane,uint smooth,int maximum,bool temporary,thread uint* clamps){
 if(native_selected_formula_type()!=0&&native_selected_driver()==Interpolated4DAnalytic&&native_formula_id0!=10089u){
  EntryChildObserver observer;
  return continuation::combined_original_observed(point,stop,program,resources,lane,smooth,maximum,temporary,clamps,observer);
 }
 return native_distance_combined(point,stop,program,resources,lane,smooth,maximum,temporary,clamps);
}
}
'''
 return helper+'namespace mb3d_native {\ntemplate<class Observer>\ninline NativeOrbitResult entry_original_observed('+SIGNATURE+',thread Observer& observer){\n'+'\n'.join((prefix,bind,observe.format(phase=1),call,observe.format(phase=2),restore,tail))+'\n}\n}\n'

if __name__=='__main__':
 import argparse
 from pathlib import Path
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-root',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args()
 output=a.output.resolve();production=Path(__file__).resolve().parents[1]/'src/render/native_metal/generated'
 if output==production or output.is_relative_to(production):ap.error('Separate experimental output required')
 source=(a.source_root/'src/render/native_metal/general_geometry.metal.h').read_text();output.mkdir(parents=True,exist_ok=True)
 for name,generate in [('original',generate_original),('regions',generate_entry),('observer',generate_observer)]:
  (output/f'entry_{name}.metal.h').write_text(generate(source))
