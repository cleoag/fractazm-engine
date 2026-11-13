#!/usr/bin/env python3
"""Preserve P1's explicit range replay; never replay to reconstruct a yield."""
import hashlib
from gen_native_metal_formulas import emit_text
from gen_native_continuations import locate
from gen_native_orbit_continuations import ORBIT_SHA,function

def regions(source):
 if hashlib.sha256(source.encode()).hexdigest()!=ORBIT_SHA:raise ValueError('Replay source changed; repeat P1 audit')
 v=function(source,'native_run_orbit');loop=locate(v,['for','(',';',';',')','{']);body=v[loop+6:-1]
 call=locate(body,['const','float','result','=']);end=body.index(';',call)+1
 return v[:loop],body[:call],body[call:end],body[end:]

def generate_replay(source):
 prefix,before,call,tail=regions(source)
 # The original entry is immutable across retries; its work counter lives in f.
 prefix=emit_text(prefix).replace('const NativeFormulaState entry = s;', 'r.entry=s;').replace('uint pass_limit = first_limit;', 'r.pass_limit=first_limit;')
 tail=emit_text(tail).replace('return result;', 'r.header.pc=99;return 99;')
 # The original single-line if needs a compound return now that PC is stored.
 tail=tail.replace('if ( exit_pass == 0xffffffffu || f . code ) r.header.pc=99;return 99;', 'if(exit_pass==0xffffffffu||f.code){r.header.pc=99;return 99;}')
 code=f'''namespace mb3d_native {{ namespace continuation {{
template<uint Part=0,bool ReplayContext=true,bool DistanceContext=false,bool CombinedContext=false,uint EntryContext=0> inline uint orbit_replay_advance(thread OrbitReplayFrame& r,thread OrbitPassFrame& o,
 thread FormulaStepFrame& g,thread WrapperFrame& w,thread BodyFrame& b,
 constant NativeFormulaProgram& p,thread NativeFormulaState& s,
 thread const NativeResources& resources,thread NativeFormulaFault& f,
 thread mb3d_native_gradient::RadiusRange* radius_range,uint first_limit){{
 if(r.header.pc==0){{{prefix}r.header.version=LayoutVersion;r.header.tag=uint(Tag::OrbitReplay);r.header.pc=1;return 1;}}
 if(r.header.pc==1){{
  o={{}};o.pass_limit=r.pass_limit;o.exit_pass=0xffffffffu;o.inert=false;
  o.header.parent_tag=uint(Tag::OrbitReplay);o.header.parent_pc=2;
  r.exit_pass=0xffffffffu;r.inert=false;r.header.pc=2;return 2;
 }}
 if(r.header.pc==2){{
  if(orbit_pass_advance<Part,ReplayContext,DistanceContext,CombinedContext,EntryContext>(o,g,w,b,p,s,resources,f,radius_range)==99){{r.result=o.result;r.exit_pass=o.exit_pass;r.inert=uint(o.inert);r.header.pc=3;}}
  return r.header.pc;
 }}
 if(r.header.pc==3){{
  thread const auto& entry=r.entry;thread uint& pass_limit=r.pass_limit;
  const uint exit_pass=r.exit_pass;const bool inert=bool(r.inert);
  {tail}
  r.header.pc=1;return 1;
 }}
 if(r.header.pc!=99)native_fail(f,InvalidFormulaAbi,s);
 r.header.pc=99;return 99;
}}
}} }}
'''
 return code

def generate_replay_observer(source):
 prefix,before,call,tail=regions(source)
 call=emit_text(call)
 return f'''namespace mb3d_native {{ namespace continuation {{
template<uint Part=0,class Observer> inline float replay_original_observed(constant NativeFormulaProgram& p,thread NativeFormulaState& s,
 thread const NativeResources& resources,thread NativeFormulaFault& f,
 thread mb3d_native_gradient::RadiusRange* radius_range,uint first_limit,thread Observer& observer){{
 {emit_text(prefix)}
 for(;;){{
  {emit_text(before)}
  observer.outer(201,s,entry,f,pass_limit,0.f,exit_pass,inert,radius_range);
  {call}
  observer.outer(202,s,entry,f,pass_limit,result,exit_pass,inert,radius_range);
  {emit_text(tail)}
 }}
}}
}} }}
'''
