#!/usr/bin/env python3
"""Source-bound inside retry and post-smoothing owner for P1 primary attempts."""
import hashlib,re
from gen_native_metal_formulas import tokens,matching,emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import function
INSIDE_SHA='da42f277078996615f7dbf0f1f48cf282c234bf2faf73afe287b2803d251e979'

def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=INSIDE_SHA:raise ValueError('Inside bridge changed; repeat P1 caller audit')
 v=function(source,'general_primary_inside');loop=locate(v,tokens('for(uint attempt=0;attempt<2;++attempt)'));a=loop+len(tokens('for(uint attempt=0;attempt<2;++attempt)'));b=matching(v,a);body=v[a+1:b]
 call=locate(body,tokens('pixel=general_primary_camera_attempt('));end=body.index(';',call)+1
 smooth=locate(v,tokens('if(!pixel.fault&&!geometry.fault&&pixel.rm_result&&pixel.start_inside&&state.inside)'));sa=smooth+len(tokens('if(!pixel.fault&&!geometry.fault&&pixel.rm_result&&pixel.start_inside&&state.inside)'));sb=matching(v,sa);sbody=v[sa+1:sb]
 q=locate(sbody,tokens('const auto result=general_de('));qe=sbody.index(';',q)+1
 return dict(prefix=v[:loop],before_attempt=body[:call],attempt_call=body[call:end],after_attempt=body[end:],after_loop=v[b+1:smooth],smooth_condition=v[smooth:sa],before_smooth=sbody[:q],smooth_call=sbody[q:qe],after_smooth=sbody[qe:],finish=v[sb+1:])

def generate_inside(source):
 r=regions(source)
 def text(name):return emit_text(r[name]).replace('return pixel;', 'return inside_phase(n,99);').replace('break;', 'return inside_phase(n,3);')
 prefix=re.sub(r'ExactPixel pixel\s*\{\s*\}\s*;', 'pixel={};',text('prefix')).replace('auto state =','state =')
 args='f.de,f.combined,f.distance,f.replay,f.orbit,f.step,f.wrapper,f.body,program,resources,lane,geometry'
 return f'''namespace mb3d_native {{ namespace continuation {{
inline uint inside_phase(thread InsideFrame& n,uint pc){{n.header.pc=pc;return pc;}}
inline uint inside_advance(thread RayFrame& f,constant ExactScene& scene,constant NativeFormulaProgram& program,constant mb3d_inside::Params& policy,thread NativeResources& resources){{
 thread auto& n=f.inside;thread auto& state=n.retry;thread auto& attempt=n.attempt;
 thread auto& pixel=f.pixel;thread auto& lane=f.lane;thread auto& geometry=f.geometry;thread auto& activity=f.inside_record;
 if(n.header.pc==0){{
  n.header.version=LayoutVersion;n.header.tag=uint(Tag::Inside);
  {prefix}attempt=0;return inside_phase(n,1);
 }}
 if(n.header.pc==1){{
  if(!(attempt<2))return inside_phase(n,3);
  {text('before_attempt')}
  f.primary={{}};f.primary.header.parent_tag=uint(Tag::Inside);f.primary.header.parent_pc=2;f.primary.inside_word=state.inside;
  return inside_phase(n,2);
 }}
 if(n.header.pc==2){{
  if(primary_advance(f,scene,program,resources)!=99)return 2;
  {text('after_attempt')}
  ++attempt;return inside_phase(n,1);
 }}
 if(n.header.pc==3){{
  {text('after_loop')}
  {text('smooth_condition')}{{
   {text('before_smooth')}
   f.de={{}};f.de.header.parent_tag=uint(Tag::Inside);f.de.header.parent_pc=4;
   for(uint k=0;k<3;++k)f.de.point[k]=point[k];f.de.stop=as_type<float>(pixel.final_stop);f.de.smooth=1;
   return inside_phase(n,4);
  }}
  return inside_phase(n,5);
 }}
 if(n.header.pc==4){{
  if(entry_advance<8>({args})!=99)return 4;
  const auto result=f.de.child;{text('after_smooth')}
  return inside_phase(n,5);
 }}
 if(n.header.pc==5){{{text('finish')}}}
 return n.header.pc;
}}
}} }}
'''
if __name__=='__main__':
 import argparse
 from pathlib import Path
 ap=argparse.ArgumentParser(description=__doc__);ap.add_argument('--source-root',type=Path,required=True);ap.add_argument('--output',type=Path,required=True);a=ap.parse_args();out=a.output.resolve();production=Path(__file__).resolve().parents[1]/'src/render/native_metal/generated'
 if out==production or out.is_relative_to(production):ap.error('Separate experimental output required')
 out.mkdir(parents=True,exist_ok=True);source=(a.source_root/'src/render/native_metal/primary_inside_bridge.metal.h').read_text();(out/'inside_regions.metal.h').write_text(generate_inside(source))
