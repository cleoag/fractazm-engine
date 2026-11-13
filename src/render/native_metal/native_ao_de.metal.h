#pragma once
#ifndef MB3D_METAL_NATIVE_FLOAT
#error Ordinary DE AO requires Native GPU arithmetic
#endif
#include "native_ao_de_math.h"
#include "native_ao_de_local.metal.h"
#include "src/render/native_metal/general_geometry.metal.h"
#include "native_camera_abi.h"
#include "pixel_independence.h"

namespace mb3d_native_ao {
using namespace metal;
using namespace mb3d_native;

inline NativeFormulaState de_initial_state(constant NativeFormulaProgram& p) {
    NativeFormulaState s{};
    for(uint i=0;i<4;++i)s.j[i]=s.original_j[i]=p.julia_value[i];
    for(uint i=0;i<6;++i)s.hybrid_counts[i]=p.hybrid_counts[i];
    s.max_iterations=p.max_iterations;s.rstop=p.radius_stop;s.julia=p.julia;
    s.inside=p.inside;s.derivative[0]=1.f;
    if(p.de_option==20)s.rstop_d=-1.0e10f;
    return s;
}
inline bool de_finite(float value,thread DERecord& record) {
    if(!isfinite(value)){record.common.fault|=NonFinite;return false;}return true;
}
inline bool de_finite(float3 value,thread DERecord& record) {
    if(!all(isfinite(value))){record.common.fault|=NonFinite;return false;}return true;
}
inline NativeOrbitResult de_evaluate(float3 point,float stop,uint inside,bool smooth,int iterations,
    constant NativeFormulaProgram& program,thread NativeResources& resources,
    thread NativeFormulaState& lane,thread NativeGeometryRecord& eval,thread DERecord& record,uint limit,
    bool temporary_maximum=false) {
    NativeOrbitResult result{};
    if(record.common.fault||!spend(record.common,limit))return result;
    if(!de_finite(point,record)||!de_finite(stop,record)||stop<=0){record.common.fault|=NonFinite;return result;}
    lane.inside=inside; // a formula's scratch writes do not replace MCT policy
    result=general_de(point,stop,program,resources,lane,eval,smooth,iterations,temporary_maximum);
    record.common.de_queries=eval.de_calls;record.formula_calls=eval.de_calls;
    record.fp32_range_exits=eval.fp32_range_exits;record.difs_carried_reads=eval.difs_carried_reads;
    record.smoothing=eval.smoothing;
    record.formula_work=eval.formula_work;record.formula_fault=eval.formula_fault.code;
    if(eval.fault)record.common.fault|=eval.fault==2?WorkExhausted:eval.fault==4?NonFinite:Invalid;
    de_finite(result.distance,record);return result;
}
inline float de_depth(uint packed,float offset,constant DEParams& p) {
    const float q=(offset-float(packed>>8))/p.z_mul+1.f;
    return (q*q-1.f)/p.z_corr;
}
inline float de_stop(float depth,constant DEParams& p) {
    return p.base_stop*(1.f+depth*p.stop_factor);
}
inline bool de_iteration_refine(thread float3& point,float3 direction,thread float& depth,float stop,
    uint inside,constant DEParams& p,constant NativeFormulaProgram& program,
    thread NativeResources& resources,thread NativeFormulaState& lane,
    thread NativeGeometryRecord& eval,thread DERecord& record) {
    // Full AO explicitly assigns iDEAddSteps=5 regardless of primary settings.
    const float target=float(program.max_iterations)-.99f;
    const int temporary_max=program.max_iterations+(program.de_option!=20?1:0);
    float step=0,multiplier=1,last=0,last_difference=0;
    for(uint pass=0;pass<=5;++pass){
        ++record.refinements;depth+=step;point+=direction*step;
        auto value=de_evaluate(point,stop,inside,true,temporary_max,program,resources,lane,eval,record,p.work_limit,true);
        if(record.common.fault)return false;float smooth=value.state.smooth;
        if(pass&&last_difference<abs(target-smooth)){
            depth-=step;point-=direction*step;smooth=last;multiplier*=step>0?.5f:.7f;
        }
        last_difference=abs(target-smooth);last=smooth;
        if(smooth>float(temporary_max)-.1f)step=-3.f;
        else{
            depth-=.001f;point-=direction*.001f;++record.refinements;
            value=de_evaluate(point,stop,inside,true,temporary_max,program,resources,lane,eval,record,p.work_limit,true);
            if(record.common.fault)return false;
            step=general_boundary_step(target,last,value.state.smooth,multiplier);
        }
        if(!de_finite(step,record)||!de_finite(depth,record)||!de_finite(point,record))return false;
    }
    lane.max_iterations=program.max_iterations;return true;
}
inline ExactSurface de_pixel(ExactSurface surface,mb3d_camera::Ray camera,uint pixel,
    constant DEParams& p,constant ExactScene& scene,constant NativeFormulaProgram& program,
    thread NativeResources& resources,thread NativeFormulaState& lane,thread uint& seed,
    thread DERecord& record) {
    thread auto& r=record.common;r.version=Version;r.algorithm=DistanceEstimate;
    r.initial_ambient=surface.amb_shadow;surface.amb_shadow=0;r.final_ambient=0;r.changed=r.initial_ambient!=0;
    if(surface.fault||surface.reserved[0]||surface.reserved[1]||surface.reserved[2]||
       surface.si_gradient>65535||surface.o_trap>65535||surface.shadow>65535||r.initial_ambient>65535||
       camera.version!=mb3d_camera::Version||camera.fault||
       camera.reserved[0]||camera.reserved[1]||camera.reserved[2]||camera.direction[3]!=0||camera.origin[3]!=0){r.fault=Invalid;surface.fault|=r.fault;return surface;}
    // CPU clears every pixel, including background/high-gradient interior.
    if((surface.depth_packed>>16)>=32768||surface.si_gradient>=32768){record.completed=1;return surface;}
    r.active=1;r.depth_reads=1;lane.calc_smooth=0;
    NativeGeometryRecord eval{};eval.version=kGeneralGeometryVersion;
    const float3 ray=float3(camera.direction[0],camera.direction[1],camera.direction[2]);
    const float3 origin=float3(camera.origin[0],camera.origin[1],camera.origin[2]);
    float depth=de_depth(surface.depth_packed,8388351.5f,p),stop=de_stop(depth,p);
    float3 point=origin+ray*depth;
    const uint inside=de_selected_inside(p.initial_inside,p.in_and_out,surface.o_trap);
    record.selected_inside=uint(inside!=0);
    if(!de_finite(ray,record)||!de_finite(origin,record)||!de_finite(depth,record))return surface;
    auto value=de_evaluate(point,stop,inside,false,program.max_iterations,program,resources,lane,eval,record,p.work_limit);
    if(r.fault)return surface;
    // Doubling a large finite stop cannot constrain any finite distance; avoid
    // manufacturing an infinity before a clamp that would otherwise hide it.
    if(stop<=0x1.fffffep126f)value.distance=min(value.distance,stop*2.f);
    const uint max_result=native_iteration_maximum(program,value);
    if(uint(value.state.iteration)<max_result||value.distance<stop){
        float step=de_depth(surface.depth_packed,8388351.9f,p)-depth;
        if(!de_finite(step,record))return surface;
        for(uint remaining=5;remaining&&abs(value.distance-stop)>.004f;--remaining){
            ++record.refinements;depth+=step;point+=ray*step;stop=de_stop(depth,p);
            if(!de_finite(depth,record)||!de_finite(point,record)||!de_finite(stop,record)||stop<=0){r.fault|=NonFinite;return surface;}
            if(remaining>1){
                value=de_evaluate(point,stop,inside,false,program.max_iterations,program,resources,lane,eval,record,p.work_limit);
                if(r.fault)return surface;step=abs(step)*(value.distance<stop?-.55f:.55f);
            }
        }
    }else{
        if(!de_iteration_refine(point,ray,depth,stop,inside,p,program,resources,lane,eval,record))return surface;
        stop=de_stop(abs(depth),p);
    }
    if(!de_finite(stop,record)||stop<=0){r.fault|=NonFinite;return surface;}
    stop=min(stop,1.e6f);const float3 surface_position=point;
    const float step_ao=stop/p.base_stop,max_distance=p.max_distance_base*sqrt(step_ao);
    // CPU forces bIsIFS=0 for this entire pass, including all inside formulas.
    if(!p.vary_stop)stop=p.base_stop/(p.step_multiplier*p.step_multiplier);
    else if(!inside)stop/=p.step_multiplier*p.step_multiplier;
    if(!de_finite(step_ao,record)||!de_finite(max_distance,record)||max_distance<=0||!de_finite(stop,record)||stop<=0){r.fault|=NonFinite;return surface;}
    float3 directions[33];
    const int x=int(pixel%p.width)+p.left,y=int(pixel/p.width)+p.top;
    const uint count=de_local_rays(p,x,y,directions);
    if(!count||count>33){r.fault|=Invalid;return surface;}
    float de_multiplier=sqrt(float(count)*.5f);
    const float angle_scale=1.2f/asin(clamp(1.f/de_multiplier,-1.f,1.f));
    const float md_denominator=max_distance*de_multiplier;
    if(!de_finite(md_denominator,record)||md_denominator<=0){r.fault|=NonFinite;return surface;}
    const float md=.1f/md_denominator;
    if(inside)de_multiplier=1.f; // MD retains the original multiplier, as CPU.
    if(!de_finite(angle_scale,record)||!de_finite(md,record)){return surface;}
    float3 normal=float3(surface.normal[0],surface.normal[1],surface.normal[2])*3.0518509476e-5f;
    normal/=sqrt((normal.x*normal.x+normal.y*normal.y)+normal.z*normal.z+1.e-30f);
    if(!de_finite(normal,record))return surface;
    const float a=acos(clamp(-normal.z,-1.f,1.f))*.5f,sa=sin(a),ca=cos(a),xy=sqrt(normal.y*normal.y+normal.x*normal.x);
    const float qx=xy<1.e-25f?0.f:-normal.y*(sa/xy),qy=xy<1.e-25f?0.f:normal.x*(sa/xy);
    const float3 rotation[3]={float3(1.f-2.f*qy*qy,2.f*qx*qy,-2.f*ca*qy),
        float3(2.f*qx*qy,1.f-2.f*qx*qx,2.f*ca*qx),float3(2.f*ca*qy,-2.f*ca*qx,1.f-2.f*(qx*qx+qy*qy))};
    float minima[33];
    for(uint i=0;i<count&&!r.fault;++i){
        ++record.rays;const float3 local=directions[i];
        const float3 oriented=float3(dot(local,rotation[0]),dot(local,rotation[1]),dot(local,rotation[2]));
        float3 direction;
        for(uint axis=0;axis<3;++axis)direction[axis]=(oriented.x*p.view_matrix[0][axis]+oriented.y*p.view_matrix[1][axis])+oriented.z*p.view_matrix[2][axis];
        float travel=step_ao*p.step_multiplier,amount=1.f;bool ended=false,first=bool(p.first_step_random);
        if(!de_finite(direction,record)||!de_finite(travel,record)||travel<=0){r.fault|=NonFinite;break;}
        for(uint k=0;k<p.max_steps;++k){
            if(!spend(r,p.work_limit))break;++r.de_steps;bool last=false;
            if(first){first=false;travel*=de_random_factor(seed);++r.rng_advances;}
            else if(travel>max_distance){travel=max_distance;last=true;}
            point=surface_position+direction*travel;
            const float margin=travel*(p.step_width*.5f);
            if(!de_finite(travel,record)||travel<=0||!de_finite(point,record)||!de_finite(margin,record)){r.fault|=NonFinite;break;}
            for(uint axis=0;axis<3;++axis)lane.c[axis]=point[axis];
            if(de_at_cut(point,margin,p)){++r.cut_skips;ended=true;break;}
            value=de_evaluate(point,stop,inside,lane.calc_smooth!=0,program.max_iterations,program,resources,lane,eval,record,p.work_limit);
            if(r.fault)break;
            float next;
            if(inside){
                if(value.distance<stop){const float ratio=travel/max_distance,square=ratio*ratio;amount=square*square;ended=true;break;}
                next=(travel+travel*p.step_multiplier*.1f)+stop;
            }else{
                const float candidate=(value.distance-stop+travel*md)/travel;
                if(!de_finite(candidate,record))break;amount=min(amount,candidate);
                if(amount<.02f){ended=true;break;}
                next=travel+max(value.distance,travel*p.step_multiplier);
            }
            if(!de_finite(next,record))break;
            if(last){ended=true;break;}if(next<=travel){r.fault|=WorkExhausted;break;}travel=next;
        }
        if(!r.fault&&!ended)r.fault|=WorkExhausted;
        if(!de_finite(amount,record))break;minima[i]=max(0.f,amount)*de_multiplier;
        if(!de_finite(minima[i],record))break;
    }
    float amount=0;
    for(uint i=0;i<count&&!r.fault;++i){
        if(inside){amount+=minima[i];continue;}
        float addition=0;
        if(minima[i]<1.f)for(uint j=0;j<count;++j){
            if(!spend(r,p.work_limit))break;if(i==j)continue;
            const float product=dot(directions[i],directions[j]);
            if(!de_finite(product,record))break;
            if(product>p.minimum_dot){
                const float overlap=minima[j]-acos(clamp(product,-1.f,1.f))*angle_scale+1.f;
                if(!de_finite(overlap,record))break;
                if(overlap>0)addition+=min(1.f-minima[i],overlap)*p.correction_weight;
            }
        }
        if(!de_finite(addition,record))break;amount+=min(1.f,addition+minima[i]);
    }
    uint ambient=0;if(!r.fault&&!de_ambient(amount,count,ambient))r.fault|=NonFinite;
    if(!r.fault){surface.amb_shadow=ambient;r.final_ambient=ambient;r.changed=ambient!=r.initial_ambient;r.levels=1;r.repeats=1;record.completed=1;}
    surface.fault|=r.fault;return surface;
}
}

// A command processes at most one pixel per logical CPU task. Cancellation and
// host fault readback occur between commands; no task state is shared by peers.
kernel void native_ao_de_pixel(constant ExactScene& scene [[buffer(0)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(1)]],
    constant mb3d_native_ao::DEParams& p [[buffer(2)]],
    device const mb3d_camera::Ray* camera [[buffer(3)]],device const ExactSurface* input [[buffer(4)]],
    device ExactSurface* output [[buffer(5)]],device mb3d_native_ao::DERecord* records [[buffer(6)]],
    device mb3d_native_ao::DETaskState* states [[buffer(7)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(8)]],device const float* pixels [[buffer(9)]],
    constant uint4& allocation [[buffer(10)]],constant uint& reset [[buffer(11)]],uint task [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;using namespace mb3d_native;
    if(task>=allocation.y)return;DETaskState state{};
    if(reset){state.version=Version;state.seed=p.initial_seed;state.lane=de_initial_state(program);}
    else state=states[task];
    if(!valid_de(p)||allocation.x!=p.width*p.height||allocation.y!=p.task_count||reset>1||
       !valid_general_scene(scene,program)||scene.width!=p.width||scene.height!=p.height||
       state.version!=Version||state.reserved[0]||state.reserved[1]||state.reserved[2]){state.fault|=Invalid;states[task]=state;return;}
    if(state.fault||state.finished)return;
    const uint first_row=task*p.task_rows,rows=min(p.task_rows,p.height-first_row),count=rows*p.width;
    if(state.cursor>=count||state.cursor!=state.processed){state.fault|=Bounds;states[task]=state;return;}
    const uint pixel=first_row*p.width+state.cursor;
    if(pixel>=allocation.x){state.fault|=Bounds;states[task]=state;return;}
    NativeResources resources{maps,pixels,allocation.z,allocation.w,nullptr};DERecord record{};
    auto surface=de_pixel(input[pixel],camera[pixel],pixel,p,scene,program,resources,state.lane,state.seed,record);
    surface.fault|=record.common.fault;records[pixel]=record;output[pixel]=surface;
    if(record.common.fault||!record.completed)state.fault|=record.common.fault?record.common.fault:Invalid;
    else{++state.cursor;++state.processed;state.finished=state.cursor==count;}
    states[task]=state;
}
kernel void native_ao_de_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=mb3d_native_ao::DEVersion;words[1]=sizeof(mb3d_native_ao::DEParams);
    words[2]=sizeof(mb3d_native_ao::DERecord);words[3]=sizeof(mb3d_native_ao::DETaskState);words[4]=32;
}

// Separate independent-pixel entry: either a closed audited depth chain
// (proof 1), which the device re-derives, or a host proof it cannot re-derive
// from the program: the independent primary state contract (5), a closed
// primary chain (6) or the conditional dIFS contract (7), whose records'
// difs_carried_reads the host checks. Keep each pixel's
// refinements/rays/DE calls ordered. ABI v5: a random first step (seeded=1)
// takes each pixel's entering seed of its CPU task, planned by the host from
// the rays every active pixel draws; the pixel's exit seed must equal its
// task successor's entering seed, or the task's exit after the pixel words.
kernel void native_ao_de_pixels(constant ExactScene& scene [[buffer(0)]],
    constant mb3d_native::NativeFormulaProgram& program [[buffer(1)]],
    constant mb3d_native_ao::DEParams& p [[buffer(2)]],
    device const mb3d_camera::Ray* camera [[buffer(3)]],device const ExactSurface* input [[buffer(4)]],
    device ExactSurface* output [[buffer(5)]],device mb3d_native_ao::DERecord* records [[buffer(6)]],
    device const mb3d_native::NativeMapDescriptor* maps [[buffer(8)]],device const float* pixels [[buffer(9)]],
    constant uint4& allocation [[buffer(10)]],constant mb3d_native_ao::Launch& launch [[buffer(11)]],
    constant uint& proof [[buffer(12)]],device const uint* seeds [[buffer(13)]],constant uint& seeded [[buffer(14)]],
    uint tid [[thread_position_in_grid]]) {
    using namespace mb3d_native_ao;using namespace mb3d_native;
    if(tid>=launch.count || launch.offset>=allocation.x || tid>=allocation.x-launch.offset)return;
    const uint pixel=launch.offset+tid;DERecord record{};
    if(!valid_de(p)||seeded>1||seeded!=p.first_step_random||(proof==kDEProofClosedDepth?!IndependentDepthScenePixels(program):
           proof!=kDEProofStateContract&&proof!=kDEProofClosedChain&&proof!=kDEProofConditionalState)||
       allocation.x!=p.width*p.height||allocation.y!=p.task_count||
       !valid_general_scene(scene,program)||scene.width!=p.width||scene.height!=p.height||
       launch.allocated_pixels!=allocation.x||launch.reserved||launch.count>kDEPixelBatch||launch.count>allocation.x-launch.offset) {
        record.common.fault=Invalid;records[pixel]=record;
        auto failed=input[pixel];failed.fault|=Invalid;output[pixel]=failed;return;
    }
    auto lane=de_initial_state(program);uint seed=seeded?seeds[pixel]:p.initial_seed;
    NativeResources resources{maps,pixels,allocation.z,allocation.w,nullptr};
    auto surface=de_pixel(input[pixel],camera[pixel],pixel,p,scene,program,resources,lane,seed,record);
    if(seeded&&!record.common.fault){
        const uint task=pixel/p.width/p.task_rows,end=min((task+1)*p.task_rows,p.height)*p.width;
        if(task>=p.task_count||seeds[pixel+1<end?pixel+1:allocation.x+task]!=seed)record.common.fault|=Invalid;
    }
    surface.fault|=record.common.fault;records[pixel]=record;output[pixel]=surface;
}
kernel void native_ao_de_pixels_abi(device uint* words [[buffer(0)]],uint tid [[thread_position_in_grid]]) {
    if(tid)return;words[0]=5;words[1]=sizeof(mb3d_native_ao::Launch);
    words[2]=sizeof(mb3d_native_ao::DEParams);words[3]=sizeof(mb3d_native_ao::DERecord);words[4]=mb3d_native_ao::kDEPixelBatch;
}
