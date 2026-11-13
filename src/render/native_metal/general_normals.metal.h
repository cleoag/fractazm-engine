#pragma once
// Staged implementation: include after general_de/general_column. No CPU callback.
#include "general_normals_abi.h"
namespace mb3d_native {
inline bool valid_general_normals(NativeNormalParams n,constant NativeFormulaProgram& p) {
    if(n.version!=kGeneralNormalVersion||n.normals_on_de>1||n.smoothing>15||n.inside>1)return false;
    for(uint i=0;i<4;++i)if(n.reserved[i])return false;
    // GetMCT forces normals-on-DE for analytic/dIFS formulas. Direct normals
    // must use the same void mMandFunction driver as ordinary CPU preparation.
    const uint d=native_selected_driver();
    return n.normals_on_de||d==Hybrid3D||d==Hybrid4D||d==Interpolated3D||d==Interpolated4D;
}
inline void general_normal_fault(thread NativeGeometryRecord& g,thread NativeNormalRecord& n,uint code) {
    g.fault|=code;n.fault|=code;
}
struct GeneralNormalSample {float value,smooth;};
inline GeneralNormalSample general_normal_sample(float3 point,float stop,bool direct,bool smooth,
    constant NativeFormulaProgram& p,thread NativeResources& resources,thread NativeFormulaState& lane,
    thread NativeGeometryRecord& g,thread NativeNormalRecord& n) {
    GeneralNormalSample out{};
    if(g.fault||n.fault)return out;
    if(n.sample_calls>=kGeneralNormalSampleBudget||g.normal_calls>=kGeneralNormalSampleBudget||
       g.formula_work>=kGeneralMaxFormulaWork){general_normal_fault(g,n,2);return out;}
    if(!all(isfinite(point))||!isfinite(stop)||stop<=0){general_normal_fault(g,n,4);return out;}
    ++n.sample_calls;++g.normal_calls;
    if(!direct){
        ++n.de_calls;const auto value=general_de(point,stop,p,resources,lane,g,smooth);
        n.fault|=g.fault;out.value=value.distance;out.smooth=value.state.smooth;return out;
    }
    if(n.direct_orbit_calls>=kGeneralNormalOrbitBudget){general_normal_fault(g,n,2);return out;}
    ++n.direct_orbit_calls;
    // Restore the scalar contract that CPU RMdoBinSearchIt restores on exit.
    // Preserve the persistent conditional/dIFS/work lanes; do not reinitialize.
    for(uint i=0;i<3;++i)lane.c[i]=point[i];
    lane.max_iterations=p.max_iterations;lane.rstop=p.radius_stop;lane.calc_smooth=1;
    NativeFormulaFault fault{};fault.limit=p.work_limit;
    thread NativeSmoothingCounters* previous_smoothing=resources.smoothing;resources.smoothing=&g.smoothing;
    thread uint* previous_exits=resources.range_exits;resources.range_exits=&g.fp32_range_exits;
    thread uint* previous_carried=resources.carried_reads;resources.carried_reads=&g.difs_carried_reads;
    (void)native_run_orbit(p,lane,resources,fault);
    resources.smoothing=previous_smoothing;resources.range_exits=previous_exits;resources.carried_reads=previous_carried;
    g.formula_work+=fault.work;
    if(fault.code){g.formula_fault=fault;general_normal_fault(g,n,
        fault.code==FormulaWorkExhausted?2u:fault.code==NonFiniteFormula?4u:8u);return out;}
    if(g.formula_work>kGeneralMaxFormulaWork){g.formula_fault=fault;
        g.formula_fault.code=FormulaWorkExhausted;general_normal_fault(g,n,2);return out;}
    if(!isfinite(lane.smooth)||!isfinite(lane.rout)||!isfinite(lane.otrap)){
        native_fail(g.formula_fault,NonFiniteFormula,lane);general_normal_fault(g,n,4);return out;}
    out.value=lane.smooth;out.smooth=lane.smooth;return out;
}
inline float3 general_normal_world(float3 v,constant ExactScene& scene){
    return (general_column(scene,0)*v.x+general_column(scene,1)*v.y)+general_column(scene,2)*v.z;
}
inline bool general_normal_tangents(float3 n,thread float3& vx,thread float3& vy) {
    if(!all(isfinite(n)))return false;
    const float xyScale=max(abs(n.x),abs(n.y));
    if(xyScale==0){vx=float3(1,0,0);vy=float3(0,1,0);return true;}
    const float2 xy=n.xy/xyScale;const float xyLength=sqrt(dot(xy,xy));
    // Source d<1e-50 is tested as |xy|<1e-25 to avoid an FP32 underflow.
    if(xyScale<1.e-25f && xyScale*xyLength<1.e-25f){vx=float3(1,0,0);vy=float3(0,1,0);return true;}
    const float scale=max(xyScale,abs(n.z));const float3 unit=n/scale;
    const float angle=acos(clamp(-unit.z/sqrt(dot(unit,unit)),-1.f,1.f))*.5f;
    const float sine=sin(angle),cosine=cos(angle);
    const float qx=-(xy.y/xyLength)*sine,qy=(xy.x/xyLength)*sine;
    vx=float3(1.f-2.f*qy*qy,2.f*qx*qy,2.f*qy*cosine);
    vy=float3(vx.y,1.f-2.f*qx*qx,-2.f*qx*cosine);
    return all(isfinite(vx))&&all(isfinite(vy));
}
inline float general_normal_scaled_product(float a,float b,float divisor) {
    // Hardware-FP32 exponent scaling prevents overflow before the roughness
    // clamp. Only the bounded ratio0..4 matters; larger values saturate.
    if(a==0||b==0)return 0.f;
    int ea=0,eb=0,ed=0;const float ma=frexp(a,ea),mb=frexp(b,eb),md=frexp(divisor,ed);
    const int exponent=ea+eb-ed;const float mantissa=(ma*mb)/md;
    if(exponent>4)return 4.f;if(exponent< -126)return 0.f;
    return min(4.f,ldexp(mantissa,exponent));
}
inline float general_normal_roughness(float3 n,float step,float variance) {
    // Same RMCalcRoughness ratio, expressed in scaled FP32 so the1e-40
    // stabilizer remains effective even on hardware flushing subnormals.
    const float scale=max(max(abs(n.x),max(abs(n.y),abs(n.z))),1.e-20f);
    const float3 v=n/scale;const float epsilon=1.e-20f/scale;
    const float denominator=(v.x*v.x+v.y*v.y)+v.z*v.z+epsilon*epsilon;
    const float term=general_normal_scaled_product(sqrt(abs(variance))*sqrt(7.f),abs(step),scale);
    const float numerator=epsilon*epsilon+(variance<0.f?-term*term:term*term);
    return clamp(sqrt(max(0.f,numerator/denominator))-.05f,0.f,1.f);
}
inline NativeNormalResult general_calculate_normals(float3 center,float depth,float stop,
    constant ExactScene& scene,constant NativeFormulaProgram& program,NativeNormalParams params,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread NativeGeometryRecord& geometry,
    thread NativeNormalRecord& record,bool reflection_pack=false) {
    NativeNormalResult out{};
    if((record.version&&record.version!=kGeneralNormalVersion)||record.reserved[0]||record.reserved[1]||
       !valid_general_normals(params,program)||!valid_general_scene(scene,program)){
        general_normal_fault(geometry,record,1);out.fault=record.fault;return out;}
    record.version=kGeneralNormalVersion;
    const uint initial_samples=record.sample_calls;
    const bool direct=!params.normals_on_de;const int smooth=int(params.smoothing);
    float offset=min(1.f,as_type<float>(scene.de_stop))*(1.f+depth*as_type<float>(scene.de_stop_factor))*.15f;
    if(!all(isfinite(center))||!isfinite(depth)||!isfinite(offset)||offset<0){
        general_normal_fault(geometry,record,4);out.fault=record.fault;return out;}
    const auto first=general_normal_sample(center,stop,direct,true,program,resources,lane,geometry,record);
    out.smooth=first.smooth;float middle=first.value;float3 normal(0.f);
    if(smooth==8){
        const float step=offset*1.3333f;
        for(int iz=-2;iz<=2&&!record.fault;++iz)for(int iy=-2;iy<=2&&!record.fault;++iy)for(int ix=-2;ix<=2&&!record.fault;++ix){
            if(!(iz|iy|ix))continue;
            float3 point=center+general_column(scene,2)*(float(iz)*step);
            point+=general_column(scene,1)*(float(iy)*step);point+=general_column(scene,0)*(float(ix)*step);
            const float value=general_normal_sample(point,stop,direct,direct,program,resources,lane,geometry,record).value;
            const float sign=direct?-1.f:1.f;
            if(iz)normal.z+=sign*value/float(iz);if(iy)normal.y+=sign*value/float(iy);if(ix)normal.x+=sign*value/float(ix);
        }
        normal*=.0075f;
    }else{
        for(uint sample=0;sample<3&&!record.fault;++sample){
            const uint axis=sample==0?2:sample==1?0:1;const float3 direction=general_column(scene,axis);
            const float3 plus=center+direction*offset,minus=plus+direction*(-2.f*offset);
            const float a=general_normal_sample(plus,stop,direct,direct,program,resources,lane,geometry,record).value;
            const float b=general_normal_sample(minus,stop,direct,direct,program,resources,lane,geometry,record).value;
            normal[axis]=(direct?b-a:a-b)*.5f;
        }
    }
    if(smooth>0&&!record.fault){
        offset*=2.f;
        if(direct||smooth<8){
            float3 point=center+general_column(scene,0)*(-offset);
            middle+=general_normal_sample(point,stop,direct,direct,program,resources,lane,geometry,record).value;
            point+=general_column(scene,0)*(2.f*offset);
            middle+=general_normal_sample(point,stop,direct,direct,program,resources,lane,geometry,record).value;
            point=center+general_column(scene,1)*(-offset);
            middle+=general_normal_sample(point,stop,direct,direct,program,resources,lane,geometry,record).value;
            point+=general_column(scene,1)*(2.f*offset);
            middle=(middle+general_normal_sample(point,stop,direct,direct,program,resources,lane,geometry,record).value)*.2f;
        }
        const float step=offset*3.f/(float(smooth)+.5f),count=float(smooth)*2.f;
        float3 vx,vy;
        if(!isfinite(step)||step<=0||!general_normal_tangents(normal,vx,vy))general_normal_fault(geometry,record,4);
        else{
            vx=general_normal_world(vx,scene);vy=general_normal_world(vy,scene);
            float sums[2]={0,0},squares[2]={0,0};
            for(uint axis=0;axis<2&&!record.fault;++axis){
                const float3 direction=axis?vy:vx;float3 point=center+direction*(-float(smooth)*step);
                for(int i=-smooth;i<=smooth&&!record.fault;++i){
                    if(i){const float value=general_normal_sample(point,stop,direct,direct,program,resources,lane,geometry,record).value;
                        const float delta=(direct?middle-value:value-middle)/float(i);sums[axis]+=delta;squares[axis]+=delta*delta;}
                    point+=direction*step;
                }
            }
            float variance=squares[0]*count-sums[0]*sums[0];variance+=squares[1]*count-sums[1]*sums[1];
            const float correction=offset*.5f/(count*step);
            if(!isfinite(variance)||!isfinite(correction)||!all(isfinite(normal)))general_normal_fault(geometry,record,4);
            else{out.roughness=general_normal_roughness(normal,correction,variance);
                if(smooth<8){normal.x+=sums[0]*correction;normal.y+=sums[1]*correction;}}
        }
    }
    for(uint i=0;i<3;++i)lane.c[i]=center[i];
    if(direct&&params.inside)normal=-normal;
    out.orbit_trap=lane.otrap;
    if(record.sample_calls-initial_samples>kGeneralNormalMaxSamples||!all(isfinite(normal))||
       !isfinite(out.smooth)||!isfinite(out.orbit_trap)||!isfinite(out.roughness))general_normal_fault(geometry,record,4);
    if(!record.fault){
        const float magnitude=max(abs(normal.x),max(abs(normal.y),abs(normal.z)));
        if(magnitude>0){const float3 scaled=normal/magnitude;
            // Preserve each pre-existing Native packing association: primary
            // multiplies by one quotient; reflection divides then multiplies.
            if(reflection_pack){const float3 packed=(scaled/length(scaled))*32767.f;
                for(uint i=0;i<3;++i)out.packed[i]=int(rint(packed[i]));}
            else{const float factor=32767.f/sqrt((scaled.x*scaled.x+scaled.y*scaled.y)+scaled.z*scaled.z);
                for(uint i=0;i<3;++i)out.packed[i]=int(rint(scaled[i]*factor));}}
        if(!out.packed[0]&&!out.packed[1])out.packed[2]=out.packed[2]>0?32767:-32767;
        for(uint i=0;i<3;++i)out.raw_normal[i]=normal[i];
        out.roughness_byte=params.smoothing?uint(rint(clamp(out.roughness,0.f,1.f)*255.f)):0u;
        out.completed=1;++record.completed_evaluations;
    }
    out.fault=record.fault|geometry.fault;return out;
}
} // namespace mb3d_native
