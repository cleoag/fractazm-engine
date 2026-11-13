#include "render/native_metal/postprocess_plan.h"
#include "render/native_metal/image_execution_policy.h"
#include "render/native_metal/restricted_execution_policy.h"
#include <iostream>
#include <limits>

using namespace mb3d::native_metal;
static unsigned checks=0;
static void check(bool ok) {++checks;if(!ok)throw std::runtime_error("postprocess plan check failed");}
template<class F> void rejected(F f) {
    bool threw=false;try{f();}catch(const std::invalid_argument&){threw=true;}check(threw);
}
int main() {try {
    mb3d_dof::Params d{};mb3d_zn::Params z{};
    check(PlanNativeDOF(d).bytes()==0);check(PlanNativeZNormals(z).bytes()==0);
    d.width=1;rejected([&]{PlanNativeDOF(d);});d={};
    z.count=1;rejected([&]{PlanNativeZNormals(z);});z={};
    d.version=1;d.width=64;d.height=48;d.passes=1;d.clip_radius=1;
    d.aperture=.1f;d.inverse_z_mul=1;d.inverse_z_corr=1;
    auto p=PlanNativeDOF(d);
    const DOFExecutionPolicy dp(d);
    check(dp.allocation().bytes()==p.bytes());
    check(dp.spatial().shared_group_memory&&dp.spatial().group_width==256);
    check(dp.sort().order==ImageOrder::QuickSortPermutation&&dp.finish().order==ImageOrder::QuantizeEachPass);
    check(dp.use_spatial(24)&&!dp.use_spatial(25));
    auto dchanged=d;dchanged.focus_low=-0.f;rejected([&]{dp.validate(dchanged,{});});
    DOFLimits changed;changed.gather_slice=256;rejected([&]{dp.validate(d,changed);});
    check(p.pixels==3072&&p.candidate_capacity==400); // Odd-radius tie requires a two-pixel bound.
    check(p.ranks==12288&&p.spatial_records==128);
    check(p.sources==98304&&p.order==24576&&p.global==32&&p.pass==32&&p.sort_state==320);
    check(p.tiles==256&&p.candidates==12800&&p.accumulators==65536&&p.rgb==49152&&p.marker==36);
    check(p.bytes()==312612); // Captured ABI budget, including ranks and spatial records.
    DOFLimits limits;limits.spatial_candidates=false;
    check(PlanNativeDOF(d,limits).bytes()==p.bytes()); // Both buffers are allocated in either policy.
    d.width=d.height=1;p=PlanNativeDOF(d);check(p.pixels==1&&p.candidate_capacity==1);
    d.width=d.height=4096;d.clip_radius=1000;limits.tile_batch=64;p=PlanNativeDOF(d,limits);
    check(p.pixels==16777216&&p.candidate_capacity==4064256);
    check(p.bytes()>(uint64_t(1)<<30)); // Planner reports demand; runtime must still enforce its cap.
    const auto valid=d;
    for(auto x:{0u,4097u,std::numeric_limits<uint32_t>::max()}) {
        d=valid;d.width=x;rejected([&]{PlanNativeDOF(d);});
    }
    for(auto x:{0.f,-1.f,std::numeric_limits<float>::infinity(),std::numeric_limits<float>::quiet_NaN()}) {
        d=valid;d.clip_radius=x;rejected([&]{PlanNativeDOF(d);});
    }
    d=valid;limits.tile_batch=0;rejected([&]{PlanNativeDOF(d,limits);});
    limits={};limits.sort_work_limit=0;rejected([&]{PlanNativeDOF(d,limits);});
    d.reserved1=1;rejected([&]{PlanNativeDOF(d);});
    for(auto passes:{0u,5u}) {d=valid;d.passes=passes;rejected([&]{PlanNativeDOF(d);});}
    d=valid;limits={};limits.tile_batch=65;rejected([&]{PlanNativeDOF(d,limits);});
    for(auto member:{&DOFLimits::source_slice,&DOFLimits::gather_slice,&DOFLimits::sort_slice}) {
        limits={};limits.*member=65537;rejected([&]{PlanNativeDOF(d,limits);});
    }
    d.focus_low=1;d.focus_high=0;rejected([&]{PlanNativeDOF(d);});
    d=valid;d.clip_radius=.5f;
    check(PlanNativeDOF(d).candidate_capacity==324);
    d.clip_radius=std::nextafter(.5f,0.f);check(PlanNativeDOF(d).candidate_capacity==324);
    d.clip_radius=std::nextafter(.5f,1.f);
    // The binary32 addition rounds this first neighbor to 1; the second crosses it.
    check(PlanNativeDOF(d).candidate_capacity==324);
    d.clip_radius=std::nextafter(d.clip_radius,1.f);check(PlanNativeDOF(d).candidate_capacity==400);
    z.version=1;z.width=z.full_width=64;z.height=z.full_height=48;z.count=3072;
    z.step=z.aspect=z.optic_z=z.inverse_z_mul=z.depth_correction=1;
    auto zp=PlanNativeZNormals(z);check(zp.bytes()==344088);
    const ZNormalsExecutionPolicy zn(z);check(zn.allocation().bytes()==zp.bytes());
    check(zn.reconstruction_and_normals().order==ImageOrder::ReconstructThenNeighbors);
    auto zm=z;zm.mode=1;rejected([&]{zn.validate(zm);});
    check(zp.coordinates==98304&&zp.records==98304&&zp.surface==147456);
    ++z.count;rejected([&]{PlanNativeZNormals(z);});--z.count;
    z.stop_factor=std::numeric_limits<float>::quiet_NaN();rejected([&]{PlanNativeZNormals(z);});
    z.stop_factor=0;z.reserved[2]=1;rejected([&]{PlanNativeZNormals(z);});
    z.reserved[2]=0;const auto zv=z;
    z.mode=4;rejected([&]{PlanNativeZNormals(z);});
    z=zv;z.full_width=4097;rejected([&]{PlanNativeZNormals(z);});
    for(auto offset:{-4097,4097}) {
        z=zv;z.left=offset;rejected([&]{PlanNativeZNormals(z);});
        z=zv;z.top=offset;rejected([&]{PlanNativeZNormals(z);});
    }
    const RestrictedExecutionRequest rr{64,48,true,true,true};
    const RestrictedExecutionPolicy restricted(rr);restricted.validate(rr);
    check(restricted.formula_domain()==StateDomain::RestrictedPrivateOrbit);
    check(restricted.dispatch().normalMode=="parallel"&&restricted.dispatch().shadowMode=="parallel");
    check(restricted.dispatch().primaryRows==256&&restricted.dispatch().primaryThreads==32&&
        restricted.dispatch().primaryTileWidth==8&&restricted.reflection_batch()==4096);
    check(restricted.shadow_owner()==LogicalOwner::WholeRow&&restricted.normal_probes_per_pixel()==6);
    auto rm=rr;rm.ambient=false;rejected([&]{restricted.validate(rm);});
    check(restricted.describe().starts_with("restricted-v1|"));
    using CommandMode=NativeImageCommandPolicy::Mode;
    const NativeImageCommandPolicy serial(CommandMode::Serial),bounded;
    for(auto phase:{NativeImagePhase::ScreenBegin,NativeImagePhase::ScreenWavelet,
                   NativeImagePhase::ThresholdEncode,NativeImagePhase::ThresholdWavelet}) {
        check(!serial.grouped(phase)&&bounded.grouped(phase));
    }
    check(!bounded.grouped(NativeImagePhase::Standalone));
    check(bounded.limits().encoders==8&&bounded.limits().invocations==65536);
    for(auto encoders:{0u,9u})rejected([&]{NativeImageCommandPolicy p(CommandMode::BoundedSlices,{encoders,65536});});
    for(auto invocations:{16383ull,65537ull})rejected([&]{NativeImageCommandPolicy p(CommandMode::BoundedSlices,{8,invocations});});
    rejected([&]{NativeImageCommandPolicy p(static_cast<CommandMode>(2));});
    rejected([&]{bounded.grouped(static_cast<NativeImagePhase>(5));});
    std::cout<<"PASS native postprocess planning checks="<<checks<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}}
