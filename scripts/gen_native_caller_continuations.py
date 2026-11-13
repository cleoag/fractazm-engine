#!/usr/bin/env python3
"""Extract P1 caller regions without rewriting their mathematical statements."""
import argparse
import hashlib
from pathlib import Path
from gen_native_metal_formulas import tokens, matching, emit_text
from gen_native_continuations import locate
from gen_native_combined_continuations import generate_combined,generate_combined_observer
from gen_native_distance_continuations import generate_distance,generate_distance_observer
from gen_native_replay_continuations import generate_replay,generate_replay_observer
from gen_native_orbit_continuations import generate_step,generate_orbit,generate_observer

WRAPPER_SHA='0714fda12478d2e2d11fc1b7551bc9c83a11301e86ebab527b263a4e9df968b7'

def generate_wrapper(source):
    if hashlib.sha256(source.encode()).hexdigest()!=WRAPPER_SHA:
        raise ValueError('Wrapper does not match selected P1 source; caller audit required')
    v=tokens(source);a=locate(v,['native_formula_step_body','(']);opening=v.index('{',a);v=v[opening+1:matching(v,opening)]
    a=locate(v,['if','(','!','native_external_raw','(']);b=v.index(';',a)+1
    prefix=emit_text(v[:a]);tail=emit_text(v[b:])
    capture='''for(uint i=0;i<64;++i)w.a[i]=a[i];
w.count=count;w.traits=t;w.option=option;w.flags=flags;w.n=n;w.c=c;
w.p[0]=p.x;w.p[1]=p.y;w.p[2]=p.z;
w.result[0]=result.x;w.result[1]=result.y;w.result[2]=result.z;
w.de=de;w.vs=vs;w.saved=saved;
w.first_counter=first_counter;w.first_counter_written=uint(first_counter_written);'''
    restore='''[[maybe_unused]] const uint count=w.count,t=w.traits,flags=w.flags,n=w.n,c=w.c;
const int option=w.option;float a[64];for(uint i=0;i<64;++i)a[i]=w.a[i];
FormulaVec3 p(w.p[0],w.p[1],w.p[2]),result(w.result[0],w.result[1],w.result[2]);
float de=w.de,vs=w.vs,saved=w.saved;int first_counter=w.first_counter;
[[maybe_unused]] bool first_counter_written=bool(w.first_counter_written);'''
    observer=prefix+'\n'+capture+'\nobserver.record(w,1,fault);\n'+emit_text(v[a:b])+'\n'+capture+'\nobserver.record(w,2,fault);\n'+tail
    return f'''// Generated from selected P1 wrapper {WRAPPER_SHA}.
namespace mb3d_native {{ namespace continuation {{
template<bool Fixed=false,uint Count=0,uint Traits=0,int Option=0,uint Flags=0>
inline bool wrapper_begin(thread WrapperFrame& w,thread NativeFormulaState& s,
 constant NativeFormulaSlot& slot,thread const NativeResources& resources,
 thread NativeFormulaFault& fault,uint local_limit) {{
{prefix}
(void)view; {capture}
return true;
}}
inline bool wrapper_finish(thread WrapperFrame& w,thread NativeFormulaState& s,
 constant NativeFormulaSlot& slot,thread NativeFormulaFault& fault) {{
{restore}
{tail}
}}
template<class Observer>
inline bool wrapper_original_observed(thread NativeFormulaState& s,
 constant NativeFormulaSlot& slot,thread const NativeResources& resources,
 thread NativeFormulaFault& fault,uint local_limit,thread Observer& observer) {{
 constexpr bool Fixed=false;constexpr uint Count=0,Traits=0,Flags=0;constexpr int Option=0;
 WrapperFrame w{{}};
{observer}
}}
}} }}
'''

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--source-root',type=Path,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
    source=(a.source_root/'src/render/native_metal/formula_wrapper.metal.h').read_text()
    output=a.output.resolve();production=Path(__file__).resolve().parents[1]/'src/render/native_metal/generated'
    if output==production or output.is_relative_to(production):p.error('Separate experimental output required')
    code=generate_wrapper(source);output.mkdir(parents=True,exist_ok=True);(output/'wrapper_regions.metal.h').write_text(code)
    orbit=(a.source_root/'src/render/native_metal/orbit_driver.metal.h').read_text()
    (output/'formula_step_regions.metal.h').write_text(generate_step(orbit))
    (output/'orbit_regions.metal.h').write_text(generate_orbit(orbit))
    (output/'orbit_observer.metal.h').write_text(generate_observer(orbit))
    (output/'combined_regions.metal.h').write_text(generate_combined(orbit))
    (output/'combined_observer.metal.h').write_text(generate_combined_observer(orbit))
    (output/'distance_regions.metal.h').write_text(generate_distance(orbit))
    (output/'distance_observer.metal.h').write_text(generate_distance_observer(orbit))
    (output/'replay_regions.metal.h').write_text(generate_replay(orbit))
    (output/'replay_observer.metal.h').write_text(generate_replay_observer(orbit))
if __name__=='__main__':main()
