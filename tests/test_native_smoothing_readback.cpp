#include "render/native_metal/smoothing_abi.h"
#include "render/native_metal/smoothing_readback.h"
#include "render/native_metal/primary_range_readback.h"
#include "render/native_metal/general_geometry_abi.h"
#include "render/native_metal/generic_shadow_abi.h"
#include "render/native_metal/generic_reflection_abi.h"
#include "render/native_metal/native_ao_de_abi.h"
#include "render/native_metal/reflection_work_readback.h"
#include <iostream>
#include <stdexcept>
using namespace mb3d_native;
int main(){try{
    unsigned checks=0;auto need=[&](bool b){++checks;if(!b)throw std::runtime_error("smoothing record validation");};
    need(sizeof(NativeGeometryRecord)==112&&kGeneralGeometryVersion==6);
    need(sizeof(NativeShadowRecord)==160&&kGenericShadowVersion==3);
    need(sizeof(NativeGenericReflectionRecord)==176&&kGenericReflectionVersion==3);
    need(sizeof(mb3d_native_ao::DERecord)==128&&mb3d_native_ao::DEVersion==4);
    need(valid_smoothing_counters({0,0,0,0},0,1));
    need(valid_smoothing_counters({4,1,2,0},9,1));
    need(valid_smoothing_counters({4,0,0,0},9,0));
    need(!valid_smoothing_counters({4,1,2,0},9,0));
    need(!valid_smoothing_counters({4,1,2,0},3,1));
    need(!valid_smoothing_counters({4,2,3,0},9,1));
    need(!valid_smoothing_counters({4,1,2,1},9,1));
    need(valid_smoothing_counters({4,1,2,1},9,1,true));
    need(!valid_smoothing_counters({4,1,2,2},9,1,true));
    need(!valid_smoothing_counters({0,0,0,0},0,2));
    need(!valid_smoothing_counters({~0u,~0u,~0u,~0u},~0u,1,true));
    need(!valid_smoothing_counters({kNativeSmoothingMaxCalls+1,0,0,0},~0u,1));
    NativeGeometryRecord g{};g.version=6;g.formula_work=5;g.de_calls=1;g.fault=4;
    g.smoothing={2,0,1,1};need(valid_smoothing_observation(g,1));
    need(!valid_smoothing_counters(g.smoothing,g.formula_work,1)); // never success
    g.gradient_range_clamps=1;need(valid_smoothing_observation(g,1));
    g.fp32_range_exits=kNativeRangeExitsPerDistance;need(valid_smoothing_observation(g,1));
    for(unsigned which=0;which<10;++which){auto bad=g;
        if(which==0)bad.version=2;
        if(which==1)bad.formula_work=kGeneralMaxFormulaWork+1;
        if(which==2)bad.de_calls=kGeneralMaxDistanceCalls+1;
        if(which==3)bad.difs_carried_reads=2*(bad.de_calls+bad.normal_calls)+1; // at most two orbits per query
        if(which==4)bad.formula_fault.reserved[1]=1;
        if(which==5)bad.smoothing.calls=6;
        if(which==6)bad.gradient_range_clamps=2;
        if(which==7)bad.version=5;
        if(which==8)bad.fp32_range_exits=kNativeRangeExitsPerDistance+1; // one DE query, no direct normal orbit
        if(which==9){bad.normal_calls=1;bad.fp32_range_exits=kNativeRangeExitsPerDistance+2;}
        need(!valid_smoothing_observation(bad,1));
    }
    mb3d_inside::ReflectionRecord inside{};inside.version=1;inside.initial_primary=1;
    NativeReflectionRecord ray{};ray.active=ray.rays=ray.bounces=ray.misses=1;
    NativeGeometryRecord empty{};empty.version=kGeneralGeometryVersion;
    need(mb3d_inside::initial_open_air_without_de(true,32768,inside,ray,empty));
    auto nonempty=empty;nonempty.gradient_range_clamps=1;
    need(!mb3d_inside::initial_open_air_without_de(true,32768,inside,ray,nonempty));
    nonempty=empty;nonempty.fp32_range_exits=1;
    need(!mb3d_inside::initial_open_air_without_de(true,32768,inside,ray,nonempty));
    for(unsigned word=0;word<4;++word)for(U32 bits:{1u,0x80000000u,0x7fc00000u}){
        auto bad=empty;
        if(word==0)bad.hit_color.version=bits;if(word==1)bad.hit_color.rout=std::bit_cast<float>(bits);
        if(word==2)bad.hit_color.rold=std::bit_cast<float>(bits);if(word==3)bad.hit_color.reserved=bits;
        need(!mb3d_inside::initial_open_air_without_de(true,32768,inside,ray,bad));
    }
    // A primary pass may have real range exits and a separate corrupt record.
    // The corruption must win over the whole-frame CPU range policy.
    NativeGeometryRecord rg{};rg.version=kGeneralGeometryVersion;rg.de_calls=rg.formula_work=rg.fp32_range_exits=1;
    ExactPixel rp{};ExactSurface rs{};mb3d_camera::Ray primaryRay{};primaryRay.version=mb3d_camera::Version;
    NativeNormalParams np{};np.version=kGeneralNormalVersion;
    NativeNormalRecord nr{};nr.version=kGeneralNormalVersion;NativeNormalResult nv{};
    mb3d_inside::Params ip{};ip.version=1;ip.full_width=1;
    mb3d_inside::Record ir{};ir.version=ir.attempts=1;
    auto valid=[&](const auto& gg,const auto& pp,const auto& ss,const auto& rr,const auto& nn,
                   const auto& vv,const auto& ii){
        return primary_range_error(gg,pp,ss,rr,0,100.,np,nn,vv,ip,ii,1,0)==nullptr;};
    need(valid(rg,rp,rs,primaryRay,nr,nv,ir));
    for(unsigned poison=0;poison<20;++poison){
        auto gg=rg;auto pp=rp;auto ss=rs;auto rr=primaryRay;auto nn=nr;auto vv=nv;auto ii=ir;
        if(poison==0)gg.fault=1;
        if(poison==1)gg.formula_fault.code=NonFiniteFormula;
        if(poison==2)gg.initial_rout=std::numeric_limits<float>::infinity();
        if(poison==3)gg.fp32_range_exits=kNativeRangeExitsPerDistance+1;
        if(poison==4)pp.fault=1;
        if(poison==5)pp.position[1]=0x7ff8000000000000ull;
        if(poison==6)pp.final_stop=0x7f800000;
        if(poison==7)pp.reserved_tail=1;
        if(poison==8)ss.fault=1;
        if(poison==9)ss.reserved[2]=1;
        if(poison==10)ss.depth_packed=1;
        if(poison==11)rr.fault=1;
        if(poison==12)rr.cut_axis=4;
        if(poison==13)rr.direction[0]=std::numeric_limits<float>::infinity();
        if(poison==14)nn.fault=1;
        if(poison==15)nn.sample_calls=1;
        if(poison==16)vv.reserved[3]=1;
        if(poison==17)vv.raw_normal[1]=std::numeric_limits<float>::quiet_NaN();
        if(poison==18)ii.fault=1;
        if(poison==19)ii.attempts=2;
        need(!valid(gg,pp,ss,rr,nn,vv,ii));
    }
    std::cout<<"PASS "<<checks<<" smoothing ABI/readback controls\n";return 0;
}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
