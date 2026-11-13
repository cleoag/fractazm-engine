#!/usr/bin/env python3
"""Source-bound ordered seven-sample normal continuation for the P1 preview.

Normal smoothing and reflection packing are separate capabilities. This private
stepper refuses smoothing before any child sample. P1 preview always uses zero.
"""
import hashlib,re
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import function
from gen_native_normal_sample_continuations import NORMAL_SHA


def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=NORMAL_SHA:raise ValueError('Normal source changed; repeat P1 audit')
 v=function(source,'general_calculate_normals')
 first=locate(v,['const','auto','first','=']);end=v.index(';',first)+1
 branch=locate(v,['if','(','smooth','==','8',')','{']);close=matching(v,branch+6)
 assert v[close+1:close+3]==['else','{']
 loop=v[close+3:matching(v,close+2)]
 begin=locate(loop,['for','(','uint','sample','=']);body=loop.index('{',begin);last=matching(loop,body)
 q=loop[body+1:last];a=locate(q,['const','float','a','=']);ae=q.index(';',a)+1
 b=locate(q,['const','float','b','=']);be=q.index(';',b)+1
 tail=locate(v,['for','(','uint','i','=','0',';','i','<','3',';','++','i',')','lane','.','c'])
 return dict(prefix=v[:first],after_first=v[end:branch],sample_setup=q[:a],after_pair=q[be:],tail=v[tail:])


def generate_normal(source):
 q=regions(source)
 def text(v):return emit_text(v)
 prefix=text(q['prefix'])
 prefix=re.sub(r'NativeNormalResult out\s*\{\s*\}\s*;', '',prefix)
 prefix=prefix.replace('const uint initial_samples =','initial_samples =').replace('const bool direct =','direct =')
 prefix=prefix.replace('const int smooth = int ( params . smoothing );','')
 prefix=prefix.replace('float offset =','offset =').replace('return out;', 'return normal_phase(n,99);')
 after=text(q['after_first']).replace('float middle =','middle =').replace('float3 normal ( 0.f );','normal=float3(0.f);')
 setup=text(q['sample_setup']).replace('const uint axis =','axis =')
 pair=text(q['after_pair'])
 tail=text(q['tail']).replace('return out;', 'return normal_phase(n,99);')
 return f'''namespace mb3d_native {{ namespace continuation {{
inline uint normal_phase(thread NormalFrame& n,uint pc){{n.header.pc=pc;return pc;}}
inline void normal_sample_child(thread NormalSampleFrame& s,float3 point,float stop,bool direct,bool smooth,uint parent_pc){{
 s={{}};s.header.parent_tag=uint(Tag::Normal);s.header.parent_pc=parent_pc;
 for(uint k=0;k<3;++k)s.point[k]=point[k];s.stop=stop;s.direct=uint(direct);s.want_smooth=uint(smooth);
}}
inline uint normal_advance(thread NormalFrame& n,thread NormalSampleFrame& s,
 thread DistanceEntryFrame& e,thread CombinedFrame& c,thread DistanceFrame& d,thread OrbitReplayFrame& r,
 thread OrbitPassFrame& o,thread FormulaStepFrame& step,thread WrapperFrame& w,thread BodyFrame& body,
 constant ExactScene& scene,constant NativeFormulaProgram& program,NativeNormalParams params,
 thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& geometry,
 thread NativeNormalRecord& record){{
 thread auto& out=n.result;thread auto& initial_samples=n.initial_samples;thread auto& direct=n.direct;
 thread auto& offset=n.offset;thread auto& middle=n.middle;thread auto& sample=n.sample;thread auto& axis=n.axis;
 const float3 center(n.center[0],n.center[1],n.center[2]);const float depth=n.depth,stop=n.stop;
 float3 normal=n.header.pc>=2?float3(n.normal[0],n.normal[1],n.normal[2]):float3(0.f);const bool reflection_pack=false;
 if(n.header.pc==0){{
  n.header.version=LayoutVersion;n.header.tag=uint(Tag::Normal);out={{}};
  if(params.smoothing){{general_normal_fault(geometry,record,1);out.fault=record.fault;return normal_phase(n,99);}}
  {prefix}
  normal_sample_child(s,center,stop,bool(direct),true,1);return normal_phase(n,1);
 }}
 if(n.header.pc==1||n.header.pc==3||n.header.pc==4){{
  uint child;
  if(n.header.pc==1)child=normal_sample_advance<true>(s,e,c,d,r,o,step,w,body,program,resources,lane,geometry,record);
  else child=normal_sample_advance(s,e,c,d,r,o,step,w,body,program,resources,lane,geometry,record);
  if(child!=99)return n.header.pc;
  if(n.header.pc==1){{
   const GeneralNormalSample first{{s.value,s.smooth}};{after}
   for(uint k=0;k<3;++k)n.normal[k]=normal[k];sample=0;return normal_phase(n,2);
  }}
  if(n.header.pc==3){{n.a=s.value;normal_sample_child(s,float3(n.minus[0],n.minus[1],n.minus[2]),stop,bool(direct),bool(direct),4);return normal_phase(n,4);}}
  const float a=n.a,b=s.value;{pair}
  for(uint k=0;k<3;++k)n.normal[k]=normal[k];++sample;return normal_phase(n,2);
 }}
 if(n.header.pc==2){{
  if(!(sample<3&&!record.fault))return normal_phase(n,90);
  {setup}
  for(uint k=0;k<3;++k){{n.plus[k]=plus[k];n.minus[k]=minus[k];}}
  normal_sample_child(s,plus,stop,bool(direct),bool(direct),3);return normal_phase(n,3);
 }}
 if(n.header.pc==90){{{tail}}}
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
 source=(a.source_root/'src/render/native_metal/general_normals.metal.h').read_text();out.mkdir(parents=True,exist_ok=True)
 (out/'normal_regions.metal.h').write_text(generate_normal(source))
