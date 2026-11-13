#pragma once
#include "generated/formula_dispatch.metal.h"
#include "generated/formula_result_contracts.metal.h"

namespace mb3d_native {
// Trait numbering is generated from the CPU contract; keep this wrapper's lane
// mapping aligned with DEFINE_EXTERNAL_FORMULA_WRAPPER_EX, not just xyz/de.
template<bool Fixed=false,uint Count=0,uint Traits=0,int Option=0,uint Flags=0>
inline bool native_formula_step_body(thread NativeFormulaState& s,constant NativeFormulaSlot& slot,
                                thread const NativeResources& resources,thread NativeFormulaFault& fault,uint local_limit){
    if(fault.code)return false;
    if(slot.version!=kFormulaAbiVersion||slot.namespace_id!=ExternalFormula||slot.param_count>58u||!local_limit||local_limit>(1u<<20))
        return native_formula_fail(fault,InvalidFormulaAbi,slot.id);
    const uint count=Fixed?Count:slot.param_count;
    const uint t=Fixed?Traits:slot.traits;
    const int option=Fixed?Option:slot.de_option;
    const uint flags=Fixed?Flags:slot.wrapper_flags;
    float a[64]={};
    for(uint i=0;i<count;++i)a[i]=slot.params[i];
    uint n=count,c=n;
    for(uint i=0;i<4;++i)a[n++]=(t&(1u<<11))?(i==3?s.j[3]:s.c[i]):s.j[i];
    a[n++]=s.vary_scale;a[n++]=float(s.first);
    if(t&(1u<<12)){if(n+4>64)return native_formula_fail(fault,InvalidFormulaParameter,slot.id);for(uint i=0;i<4;++i)a[n++]=i==3?s.j[3]:s.c[i];}
    if(t&(1u<<13)){if(n>=64)return native_formula_fail(fault,InvalidFormulaParameter,slot.id);a[n++]=float(s.julia);}
    if((t&(1u<<8))||native_external_reads_caller_rout(slot.id,count,t,option,flags))a[56]=s.rout;
    a[52]=a[53]=a[54]=a[55]=NAN;
    if(t&(1u<<3)){a[47]=s.xyz[3];for(uint i=0;i<4;++i)a[48+i]=s.sse[i];}
    else if(t&(1u<<2)){a[48]=s.dfree1;a[49]=s.dfree2;a[50]=s.derivative[1];a[51]=s.derivative[2];}
    else if(t&(1u<<4)){a[48]=s.derivative[0];a[49]=s.derivative[1];a[50]=s.derivative[2];a[51]=s.dfree1;}
    else if(t&(1u<<1)){for(uint i=0;i<3;++i)a[48+i]=s.derivative[i];}
    if(t&(1u<<9))a[51]=s.xyz[3];
    a[59]=float(s.iteration);a[57]=float(s.first);
    FormulaVec3 p(s.xyz[0],s.xyz[1],s.xyz[2]),result=p;
    float de=0.0f,vs=s.vary_scale,saved=s.vary_scale;
    if(option==20){a[63]=s.dfree1;a[62]=s.rstop_d;a[58]=s.rold;a[61]=slot.difs_style?0.0f:1.0f;a[60]=vs;}
    else if(option==21){a[60]=vs;if(slot.difs_style)de=vs;}
    else{if(option==2||option==11||option==4||option==5||option==6)de=s.xyz[3];a[60]=vs;}
    int first_counter=s.first;bool first_counter_written=false;
    FormulaParams view{a,&slot,&resources,&fault,&first_counter,&first_counter_written,local_limit};
    if(!native_external_raw(slot.id,p,view,de,vs,result))return native_formula_fail(fault,UnsupportedFormula,slot.id);
    if(fault.code)return false;
    if(option==20){
        s.vary_scale=slot.difs_style?a[60]:vs;s.rout=de;s.dfree1=a[63];
        if(slot.difs_style)s.first=first_counter;
        else if(a[61]==0.0f)result=p;
    }else if(option==21){
        s.vary_scale=slot.difs_style?(a[60]!=saved?a[60]:de):vs;
        if(slot.difs_style)s.first=first_counter;
    }else{
        if(option==6)s.derivative[0]*=abs(a[0]);
        s.vary_scale=a[60];s.first=first_counter;
        if(option==2||option==11||option==4||option==5||option==6)s.xyz[3]=de;
    }
    // Checked native lowering can return original coordinates directly, while
    // keeping the uploaded CPU adapter descriptor unchanged.
    if(!(flags&1u)&&!native_external_returns_coordinates(slot.id)){
        float4 add(a[c],a[c+1],a[c+2],a[c+3]);
        if(option==4||option==5||option==6||(t&(1u<<5)))for(uint i=0;i<4;++i)if(!isnan(a[52+i]))add[i]=a[52+i];
        result+=add.xyz;if(option==4||option==5||option==6)s.xyz[3]+=add.w;
    }
    if(t&(1u<<3)){for(uint i=0;i<4;++i)s.sse[i]=a[48+i];}
    else if(t&(1u<<2)){s.dfree1=a[48];s.dfree2=a[49];s.derivative[1]=a[50];s.derivative[2]=a[51];}
    else if(t&(1u<<4)){s.derivative[0]=a[48];s.derivative[1]=a[49];s.derivative[2]=a[50];s.dfree1=a[51];}
    else if(t&(1u<<1)){for(uint i=0;i<3;++i)s.derivative[i]=a[48+i];}
    if(t&(1u<<9))s.xyz[3]=a[51];
    for(uint i=0;i<4;++i)if((t&(1u<<(i==3?7:6)))||!isnan(a[52+i])){
        s.j[i]=a[52+i];if(i<3&&(t&(1u<<10)))s.c[i]=a[52+i];
    }
    // A non-finite result is an FP32 range exit; native_step classifies it.
    s.xyz[0]=result.x;s.xyz[1]=result.y;s.xyz[2]=result.z;
    return true;
}
constant uint native_wrapper_policy [[function_constant(18)]];
inline uint native_selected_wrapper_policy() {
    return is_function_constant_defined(native_wrapper_policy)?native_wrapper_policy:0u;
}
inline uint native_fixed_wrapper_id(constant NativeFormulaSlot& slot) {
    if(native_selected_wrapper_policy()!=1 || slot.version!=kFormulaAbiVersion ||
       slot.namespace_id!=ExternalFormula || slot.difs_style)return 0;
    if(slot.id==9401 && slot.param_count==1 && slot.traits==0 && slot.de_option==4 && slot.wrapper_flags==0)return 9401;
    if(slot.id==10350 && slot.param_count==25 && slot.traits==0 && slot.de_option==6 && slot.wrapper_flags==1)return 10350;
    // MixPinski4 stays generic: specializing its metadata changed Z rounding
    // under the Native compiler's contraction policy in the device controls.
    if(slot.id==5823 && slot.param_count==6 && slot.traits==0 && slot.de_option==-1 && slot.wrapper_flags==1)return 5823;
    return 0;
}
inline bool native_formula_step(thread NativeFormulaState& s,constant NativeFormulaSlot& slot,
    thread const NativeResources& resources,thread NativeFormulaFault& fault,uint local_limit) {
    const uint policy=native_selected_wrapper_policy();
    if(policy>1)return native_formula_fail(fault,InvalidFormulaAbi,slot.id);
    // Only immutable ABI metadata is specialized. Formula bodies, operation
    // order, carried state and finite/work guards remain the shared code.
    // An unrecognised descriptor still follows the original generic wrapper.
    const uint fixed=native_fixed_wrapper_id(slot);
    {
        if(fixed==9401)
            return native_formula_step_body<true,1,0,4,0>(s,slot,resources,fault,local_limit);
        if(fixed==10350)
            return native_formula_step_body<true,25,0,6,1>(s,slot,resources,fault,local_limit);
        if(fixed==5823)
            return native_formula_step_body<true,6,0,-1,1>(s,slot,resources,fault,local_limit);
    }
    return native_formula_step_body<>(s,slot,resources,fault,local_limit);
}
#ifdef MB3D_NATIVE_FORMULA_PROBE_KERNELS
// Separate diagnostics: never change the 59-word arithmetic result record.
[[host_name("native_formula_wrapper_audit")]] kernel void native_formula_wrapper_audit(constant NativeFormulaProgram& program [[buffer(0)]],
    device uint* output [[buffer(1)]],uint tid [[thread_position_in_grid]]) {
    if(tid>=6)return;
    output[tid*3]=1;output[tid*3+1]=native_selected_wrapper_policy();
    output[tid*3+2]=native_fixed_wrapper_id(program.slots[tid]);
}
#endif
} // namespace mb3d_native
