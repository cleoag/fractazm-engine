#include "render/native_metal/render_plan.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "io/m3p_parser.h"
#include "core/numeric_precision.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/ray_marcher.h"
#include "core/reflection.h"
#include "core/render_context.h"
#include "render/native_metal/native_inside_math.h"
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <type_traits>
#include <limits>
#include <bit>
#include <filesystem>
#include <fstream>
#include <unistd.h>

using namespace mb3d;
using namespace mb3d::native_metal;
static unsigned checks=0;
static unsigned stateContracts=0,independentContracts=0;
static void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
template<class F> static void rejects_policy(F f){bool rejected=false;
    try{f();}catch(const std::invalid_argument&){rejected=true;}check(rejected,"stale execution policy admitted");}
static void execution_contract(const PreparedRenderPlan& plan){
    const auto& p=plan.scene.formulas.device;
    const auto* second=plan.scene.formulas.program_count()>1?&plan.scene.formulas.second:nullptr;
    // Frozen49 selection oracle: independent entry proofs must not widen
    // shadow/DEAO state ownership; reflections take pixel ownership only from
    // a fresh-lane primary state. These are scheduling fixtures, not
    // scene-admission overrides or newly qualified formula chains.
    for(uint32_t workers:{1u,std::min(6u,plan.scene.base.device.height),plan.scene.base.device.height})
    for(unsigned flags=0;flags<64;++flags){
        auto r=NativeGeneralExecutionRequest(plan);r.workers=workers;
        r.shadows=flags&1;r.soft_shadows=flags&2;r.reflections=flags&4;
        r.maps_empty=!(flags&8);r.in_and_out=(flags&16)?1:0;r.conditional_state=flags&32;
        GeneralExecutionPolicy policy(p,r,second);policy.validate(p,r,second);
        // An independent primary contract replaces the legacy primary order
        // and the persistent shadow rows. Soft-shadow rows carry only the
        // lane, so wherever they would start fresh lanes they take pixels.
        const auto state=mb3d_native::AnalyzePrimaryPixelState(p,second);
        // A conditional (dIFS) contract applies only when the request admits it.
        const bool independent=state.independent&&(!state.conditional||r.conditional_state);
        const bool statePixels=r.legacy_formula_persistent&&!state.production_chain&&independent;
        const bool primary=r.legacy_formula_persistent&&!state.production_chain&&!statePixels;
        const bool soft=r.shadows&&r.soft_shadows&&r.maps_empty&&
            mb3d_native::AnalyzeSoftShadowPixelState(p,r.initial_inside,r.in_and_out).production_chain;
        const bool shadow=r.legacy_formula_persistent&&!soft&&!statePixels;
        const bool softHost=r.shadows&&r.soft_shadows&&!soft&&!shadow;
        const bool reflection4D=!r.legacy_formula_persistent&&
            (p.driver==mb3d_native::Hybrid4D||p.driver==mb3d_native::Hybrid4DAnalytic);
        const bool reflection=!r.legacy_formula_persistent||independent;
        check(policy.conditional_state()==(r.legacy_formula_persistent&&independent&&state.conditional),
              "conditional state selection changed");
        const uint32_t rows=workers==1?r.height:std::max(1u,r.height/(workers*4)),tasks=(r.height+rows-1)/rows;
        check(policy.independent_primary_state()==statePixels&&policy.primary().proof==
            (state.production_chain?SelectionProof::ClosedPrimaryEntry:statePixels?SelectionProof::IndependentPrimaryState:
             primary?SelectionProof::ConservativeLegacyOrder:SelectionProof::LegacyQualifiedPixels),"primary proof changed");
        check(policy.primary_persistent()==primary&&policy.shadow_persistent()==shadow&&
            policy.parallel_soft_shadows()==(soft||softHost)&&policy.parallel_reflections()==reflection,"entry selection changed49");
        check(policy.soft_shadow_pixel_proof()==(soft?1u:!softHost?0u:statePixels?5u:6u),"soft-shadow pixel proof changed");
        check(policy.shadow_rows()==rows&&policy.shadow_tasks()==tasks,"logical row ownership changed49");
        // Only whole-row hard shadows (single or multi-light) rebuild their
        // row caches from fresh-entry pixel traces.
        check(policy.speculative_shadows()==(r.shadows&&!r.soft_shadows&&!shadow),"speculative shadow selection changed");
        const bool multi=r.shadows&&!r.soft_shadows&&(r.light_mask!=1||r.position_mask);
        const bool wide=multi||r.soft_shadows||statePixels||mb3d_native::IndependentDepthScenePixels(p);
        check(policy.shadow().dispatch_batch==(soft||softHost?kPixelDispatchBatch:shadow?tasks:wide?512u:64u)&&
            policy.shadow().group_width==(soft||softHost?32u:shadow?1u:wide?8u:1u),"shadow schedule changed49");
        check(policy.primary().dispatch_batch==(primary?workers:kPixelDispatchBatch)&&
            policy.primary().group_width==(primary?1u:32u),"primary schedule changed49");
        check(policy.reflection().dispatch_batch==(reflection?kPixelDispatchBatch:tasks)&&
            policy.reflection().group_width==(reflection?32u:1u),"reflection schedule changed49");
        check(policy.reflection().proof==(reflection4D?SelectionProof::LegacyQualified4DReflection:
            reflection?SelectionProof::PixelReflectionState:SelectionProof::ConservativeLegacyOrder),"reflection proof changed");
        check(policy.reflection().initialization==Initialization::ZeroThenPartialActive&&
            policy.reflection().fault_output==FaultOutput::PreserveIncomingRGB,"reflection commit changed");
        check(policy.shadow().fault_flow==(shadow?OwnerFaultFlow::StopContinuation:
            soft||softHost?OwnerFaultFlow::SingleAttempt:OwnerFaultFlow::ContinueWholeRow),"row fault flow lost");
        auto altered=r;altered.maps_empty=!r.maps_empty;rejects_policy([&]{policy.validate(p,altered);});
        auto q=p;q.slots[0].param_bits[0][1]^=1;rejects_policy([&]{policy.validate(q,r);});
        q=p;q.julia_value[0]=std::bit_cast<float>(std::bit_cast<uint32_t>(q.julia_value[0])^0x80000000u);
        rejects_policy([&]{policy.validate(q,r);});
    }
    const auto res=PlanNativeRenderResources(plan,{uint64_t(1)<<32,uint64_t(8)<<30});
    check(res.execution&&res.dof_execution&&res.znormal_execution,"missing entry policies");
    const auto description=DescribeNativeExecutionPolicy(res);
    check(description.starts_with("general-v1|")&&description==DescribeNativeExecutionPolicy(res),"unstable policy metadata");
    if(res.distance_execution){
        const auto& policy=*res.distance_execution;const auto& d=plan.distance_ao;
        const bool closed=mb3d_native::IndependentDepthScenePixels(p);
        const bool chain=!closed&&!second&&mb3d_native::IndependentPrimaryPixels(p);
        const auto state=mb3d_native::AnalyzePrimaryPixelState(p,second);
        const bool contract=!closed&&!chain&&state.independent;
        check(policy.independent()==(closed||chain||contract)&&policy.seeded()==(policy.independent()&&d.first_step_random),
              "DEAO selection changed49");
        check(policy.conditional_state()==(policy.independent()&&contract&&state.conditional),"DEAO conditional state changed");
        check(policy.pixel_proof()==(!policy.independent()?0u:closed?1u:chain?6u:state.conditional?7u:5u),"DEAO device proof changed");
        const DistanceAOExecutionPolicy fallback(p,d,second,true);
        check(!fallback.conditional_state()&&!fallback.independent()&&!fallback.pixel_proof(),
              "DEAO ordered fallback kept the pixel owner");
        // A random first step keeps the pixel owner with host-planned seeds.
        auto random=d;random.first_step_random=1;DistanceAOExecutionPolicy seeded(p,random,second);
        check(seeded.independent()==policy.independent()&&seeded.seeded()==seeded.independent()&&
            seeded.pixel_proof()==policy.pixel_proof()&&seeded.entry().initialization==Initialization::DEInitialState,
            "random DEAO changed pixel ownership");
        const DistanceAOExecutionPolicy randomFallback(p,random,second,true);
        check(!randomFallback.seeded()&&!randomFallback.independent()&&!randomFallback.conditional_state(),"random DEAO ordered fallback");
        ++random.initial_seed;rejects_policy([&]{seeded.validate(p,random,second);});
        if(second){auto other=*second;++other.max_iterations;rejects_policy([&]{policy.validate(p,d,&other);});}
    }
    if(res.screen_execution){const auto& a=*res.screen_execution;
        a.validate(plan.screen_ao);auto p=plan.screen_ao;
        check(a.raster_slice_batch()==(p.params.algorithm==2?16384u:1024u)&&a.rng().continuation==128&&
            a.median().continuation==256,"AO phase bounds changed49");
        check(a.rng().group_width==0,"Unqualified AO grouping changed the production default");
        check(a.uses_certificates()==mb3d_native_ao::random_mode(p.params.algorithm)&&!a.force_unknown(),"Production AO certificate policy mismatch");
        check(!ScreenAOExecutionPolicy(p).uses_certificates(),"Direct AO policy lost ordered control");
        rejects_policy([&]{ScreenAOExecutionPolicy(p,true,ScreenAOExecutionPolicy::LaneGrouping::LegacySimd,NativeImageCommandPolicy{},static_cast<ScreenAOExecutionPolicy::SeedGeometry>(3));});
        rejects_policy([&]{ScreenAOExecutionPolicy(plan.screen_ao,true,static_cast<ScreenAOExecutionPolicy::LaneGrouping>(2));});
        p.params.directions[0]=std::bit_cast<float>(std::bit_cast<uint32_t>(p.params.directions[0])^0x80000000u);
        rejects_policy([&]{a.validate(p);});p=plan.screen_ao;
        if(!p.levels.empty()){p.levels[0].repeat_index^=1;rejects_policy([&]{a.validate(p);});}
    }
}
template<class T> static void same(const T& a,const T& b,const char* message) {
    static_assert(std::is_trivially_copyable_v<T>);check(!std::memcmp(&a,&b,sizeof(T)),message);
}
static void pixel_state_contract(const mb3d_native::NativeFormulaProgram& p) {
    using namespace mb3d_native;
    const auto c=AnalyzePrimaryPixelState(p);
    if(!c.production_chain)return;
    ++stateContracts;
    check(c.known&&c.independent&&c.entry==1&&c.version==1,"incomplete state contract");
    check(!(c.formula_reads_before_write&~(c.initialized|c.live_inputs)),"uninitialized semantic read");
    check(!(c.formula_may_write&c.live_inputs),"mutable live input admitted");
    check(c.scope==1&&c.caller_owned_controls==pixel_state::CallerControls,"caller ownership lost");
    auto reject=[](const auto& value){check(!AnalyzePrimaryPixelState(value).production_chain,"changed descriptor retained production proof");};
    for(unsigned i=0;i<6;++i){
        auto q=p;q.slots[i].version=2;reject(q);
        q=p;q.hybrid_counts[i]^=1;reject(q);
        if(!p.slots[i].enabled)continue;
        q=p;q.slots[i].namespace_id^=1;reject(q);
        q=p;++q.slots[i].id;reject(q);
        q=p;++q.slots[i].de_option;reject(q);
        q=p;++q.slots[i].param_count;reject(q);
        q=p;q.slots[i].traits=1;reject(q);
        q=p;q.slots[i].wrapper_flags^=1;reject(q);
        q=p;q.slots[i].difs_style=1;reject(q);
        q=p;q.slots[i].enabled=0;reject(q);
    }
    auto q=p;q.version=2;reject(q);
    q=p;q.driver=HybridDifs;reject(q);
    q=p;q.de_option=32;reject(q);
    q=p;q.slot_count=5;reject(q);
    q=p;q.start_slot=1;reject(q);
    q=p;q.repeat_slot=p.repeat_slot?0:1;reject(q);
    q=p;q.end_slot=0;reject(q);
    // An audited set in a new order loses the closed descriptor proof; the
    // execution policy may still admit it through the independent contract.
    q=p;std::swap(q.slots[0],q.slots[2]);reject(q);
}
static mb3d_native::NativeFormulaSlot slot(mb3d_native::U32 ns,mb3d_native::U32 id,mb3d_native::I32 de,
    mb3d_native::U32 params,mb3d_native::U32 traits,mb3d_native::U32 flags){
    mb3d_native::NativeFormulaSlot s{};s.version=1;s.enabled=1;s.namespace_id=ns;s.id=id;s.de_option=de;
    s.param_count=params;s.traits=traits;s.wrapper_flags=flags;return s;
}
static void generated_contract_fixtures(){
    // Loaded-program metadata of BulboxCut, JuliaSetSample and QuatP4hybridJulia.
    // Compiler-derived masks complete the contract without a descriptor chain;
    // an independent contract replaces only the legacy persistent primary order.
    using namespace mb3d_native;
    auto program=[](U32 driver,bool julia,std::initializer_list<NativeFormulaSlot> slots,std::initializer_list<I32> counts){
        NativeFormulaProgram p{};p.version=1;p.slot_count=6;p.driver=driver;p.julia=julia;
        for(auto& s:p.slots)s.version=1; // the host uploads disabled slots with the ABI version too
        unsigned i=0;for(const auto& s:slots)p.slots[i++]=s;i=0;for(I32 c:counts)p.hybrid_counts[i++]=c;
        p.end_slot=U32(slots.size()-1);return p;};
    const auto bulbox=program(Hybrid3D,false,{slot(InternalFormula,5,0,0,0,0)},{1});
    const auto julia=program(Hybrid3D,false,{slot(ExternalFormula,10003,-1,2,2112,1),slot(InternalFormula,0,0,0,0,0)},
        {std::bit_cast<I32>(0x80000001u),100});
    const auto quat=program(Hybrid3D,true,{slot(InternalFormula,2,4,0,0,0),slot(InternalFormula,0,0,0,0,0)},{1,2});
    auto request=[](bool legacy){GeneralExecutionRequest r{};r.width=64;r.height=48;r.workers=6;
        r.legacy_formula_persistent=legacy;r.shadows=true;r.reflections=true;return r;};
    for(const auto* p:{&bulbox,&julia,&quat}){
        const auto c=AnalyzePrimaryPixelState(*p);
        check(c.known&&c.generated&&c.independent&&!c.production_chain,"generated contract");
        check(!(c.formula_reads_before_write&~(c.initialized|c.live_inputs)),"generated uninitialized read");
        check(!c.body_must_write_union,"generated must-write claimed");
        const GeneralExecutionPolicy admitted(*p,request(true)),legacy(*p,request(false));
        check(admitted.independent_primary_state()&&!admitted.primary_persistent()&&
            admitted.primary().tasks==64*48,"independent contract not admitted");
        // The explicit secondary extension: shadow rows leave the persistent
        // tasks (one fresh lane per whole row); each reflection pixel starts
        // from a fresh lane with its bounces kept in order.
        check(!admitted.shadow_persistent()&&admitted.shadow().owner==LogicalOwner::WholeRow&&
            admitted.shadow().proof==SelectionProof::IndependentPrimaryState&&admitted.shadow().tasks==48&&
            !admitted.parallel_soft_shadows()&&admitted.reflection().owner==LogicalOwner::Pixel&&
            admitted.reflection().proof==SelectionProof::PixelReflectionState&&admitted.reflection().tasks==64*48,
            "secondary contract extension changed");
        // Whole-row hard shadows, single or multi-light, take speculative rows.
        auto multi=request(true);multi.light_mask=5;
        check(admitted.speculative_shadows()&&legacy.speculative_shadows()&&
            GeneralExecutionPolicy(*p,multi).speculative_shadows(),"whole-row shadows not speculative");
        mb3d_native_ao::DEParams ao{};ao.width=64;ao.height=48;
        const DistanceAOExecutionPolicy pixels(*p,ao);ao.first_step_random=1;const DistanceAOExecutionPolicy seeded(*p,ao);
        check(pixels.independent()&&pixels.pixel_proof()==5&&pixels.entry().proof==SelectionProof::IndependentPrimaryState&&
            !pixels.seeded()&&seeded.independent()&&seeded.seeded()&&seeded.pixel_proof()==5,"DE AO contract extension changed");
        check(!legacy.independent_primary_state()&&
            legacy.primary().proof==SelectionProof::LegacyQualifiedPixels,"legacy pixel proof replaced");
    }
    {
        // ABoxMod1 writes Vary conditionally on first use. First=0 is not a
        // driver reset of Vary; without a path proof keep ordered DE AO.
        auto boxes=program(Hybrid3DAnalytic,false,{slot(ExternalFormula,8024,2,8,0,1),slot(ExternalFormula,8900,2,10,0,0)},{1,1});
        boxes.de_option=2;
        check(!IndependentPrimaryPixels(boxes)&&!IndependentDepthScenePixels(boxes),"mutable Vary whitelist fixture");
        mb3d_native_ao::DEParams ao{};ao.width=64;ao.height=48;
        const DistanceAOExecutionPolicy chain(boxes,ao);
        check(!chain.independent()&&!chain.conditional_state()&&!chain.pixel_proof(),"mutable Vary DE AO admitted");
        ao.first_step_random=1;check(!DistanceAOExecutionPolicy(boxes,ao).seeded()&&!DistanceAOExecutionPolicy(boxes,ao).independent(),
              "random DE AO mutable Vary admitted");
        auto mixed=boxes;mixed.slots[1].id=9526;
        ao.first_step_random=0;check(!IndependentPrimaryPixels(mixed)&&DistanceAOExecutionPolicy(mixed,ao).pixel_proof()!=6,
              "unaudited DE AO chain admitted");
    }
    const auto juliaCarry=AnalyzePrimaryPixelState(quat);
    check(!(juliaCarry.initialized&pixel_state::J4)&&(juliaCarry.live_inputs&pixel_state::J4),"Julia J4 preservation");
    auto q=quat;q.julia=0;
    check(AnalyzePrimaryPixelState(q).independent&&(AnalyzePrimaryPixelState(q).live_inputs&pixel_state::J4),
        "immutable Mandelbrot J4 input rejected");
    // A generated J4 writer prevents an immutable live-input proof.
    q.slots[1]=slot(InternalFormula,7,0,0,0,0);
    check(!AnalyzePrimaryPixelState(q).independent,"mutable J4 input admitted");
    // Persistent shadow tasks carry formula state between pixels: no speculation.
    {const GeneralExecutionPolicy dependent(q,request(true));
     check(dependent.shadow_persistent()&&!dependent.speculative_shadows(),"persistent shadows made speculative");}
    q=julia;q.slots[0].traits=0;check(!AnalyzePrimaryPixelState(q).known,"generated metadata mismatch");
    q=julia;++q.slots[0].param_count;check(!AnalyzePrimaryPixelState(q).known,"generated parameter mismatch");
    // The analysis reads difs_style at run time: its masks cover both styles.
    q=bulbox;q.slots[0].difs_style=1;check(AnalyzePrimaryPixelState(q).independent,"dIFS style mask coverage lost");
    q=bulbox;q.driver=HybridDifs;check(!AnalyzePrimaryPixelState(q).known,"generated driver domain widened");
    {
        // dIFS shrub shape (KnotsIFS-style dIFS body plus a difs_style dIFS
        // body): reads rstop_d/dfree1/iteration, all initialized per query.
        auto difs=program(HybridDifs,false,{slot(ExternalFormula,10199,20,14,0,1),slot(ExternalFormula,9090,20,18,0,1)},{1,1});
        difs.de_option=20;difs.slots[1].difs_style=1;
        const auto c=AnalyzePrimaryPixelState(difs);
        check(c.known&&c.generated&&c.independent&&c.conditional&&!c.production_chain,"conditional dIFS contract");
        check((c.initialized&pixel_state::DifsInitialized)==pixel_state::DifsInitialized,"dIFS entry initialization");
        const GeneralExecutionPolicy ordered(difs,request(true));
        check(ordered.primary_persistent()&&!ordered.conditional_state()&&!ordered.parallel_reflections(),
              "conditional contract admitted without the request");
        auto r=request(true);r.conditional_state=true;
        const GeneralExecutionPolicy pixels(difs,r);
        check(pixels.independent_primary_state()&&pixels.conditional_state()&&pixels.parallel_reflections()&&
              pixels.shadow().owner==LogicalOwner::WholeRow,"conditional contract not pixel-owned");
        mb3d_native_ao::DEParams ao{};ao.width=64;ao.height=48;
        // DE AO queries enter the same orbits: the conditional contract frees
        // AO pixels, and the ordered fallback restores the persistent tasks.
        const DistanceAOExecutionPolicy aoPixels(difs,ao),aoOrdered(difs,ao,nullptr,true);
        check(aoPixels.independent()&&aoPixels.conditional_state()&&aoPixels.pixel_proof()==7&&
              aoPixels.entry().proof==SelectionProof::IndependentPrimaryState&&aoPixels.entry().tasks==64*48,
              "DE AO conditional contract not pixel-owned");
        check(!aoOrdered.independent()&&!aoOrdered.conditional_state()&&!aoOrdered.pixel_proof()&&
              aoOrdered.entry().owner==LogicalOwner::ContiguousRows,"DE AO conditional fallback not ordered");
        {auto random=ao;random.first_step_random=1;
         const DistanceAOExecutionPolicy seeded(difs,random),ordered(difs,random,nullptr,true);
         check(seeded.seeded()&&seeded.conditional_state()&&seeded.pixel_proof()==7&&!ordered.independent()&&!ordered.seeded(),
               "DE AO random sequence not seeded conditional pixels");}
        auto noInit=difs;noInit.driver=HybridDifsNoInitialization;
        check(!AnalyzePrimaryPixelState(noInit).known,"dIFS without initialization admitted");
        // DE combination (Recycledrelatives shape): a dIFS part in either
        // position makes the contract conditional. The second part may read
        // what the first part's entry initialized within the same query.
        auto surf=slot(ExternalFormula,10231,20,21,0,1);surf.difs_style=1;
        auto combined=program(HybridDifs,false,{surf,slot(ExternalFormula,10173,2,17,0,1)},{1,2});
        combined.de_option=20;combined.formula_type=CombineMinimum;combined.end_slot=0;
        auto part=combined;part.driver=Hybrid3DAnalytic;part.de_option=2;
        part.start_slot=part.repeat_slot=part.end_slot=1;
        const auto mixed=AnalyzePrimaryPixelState(combined,&part);
        check(mixed.known&&mixed.generated&&mixed.independent&&mixed.conditional,"dIFS-first combination contract");
        check(mixed.driver_may_write&pixel_state::Dfree1,"dIFS driver dfree1 write unreported");
        check(GeneralExecutionPolicy(combined,r,&part).conditional_state()&&
              GeneralExecutionPolicy(combined,r,&part).parallel_reflections(),"dIFS combination not pixel-owned");
        check(DistanceAOExecutionPolicy(combined,ao,&part).conditional_state()&&
              DistanceAOExecutionPolicy(combined,ao,&part).pixel_proof()==7&&
              !DistanceAOExecutionPolicy(combined,ao,&part,true).independent(),"DE AO dIFS combination contract");
        // Swapped order (ThePearl shape): the dIFS body's dfree1 read is
        // covered because only the dIFS part, whose entry clears it, runs it.
        auto analyticFirst=program(Hybrid3DAnalytic,false,{slot(ExternalFormula,10173,2,17,0,1),surf},{2,1});
        analyticFirst.de_option=2;analyticFirst.formula_type=CombineMinimum;analyticFirst.end_slot=0;
        auto difsSecond=analyticFirst;difsSecond.driver=HybridDifs;difsSecond.de_option=20;
        difsSecond.start_slot=difsSecond.repeat_slot=difsSecond.end_slot=1;
        const auto swapped=AnalyzePrimaryPixelState(analyticFirst,&difsSecond);
        check(swapped.known&&!swapped.independent&&swapped.conditional&&
              (swapped.driver_may_write&pixel_state::Vary)&&!(swapped.live_inputs&pixel_state::Vary),
              "dIFS-second carried Vary admitted");
        // A first part whose range reaches that slot would read the previous
        // query's dfree1: a shared slot keeps the first part's initialization.
        auto reaching=analyticFirst;reaching.end_slot=1;
        check(reaching.slots[1].id==10231&&!AnalyzePrimaryPixelState(reaching,&difsSecond).independent,
            "carried dfree1 read admitted");
        // Dodeca shape: dIFS without initialization continues the first
        // part's orbit, so it is admitted only in second position.
        auto dodeca=program(Hybrid3DAnalytic,false,{slot(ExternalFormula,10041,2,25,0,1),
            slot(ExternalFormula,4952,20,10,0,1)},{1,1});
        dodeca.de_option=2;dodeca.formula_type=CombineMix;dodeca.end_slot=0;dodeca.slots[1].difs_style=1;
        auto continued=dodeca;continued.driver=HybridDifsNoInitialization;continued.de_option=20;
        continued.start_slot=continued.repeat_slot=continued.end_slot=1;
        const auto chained=AnalyzePrimaryPixelState(dodeca,&continued);
        check(chained.known&&!chained.independent&&chained.conditional,"dIFS continuation carried Vary admitted");
        auto reversed=continued;reversed.start_slot=reversed.repeat_slot=reversed.end_slot=0;
        auto analyticSecond=dodeca;analyticSecond.start_slot=analyticSecond.repeat_slot=analyticSecond.end_slot=1;
        check(!AnalyzePrimaryPixelState(reversed,&analyticSecond).known,"uninitialized dIFS first part admitted");
    }
    {
        // Aexion 10bulbs shape: Hybrid4D resets xyz/w and j.xyzw, never
        // vary_scale. No body writes it, so it stays the initial live input.
        const I32 skip=std::bit_cast<I32>(0x80000001u);
        auto four=program(Hybrid4D,false,{slot(ExternalFormula,7065,-1,7,0,1),slot(ExternalFormula,7055,-1,2,0,1),
            slot(ExternalFormula,10323,4,1,192,1)},{skip,skip,4});
        four.de_option=4;four.repeat_slot=2;
        const auto c=AnalyzePrimaryPixelState(four);
        check(c.known&&c.generated&&c.independent&&!c.conditional,"Hybrid4D contract");
        check((c.live_inputs&pixel_state::Vary)&&!(c.initialized&pixel_state::Vary)&&
              (c.initialized&pixel_state::bit(11))&&!(c.driver_may_write&pixel_state::Vary),"Hybrid4D vary_scale input");
        check(GeneralExecutionPolicy(four,request(true)).independent_primary_state(),"Hybrid4D not pixel-owned");
        // A body writing vary_scale writes that live input; its step guard
        // would read the previous query's value.
        auto written=four;written.slots[0]=slot(ExternalFormula,10303,20,13,0,1);
        const auto carried=AnalyzePrimaryPixelState(written);
        check(carried.known&&(carried.formula_may_write&pixel_state::Vary)&&!carried.independent,
              "carried vary_scale admitted");
        auto analytic=four;analytic.driver=Hybrid4DAnalytic;
        check(!AnalyzePrimaryPixelState(analytic).known,"Hybrid4D analytic admitted");
        auto combined=four;combined.formula_type=CombineMinimum;auto part=combined;
        check(!AnalyzePrimaryPixelState(combined,&part).known,"Hybrid4D combination admitted");
    }
    {
        // ThePearl-style Hybrid3D chain: 8896 reads the carried Deriv1 and only
        // rescales it in place. No 3D driver reads Deriv1, so the carry never
        // reaches the pixel and the contract stays independent.
        auto rescaled=program(Hybrid3D,true,{slot(InternalFormula,0,0,0,0,0),slot(ExternalFormula,7010,-1,5,512,1),
            slot(InternalFormula,3,0,0,0,0),slot(ExternalFormula,8896,-1,14,8770,1)},{1,1,1,1});
        const auto c=AnalyzePrimaryPixelState(rescaled);
        check(c.known&&c.independent&&c.contained==pixel_state::bit(16)&&
              !(c.initialized&pixel_state::bit(16)),"contained Deriv1");
        check(GeneralExecutionPolicy(rescaled,request(true)).independent_primary_state(),"contained Deriv1 not pixel-owned");
        // Hybrid3DAnalytic DE 2 reads w, not Deriv1 (10343 rescales it).
        auto analytic=program(Hybrid3DAnalytic,true,{slot(InternalFormula,4,11,0,0,2),
            slot(ExternalFormula,10343,6,21,2,1),slot(ExternalFormula,9526,2,22,0,1),slot(ExternalFormula,10181,11,10,0,1)},
            {1,1,1,1});
        analytic.de_option=2;analytic.repeat_slot=2;
        const auto varying=AnalyzePrimaryPixelState(analytic);
        check(varying.known&&!varying.independent&&(varying.formula_may_write&pixel_state::Vary),
              "conditional variable-scale body erased carry");
        analytic.slots[3]=slot(ExternalFormula,9452,2,1,0,1);
        const auto a=AnalyzePrimaryPixelState(analytic);
        check(a.known&&a.independent&&a.contained==pixel_state::bit(16),"contained analytic Deriv1");
        check((a.driver_may_write&(15ull<<20))==(15ull<<20),"analytic DE2 caller SSE writes omitted");
        check(!(a.initialized&(15ull<<20)),"conditional analytic SSE writes became unconditional initialization");
        auto otherDe=analytic;otherDe.de_option=11;
        check((AnalyzePrimaryPixelState(otherDe).driver_may_write&pixel_state::SSE)==pixel_state::SSE,
              "DE11 caller return SSE writes omitted");
        // Another body consuming Deriv1 makes the carry reach the pixel.
        auto consumed=rescaled;consumed.slots[1]=slot(ExternalFormula,10353,-1,6,0,1);
        const auto u=AnalyzePrimaryPixelState(consumed);
        check(u.known&&!(u.contained&pixel_state::bit(16))&&!u.independent,"consumed Deriv1 contained");
        // A 4D driver reads Deriv1 for its analytic DE.
        auto four=rescaled;four.driver=Hybrid4D;four.de_option=4;four.slots[1]=slot(ExternalFormula,10343,6,21,2,1);
        check(!AnalyzePrimaryPixelState(four).contained,"driver-read Deriv1 contained");
        for(const auto& [driver,de]:{std::pair{Hybrid3D,0},{Hybrid3DAnalytic,2},{HybridDifs,20}}){
            auto q=rescaled;q.driver=driver;q.de_option=de;
            check(pixel_state::driver_unread(q)==pixel_state::Derivative,"3D driver Deriv1 read");
            q.de_option=(de&~0x38)|32;check(!pixel_state::driver_unread(q),"XYZ-derivative driver Deriv1 unread");
        }
    }
    // DE combination (DEcomb1 shape): both parts use admitted drivers over the
    // shared slots; the policy needs the second part, never a closed proof.
    auto first=program(Hybrid3D,false,{slot(InternalFormula,5,0,0,0,0),slot(InternalFormula,0,0,0,0,0)},{1,1});
    first.end_slot=0;first.formula_type=CombineMinimum;first.combination_smooth=1.f;
    auto second=first;second.driver=Hybrid3DAnalytic;second.de_option=2;
    second.start_slot=second.repeat_slot=second.end_slot=1;
    {
        const auto c=AnalyzePrimaryPixelState(first,&second);
        check(c.known&&c.independent&&!c.production_chain,"admitted DE combination contract");
        check(GeneralExecutionPolicy(first,request(true),&second).independent_primary_state(),
            "admitted DE combination not pixel-owned");
        check(!AnalyzePrimaryPixelState(first).known,"DE combination without its second part");
        check(!AnalyzePrimaryPixelState(bulbox,&second).known,"second part without a combination");
        auto difs=second;difs.driver=HybridDifs;difs.de_option=20;
        const auto conditional=AnalyzePrimaryPixelState(first,&difs);
        check(!conditional.independent&&conditional.conditional&&
              (conditional.driver_may_write&pixel_state::Vary)&&!(conditional.live_inputs&pixel_state::Vary),
              "later dIFS reset falsely made Vary immutable");
        check(!GeneralExecutionPolicy(first,request(true),&difs).independent_primary_state(),
            "dIFS second part admitted without the conditional request");
        auto other=second;other.formula_type=CombineMaximum;
        check(!AnalyzePrimaryPixelState(first,&other).known,"mismatched combination admitted");
        const GeneralExecutionPolicy bound(first,request(true),&second);
        bool rejected=false;
        try{bound.validate(first,request(true),&difs);}catch(const std::invalid_argument&){rejected=true;}
        check(rejected,"policy accepted a different second part");
    }
    // Conditional counters are imprecise over every lane and stay dependent.
    q=program(Hybrid3D,false,{slot(InternalFormula,5,0,0,0,0),slot(InternalFormula,7,0,0,0,0)},{1,1});
    check(AnalyzePrimaryPixelState(q).known&&!AnalyzePrimaryPixelState(q).independent,"imprecise internal admitted");
    check(GeneralExecutionPolicy(q,request(true)).primary_persistent(),"dependent contract admitted");
    // Every hand-audited body covers its compiler-derived reads and possible writes.
    for(const auto& s:{slot(InternalFormula,4,11,0,0,0),slot(InternalFormula,4,11,0,0,2),
        slot(ExternalFormula,9452,2,1,0,1),slot(ExternalFormula,7046,0,5,0,0),slot(ExternalFormula,3676,-1,7,0,1),
        slot(ExternalFormula,10010,-1,15,0,1),slot(ExternalFormula,7076,-1,7,0,1),
        slot(ExternalFormula,10171,2,22,0,1),slot(ExternalFormula,1509,11,15,0,1)}){
        const auto hand=pixel_state::formula(s),compiler=pixel_state::generated_formula(s);
        check(hand.known&&compiler.known&&!(compiler.reads_before_write&~hand.reads_before_write)&&
            !(compiler.may_write&~hand.may_write),"hand audit misses a compiler-derived lane");
    }
    for(const auto& e:generated_effects::kEntries)
        check(e.namespace_id<=ExternalFormula&&(e.reads|e.writes)<(StateMask(1)<<47),"generated mask layout");
}
static void state_contract_fixtures(){
    // Metadata from independently exported loaded programs, without scene
    // assets. These exercise rejection on ordinary fresh-checkout CI too.
    using namespace mb3d_native;
    for(unsigned chain=0;chain<2;++chain){
        NativeFormulaProgram p{};p.version=1;p.slot_count=6;
        p.driver=chain?Hybrid3DAnalytic:Hybrid3D;p.de_option=chain?2:0;
        p.repeat_slot=chain?1:0;p.end_slot=chain?5:2;
        const U32 ids[2][6]={{4,9452,7046,0,0,0},{3676,4,10010,7076,10171,1509}};
        const U32 counts[2][6]={{12,1,2,0,0,0},{0x80000001u,1,0x80000001u,0x80000001u,8,102}};
        const I32 options[2][6]={{11,2,0,0,0,0},{-1,11,-1,-1,2,11}};
        const U32 params[2][6]={{0,1,5,0,0,0},{7,0,15,7,22,15}};
        const U32 flags[2][6]={{0,1,0,0,0,0},{1,2,1,1,1,1}};
        for(unsigned i=0;i<6;++i){auto& s=p.slots[i];s.version=1;
            s.enabled=ids[chain][i]!=0;s.id=ids[chain][i];
            s.namespace_id=s.enabled&&s.id!=4?ExternalFormula:InternalFormula;
            s.de_option=options[chain][i];s.param_count=params[chain][i];s.wrapper_flags=flags[chain][i];
            p.hybrid_counts[i]=std::bit_cast<I32>(counts[chain][i]);}
        pixel_state_contract(p);
        const auto shadow=AnalyzeSoftShadowPixelState(p,0,0);
        check(!shadow.production_chain,"soft-shadow stale Vary proof retained");
        if(chain){
            check(shadow.entry==2&&shadow.known&&!shadow.independent,"soft-shadow carried Vary admitted");
            check((shadow.formula_reads_before_write&~(shadow.initialized|shadow.live_inputs))&pixel_state::Vary,
                  "soft-shadow late Vary writer lost dependency");
            check(!AnalyzePrimaryPixelState(p).independent&&!AnalyzePrimaryPixelState(p).production_chain,
                  "Spirals late Vary writer retained closed proof");
            check(!AnalyzeSoftShadowPixelState(p,1,0).production_chain&&
                  !AnalyzeSoftShadowPixelState(p,0,1).production_chain,"unqualified inside shadow policy");
            auto reject=[](const auto& q){check(!SoftShadowPixelChain(q)&&
                !AnalyzeSoftShadowPixelState(q,0,0).production_chain,"shadow descriptor mutation admitted");};
            for(unsigned i=0;i<6;++i){
                auto q=p;q.slots[i].namespace_id^=1;reject(q);
                q=p;++q.slots[i].id;reject(q);
                q=p;q.hybrid_counts[i]^=int(0x80000000u);reject(q);
                q=p;++q.hybrid_counts[i];reject(q);
                q=p;++q.slots[i].de_option;reject(q);
                q=p;++q.slots[i].param_count;reject(q);
                q=p;q.slots[i].traits=1;reject(q);
                q=p;q.slots[i].wrapper_flags^=1;reject(q);
                q=p;q.slots[i].difs_style=1;reject(q);
                q=p;q.slots[i].enabled=0;reject(q);
                q=p;q.slots[i].version=0;reject(q);
                q=p;q.slots[i].reserved[1]=1;reject(q);
            }
            auto q=p;q.inside=1;reject(q);
            q=p;q.version=0;reject(q);q=p;q.driver=Hybrid3D;reject(q);
            q=p;q.de_option=11;reject(q);q=p;q.slot_count=5;reject(q);
            q=p;q.start_slot=1;reject(q);q=p;q.repeat_slot=0;reject(q);
            q=p;q.end_slot=4;reject(q);q=p;q.mix_power=1;reject(q);
            q=p;q.combination_smooth=1;reject(q);q=p;q.formula_type=1;reject(q);
            q=p;q.mix_color=1;reject(q);q=p;std::swap(q.slots[0],q.slots[2]);reject(q);
        }
    }
    check(stateContracts==1,"stateless Loopy proof or mutable Spirals fallback changed");stateContracts=0;
    generated_contract_fixtures();
}
template<class T> static void same(const std::vector<T>& a,const std::vector<T>& b,const char* message) {
    check(a.size()==b.size(),message);
    if(!a.empty())check(!std::memcmp(a.data(),b.data(),a.size()*sizeof(T)),message);
}
static void same(const ExactColorScene& a,const ExactColorScene& b,const char* message) {
    // Eight ABI alignment bytes after the last field are not material data.
    constexpr size_t fields=offsetof(ExactColorScene,diffuse_table)+sizeof(a.diffuse_table);
    static_assert(fields==4760&&sizeof(ExactColorScene)==4768);
    check(!std::memcmp(&a,&b,fields),message);
}
static void descriptors(const PreparedRenderPlan& a,const PreparedRenderPlan& b) {
    same(a.scene.base.device,b.scene.base.device,"scene");
    same(a.scene.formulas.device,b.scene.formulas.device,"formulas");
    same(a.scene.formulas.maps,b.scene.formulas.maps,"maps");
    same(a.scene.formulas.map_pixels,b.scene.formulas.map_pixels,"map pixels");
    same(a.scene.camera.device,b.scene.camera.device,"camera");
    same(a.scene.camera.columns,b.scene.camera.columns,"camera columns");
    same(a.scene.camera.rows,b.scene.camera.rows,"camera rows");
    same(a.packing,b.packing,"packing");same(a.color.base.device,b.color.base.device,"color");
    same(a.color.interior,b.color.interior,"interior");same(a.normals,b.normals,"normals");
    same(a.color.maps,b.color.maps,"material maps");same(a.color.map_samples,b.color.map_samples,"material map samples");
    same(a.inside,b.inside,"inside");same(a.inside_reflection,b.inside_reflection,"inside reflection");
    same(a.shadow,b.shadow,"shadow");same(a.soft_shadow,b.soft_shadow,"soft shadow");
    same(a.reflection,b.reflection,"reflection");same(a.dof,b.dof,"dof");same(a.z_normals,b.z_normals,"z normals");
    same(a.distance_ao,b.distance_ao,"distance ao");same(a.screen_ao.params,b.screen_ao.params,"screen ao");
    same(a.screen_ao.levels,b.screen_ao.levels,"ao levels");
}
static void resource_intent(const TMandHeader10& original,const THeaderCustomAddon& addon,
                            const RenderPlanRequest& request) {
    // Production loader historically deactivates missing maps. The Native
    // capability decision retains the original request across that mutation:
    // camera color and the reflection material sample diffuse, light and
    // background maps whether or not they load (a missing one keeps the
    // loader's inactive state).
    for(int kind=0;kind<8;++kind)for(int reflections=0;reflections<2;++reflections) {
        auto h=original;
        if(reflections)h.bCalcSRautomatic|=1;else h.bCalcSRautomatic&=uint8_t(~1u);
        if(kind==0){h.Light.bColorMap=254;h.Light.Lights[1].AdditionalByteEx=255;}
        else if(kind==1)std::memcpy(h.Light.BGbmp,"missing-resource.bmp",21);
        else {h.Light.Lights[kind-2].Loption=2;h.Light.Lights[kind-2].LightMapNr=65534;}
        const auto before=h;
        const auto required=NativeLightingRequirements(h);
        check(required.size()==1,"requested resource count");
        check(required[0].light_index==(kind<2?-1:kind-2),"original light index lost");
        auto p=PrepareNativeRenderPlan(h,addon,request,PlanDiagnostics::CollectIndependentStages);
        const auto color=std::find_if(p.stages.begin(),p.stages.end(),[](const auto& stage){return stage.name=="color";});
        check(color!=p.stages.end(),"color stage missing");
        check(p.lighting_requirements.size()==1,"plan resource intent lost");
        same(h,before,"resource probe input changed");
        check(CurrentPrecisionMode()==PrecisionMode::Compatible,"resource rejection precision leaked");
        check(color->status=="ACCEPTED","camera color rejected a requested material map");
        check(!(p.color.interior.flags&mb3d_native::kNativeMaterialMapFlag)&&p.color.map_samples.empty()&&
            !p.color.maps.diffuse_mode&&!p.color.maps.light_mask&&!p.color.maps.background,"missing material map bound");
        check(p.color.reflection.version==(reflections?mb3d_native::kNativeReflectionMaterialVersion:0u)&&
            !p.color.reflection.maps.diffuse_mode&&!p.color.reflection.maps.light_mask&&!p.color.reflection.maps.background,
            "missing reflection map bound");
        const auto direct=PrepareGeneralColor(p.scene.base,p.shadows);
        same(direct.interior,p.color.interior,"direct material map preparation");
    }
    // A missing background with bUseSmallBGpicForAmb keeps its rotated-normal
    // ambient: the paint geometry is bound, no map or samples.
    for(int reflections=0;reflections<2;++reflections) {
        auto h=original;std::memcpy(h.Light.BGbmp,"missing-resource.bmp",21);h.Light.AdditionalOptions|=32;
        if(reflections)h.bCalcSRautomatic|=1;else h.bCalcSRautomatic&=uint8_t(~1u);
        auto p=PrepareNativeRenderPlan(h,addon,request,PlanDiagnostics::CollectIndependentStages);
        const auto color=std::find_if(p.stages.begin(),p.stages.end(),[](const auto& stage){return stage.name=="color";});
        check(color!=p.stages.end()&&color->status=="ACCEPTED","rotated background ambient rejected");
        const auto& maps=p.color.maps;
        check((p.color.interior.flags&mb3d_native::kNativeMaterialMapFlag)&&(p.color.interior.flags&mb3d_native::kNativeLightGeometryFlag)&&
            maps.background==mb3d_native::kNativeBackgroundRotatedAmbient&&!maps.sample_count&&p.color.map_samples.empty()&&
            !maps.background_map.width,"rotated background ambient descriptor");
        if(reflections)same(p.color.reflection.maps,maps,"reflection rotated background ambient");
    }
    // A loaded map is bound: the diffuse mode, raw seam-layout samples, and a
    // light-map light at its sorted shading index, which stays disabled.
    const auto root=std::filesystem::temp_directory_path()/("mb3d-material-maps-"+std::to_string(getpid()));
    std::filesystem::create_directories(root/"M3Maps");
    {
        std::ofstream map(root/"M3Maps"/"7.pgm");map<<"P2\n8 6\n255\n";
        for(int i=0;i<48;++i)map<<(i*37)%256<<(i%8==7?'\n':' ');
    }
    // The background picture resolves against the resource root itself.
    std::filesystem::copy_file(root/"M3Maps"/"7.pgm",root/"7.pgm");
    auto mapped=request;mapped.resource_root=root.string();
    for(int kind:{0,7}) {
        auto h=original;h.bCalcSRautomatic&=uint8_t(~1u);
        if(kind==0){h.Light.bColorMap=7;h.Light.Lights[1].AdditionalByteEx=0;h.Light.Lights[1].FreeByte&=uint8_t(~3u);}
        else {h.Light.Lights[5].Loption=2;h.Light.Lights[5].LightMapNr=7;}
        auto p=PrepareNativeRenderPlan(h,addon,mapped,PlanDiagnostics::CollectIndependentStages);
        const auto color=std::find_if(p.stages.begin(),p.stages.end(),[](const auto& stage){return stage.name=="color";});
        check(color!=p.stages.end()&&color->status=="ACCEPTED","loaded material map rejected");
        const auto& maps=p.color.maps;const std::size_t samples=9*7*3;
        check((p.color.interior.flags&mb3d_native::kNativeMaterialMapFlag)&&maps.version==mb3d_native::kNativeMaterialMapVersion&&
            maps.sample_count==samples&&p.color.map_samples.size()==samples,"loaded material map samples");
        if(kind==0) {
            check((maps.diffuse_mode&~1u)==2&&((maps.diffuse_mode&1u)!=0)==((p.color.interior.flags&8u)!=0)&&
                maps.diffuse.width==8&&maps.diffuse.height==6&&!maps.diffuse.offset&&!maps.light_mask,"diffuse map descriptor");
        } else {
            check(!maps.diffuse_mode&&std::popcount(maps.light_mask)==1,"light map mask");
            const unsigned i=unsigned(std::countr_zero(maps.light_mask));
            check(maps.lights[i].width==8&&maps.lights[i].height==6&&!p.color.interior.lights[i].enabled,"light map descriptor");
        }
        auto again=PrepareNativeRenderPlan(h,addon,mapped,PlanDiagnostics::CollectIndependentStages);
        same(again.color.maps,maps,"material map descriptor repeat");
        same(again.color.map_samples,p.color.map_samples,"material map samples repeat");
        // Reflections shade the same bound maps through their material descriptor.
        h.bCalcSRautomatic|=1;
        auto reflected=PrepareNativeRenderPlan(h,addon,mapped,PlanDiagnostics::CollectIndependentStages);
        check(std::any_of(reflected.stages.begin(),reflected.stages.end(),[](const auto& stage){
            return stage.name=="color"&&stage.status=="ACCEPTED";}),"loaded material map rejected with reflections");
        check(reflected.color.reflection.version==mb3d_native::kNativeReflectionMaterialVersion,"reflection material missing");
        same(reflected.color.reflection.maps,maps,"reflection material maps");
        same(reflected.color.map_samples,p.color.map_samples,"reflection material samples");
    }
    // A loaded background map is bound with the light geometry it is sampled
    // through (AbsViewVec, the paint matrix), for camera color and reflections.
    for(unsigned options:{0u,8u,40u})for(int reflections=0;reflections<2;++reflections) {
        auto h=original;std::memset(h.Light.BGbmp,0,sizeof(h.Light.BGbmp));std::memcpy(h.Light.BGbmp,"7.pgm",5);
        h.Light.AdditionalOptions=uint8_t((h.Light.AdditionalOptions&~40u)|options);h.Light.TBoptions&=~0x8000u;
        if(reflections)h.bCalcSRautomatic|=1;else h.bCalcSRautomatic&=uint8_t(~1u);
        auto p=PrepareNativeRenderPlan(h,addon,mapped,PlanDiagnostics::CollectIndependentStages);
        const auto color=std::find_if(p.stages.begin(),p.stages.end(),[](const auto& stage){return stage.name=="color";});
        check(color!=p.stages.end()&&color->status=="ACCEPTED","loaded background map rejected");
        const auto& maps=p.color.maps;const auto& m=maps.background_map;
        const uint32_t expected=mb3d_native::kNativeBackgroundMap|((options&8u)?mb3d_native::kNativeBackgroundAddLight:0u)|
            ((options&32u)?mb3d_native::kNativeBackgroundAmbient:0u);
        check((p.color.interior.flags&mb3d_native::kNativeMaterialMapFlag)&&(p.color.interior.flags&mb3d_native::kNativeLightGeometryFlag)&&
            maps.background==expected&&m.width&&m.height&&
            p.color.map_samples.size()==std::size_t(m.width+1)*(m.height+1)*3&&maps.sample_count==p.color.map_samples.size(),
            "background map descriptor");
        if(reflections)same(p.color.reflection.maps,maps,"reflection background map");
        auto again=PrepareNativeRenderPlan(h,addon,mapped,PlanDiagnostics::CollectIndependentStages);
        same(again.color.maps,maps,"background map descriptor repeat");
        same(again.color.map_samples,p.color.map_samples,"background map samples repeat");
    }
    {
        // Image coordinates (bDirectImageCoord) stay closed.
        auto h=original;std::memset(h.Light.BGbmp,0,sizeof(h.Light.BGbmp));std::memcpy(h.Light.BGbmp,"7.pgm",5);
        h.Light.TBoptions|=0x8000u;
        auto p=PrepareNativeRenderPlan(h,addon,mapped,PlanDiagnostics::CollectIndependentStages);
        const auto color=std::find_if(p.stages.begin(),p.stages.end(),[](const auto& stage){return stage.name=="color";});
        check(color!=p.stages.end()&&color->status=="REJECTED"&&color->reason.find("image coordinates")!=std::string::npos,
            "background image coordinates admitted");
    }
    std::filesystem::remove_all(root);
    // Semantic off states stay off despite stale IDs. Light indices are raw,
    // never the sorted shading indices; higher Loption bits do not select maps.
    for(unsigned option=0;option<256;++option)for(int index=0;index<6;++index) {
        auto h=original;h.Light.Lights[index].Loption=uint8_t(option);
        h.Light.Lights[index].LightMapNr=0;
        check(NativeLightingRequirements(h).empty(),"map zero not off");
        h.Light.Lights[index].LightMapNr=1633;
        const auto requirements=NativeLightingRequirements(h);
        check(requirements.size()==((option%4)==2?1u:0u),"dormant light map request");
        if(!requirements.empty())check(requirements[0].light_index==index&&requirements[0].map_id==1633,
            "light map identity changed");
    }
    for(unsigned id:{0u,55u,256u,1633u,65535u}) {
        auto h=original;h.Light.bColorMap=uint8_t(id);h.Light.Lights[1].AdditionalByteEx=uint8_t(id>>8);
        const auto requirements=NativeLightingRequirements(h);
        check(requirements.size()==(id?1u:0u),"diffuse request lost high byte");
        if(id)check(requirements[0].map_id==id,"diffuse map identity");
    }
    auto h=original;std::memset(h.Light.BGbmp,'x',sizeof(h.Light.BGbmp));
    check(NativeLightingRequirements(h).at(0).filename==std::string(24,'x'),"bounded background name");
    h.Light.BGbmp[0]=0;
    check(NativeLightingRequirements(h).empty(),"dormant background bytes became request");
}
static void orbit_color_contract(const PreparedGeneralScene& scene) {
    auto& m=scene.base.host->mct;const auto saved=m;
    m.ColorOnIt=0;m.DFogOnIt=0;m.ColorOption=0;
    m.mctColorMul=std::numeric_limits<float>::quiet_NaN();
    auto p=PrepareGeneralSurface(scene);
    check(p.version==3&&!p.color_option&&p.orbit_color_multiplier==0&&!p.reserved,"mode0 canonical unused multiplier");
    m.ColorOption=1;m.mctColorMul=-1024.f;
    p=PrepareGeneralSurface(scene);
    check(p.color_option==1&&p.orbit_color_multiplier==-1024.f,"orbit multiplier distinct from SIgradient");
    // Scene preparation owns ColorOnIt (color_on_it_contract); the surface accepts it.
    m.ColorOnIt=52;p=PrepareGeneralSurface(scene);
    check(p.color_option==1&&p.orbit_color_multiplier==-1024.f,"ColorOnIt changed the surface descriptor");
    for(unsigned bad=1;bad<5;++bad){
        m.ColorOnIt=0;m.DFogOnIt=0;m.ColorOption=1;m.mctColorMul=512.f;m.DEoption=saved.DEoption;
        if(bad==1)m.ColorOption=2;if(bad==2)m.DFogOnIt=65535;
        if(bad==3)m.DEoption=20;if(bad==4)m.mctColorMul=std::numeric_limits<float>::infinity();
        bool threw=false;try{(void)PrepareGeneralSurface(scene);}catch(const std::invalid_argument&){threw=true;}
        check(threw,"unsupported orbit color contract rejected");
    }
    m=saved;
}
// RMdoColorOnIt 1 has no ColorOption 0/1 effect; N>1 carries the iteration
// maximum N-1 and Single(RStop*RStop*64). A maximum beyond maxIt+1 stays closed.
static void color_on_it_contract(const TMandHeader10& original,const THeaderCustomAddon& addon,
    const RenderPlanRequest& request) {
    for(unsigned value:{0u,1u,2u,5u}){
        auto h=original;h.bColorOnIt=uint8_t(value);
        const auto plan=PrepareNativeRenderPlan(h,addon,request);check(plan.ready(),"ColorOnIt plan rejected");
        const auto& d=plan.scene.base.device;const auto& m=plan.scene.base.host->mct;
        if(value<2)check(!d.color_on_it&&!d.color_radius_stop,"inactive ColorOnIt carried");
        else check(d.color_on_it==value-1&&
            std::bit_cast<float>(d.color_radius_stop)==static_cast<float>(m.dRStop*m.dRStop*64.0f),
            "ColorOnIt descriptor");
    }
    if(original.Iterations+3<=255){
        auto h=original;h.bColorOnIt=uint8_t(original.Iterations+3);
        const auto plan=PrepareNativeRenderPlan(h,addon,request);
        check(!plan.ready()&&plan.firstFailure().find("ColorOnIt exceeds")!=std::string::npos,
            "ColorOnIt beyond the maximum admitted");
    }
}
// RMcalcVLight packs the volumetric step count for ConvertVLight; the Native
// inside policy packs it with the same codec.
static void volume_code_contract() {
    for(int i=-8;i<=70000;++i){const float c=float(i)*.25f;
        check(uint32_t(RMcalcVLight(c))==mb3d_inside::volume_light_code(c),"RMcalcVLight codec");}
    for(float c:{.49999997f,.5f,1.5f,2.5f,127.5f,16382.5f,16383.4f,1.e9f,std::numeric_limits<float>::max()})
        check(uint32_t(RMcalcVLight(c))==mb3d_inside::volume_light_code(c),"RMcalcVLight rounding and cap");
    mb3d_inside::Params p{mb3d_inside::Version,0,0,64,1,{0,0,0}};
    const auto packed=mb3d_inside::pack(p,0,0,0,1000.f,1.f,0.f,0.f,0,0);
    check(!packed.fault&&packed.shadow==(uint32_t(RMcalcVLight(1000.f))&1023u),"volumetric inside packing");
    p.volume_light=2;check(!mb3d_inside::valid(p),"volumetric inside flag range");
}
// bVolLightNr: DFogOnIt 65535 carries CalcVolLightMap's geometry (a positional
// cube, a directional face, or no map for a disabled light), the RMcalcVLight
// inside policy and the ConvertVLight color flag.
static void volume_light_contract(const TMandHeader10& original,const THeaderCustomAddon& addon,
    const RenderPlanRequest& request) {
    for(unsigned mode=0;mode<3;++mode){
        auto h=original;h.bVolLightNr=0x21;
        auto& light=h.Light.Lights[0];
        light.Loption=uint8_t((light.Loption&~7u)|(mode==0?4u:mode==2?1u:0u));
        const auto plan=PrepareNativeRenderPlan(h,addon,request);check(plan.ready(),"volumetric plan rejected");
        const auto& v=plan.scene.volume;const auto& m=plan.scene.base.host->mct;
        check(plan.scene.base.device.fog_iteration==65535&&plan.inside.volume_light==1&&
            (plan.color.interior.flags&mb3d_native::kNativeVolumeLightFlag),"volumetric policy");
        check(v.version==mb3d_native::kNativeVolumeLightVersion,"volumetric descriptor version");
        if(mode==2){check(!v.side_count&&!v.cube_size,"disabled volumetric light carried a map");continue;}
        ScopedRenderContext binding(plan.scene.base.host->context);
        auto header=plan.scene.base.host->file.header;
        TLightVals lights{};TLValigned aligned{};lights.PLValigned=&aligned;
        MakeLightValsFromHeaderLight(&header,&lights,1.0,0);CompleteLightValsSetup(&header,&lights);
        TVolumeLightMap map;TVolumeLightMapSetup setup;
        check(PrepareVolLightMap(&header,&lights,&m,map,setup),"CPU volumetric preparation");
        check(v.positional==unsigned(mode==0)&&v.side_count==unsigned(map.SideCount)&&
            v.cube_size==unsigned(map.CubeSize)&&v.half_size==map.HalfSize&&v.size_factor==map.SizeFactor&&
            v.stretch==map.StretchSide1&&v.min_distance==map.MinDistance&&v.max_distance==setup.maxDist&&
            v.cell_step==setup.sgl&&v.vl_mul==m.VLmul&&v.vl_step_mul==m.VLstepmul,"volumetric geometry");
        for(unsigned i=0;i<3;++i){
            check(v.light[i]==float(map.LightPos[i])&&v.forward[i]==float(setup.params.Vgrads[2][i])&&
                v.origin[i]==float(setup.params.Ystart[i]),"volumetric frame");
            for(unsigned j=0;j<3;++j)check(v.rotation[i][j]==map.RotMatrix[i][j]&&v.inverse[i][j]==setup.invMs[i][j],
                "volumetric rotation");
        }
    }
}
int RunNativePixelStateContractChecks();
int main(int argc,char** argv){try{
    check(RunNativePixelStateContractChecks()==0,"caller ownership regression");
    state_contract_fixtures();
    volume_code_contract();
    if(argc<3)throw std::invalid_argument("usage: test_native_render_plan RESOURCE_ROOT [--original-size] SCENE...");
    const bool originalSize=std::string(argv[2])=="--original-size";
    if(originalSize&&argc<4)throw std::invalid_argument("missing original scene");
    const ScopedNumericPrecision compatible(PrecisionMode::Compatible);
    unsigned accepted=0,rejected=0,resourceRejected=0;bool resourcesChecked=false,orbitChecked=false;
    for(int i=originalSize?3:2;i<argc;++i){
        auto file=LoadM3PFile(argv[i]);check(file.success,"scene load");
        const auto header=file.header;const auto addon=file.addon;
        RenderPlanRequest r;r.width=originalSize?header.Width:64;r.height=originalSize?header.Height:48;
        r.logical_workers=6;r.resource_root=argv[1];
        r.deterministic=true;r.force_calc3d=true;
        auto a=PrepareNativeRenderPlan(header,addon,r);
        check(CurrentPrecisionMode()==PrecisionMode::Compatible,"caller precision after prepare");
        auto b=PrepareNativeRenderPlan(header,addon,r,PlanDiagnostics::CollectIndependentStages);
        check(a.ready()==b.ready()&&a.firstFailure()==b.firstFailure(),"first failure differs");
        for(size_t n=0;n<a.stages.size();++n)check(a.stages[n].name==b.stages[n].name&&
            a.stages[n].status==b.stages[n].status&&a.stages[n].reason==b.stages[n].reason,"stage prefix differs");
        same(file.header,header,"caller header mutated");same(file.addon,addon,"caller addon mutated");
        same(a.requested_header,header,"lost original header");same(a.requested_addon,addon,"lost original addon");
        if(!a.ready()){
            ++rejected;bool threw=false;
            try{(void)PlanNativeRenderResources(a,{uint64_t(1)<<32,uint64_t(8)<<30});}
            catch(const std::invalid_argument&){threw=true;}
            check(threw,"rejected plan allocated");continue;
        }
        ++accepted;descriptors(a,b);
        if(originalSize){
            // Preparation admission is separate from the bounded workspace.
            // The independent old/new planner test compares these rejection
            // identities too; an oversized original is not a rendered PASS.
            try{(void)PlanNativeRenderResources(a,{uint64_t(1)<<32,uint64_t(8)<<30});}
            catch(const std::invalid_argument& e){
                const std::string reason=e.what();
                if(reason!="Native general buffers exceed device budget"&&
                   reason!="Native general and DOF buffers exceed device budget"&&
                   reason!="Native general and Z normal buffers exceed device budget"&&
                   reason!="Native general and AO buffers exceed device budget")throw;
                ++resourceRejected;continue;
            }
        }
        execution_contract(a);
        if(!orbitChecked&&a.scene.formulas.device.driver<mb3d_native::HybridDifs&&a.scene.base.host->mct.DEoption!=20){
            orbit_color_contract(a.scene);
            if(!a.scene.formulas.device.formula_type)color_on_it_contract(header,addon,r);
            volume_light_contract(header,addon,r);orbitChecked=true;
        }
        check(!a.reflections&&std::all_of(a.lighting_requirements.begin(),a.lighting_requirements.end(),[](const auto& r){
            return r.kind=="diffuse_map"||r.kind=="light_map";})||a.lighting_requirements.empty(),
            "unsupported resources in accepted plan");
        if(!resourcesChecked&&a.scene.formulas.maps.empty()) {
            resource_intent(header,addon,r);resourcesChecked=true;
        }
        const auto resources=PlanNativeRenderResources(a,{uint64_t(1)<<32,uint64_t(8)<<30});
        pixel_state_contract(a.scene.formulas.device);
        if(resources.execution->independent_primary_state())++independentContracts;
        if(resources.primary_state.production_chain||resources.execution->independent_primary_state()){
            check(!resources.primary_persistent&&resources.persistent,"primary proof widened other stage admission");
            check(resources.buffers[18]==16&&resources.buffers[19]==16&&resources.buffers[20]==16,"unused primary lanes allocated");
            // Closed proofs keep persistent shadow tasks; the state contract
            // moves shadows to whole rows without lane state.
            const bool stateRows=resources.execution->independent_primary_state();
            if(a.shadows&&!resources.parallel_soft_shadows)
                check(stateRows?(!resources.shadow_persistent&&resources.buffers[21]==16&&resources.buffers[22]==16):
                    resources.buffers[22]>16,"shadow state allocation changed");
        }
        if(resources.parallel_soft_shadows&&resources.execution->soft_shadow_pixel_proof()==1){
            check(a.soft_shadows&&!resources.shadow_persistent&&resources.soft_shadow_state.entry==2,
                "wrong soft-shadow entry");
            for(unsigned i=21;i<=24;++i)check(resources.buffers[i]==16,"unused shadow lanes allocated");
            const auto saved=b.inside.initial_inside;b.inside.initial_inside=1;
            const auto guarded=PlanNativeRenderResources(b,{uint64_t(1)<<32,uint64_t(8)<<30});
            b.inside.initial_inside=saved;
            check(!guarded.parallel_soft_shadows&&guarded.shadow_persistent&&guarded.buffers[22]>16,
                "inside shadow policy lost persistent fallback");
            check(guarded.primary_persistent==resources.primary_persistent&&
                guarded.parallel_reflections==resources.parallel_reflections,"shadow proof changed other stage policy");
        }
        check(resources.allocated_bytes<=resources.budget,"workspace budget");
        uint64_t sum=0;for(auto n:resources.buffers)sum+=n;
        check(sum+NativeDOFAllocationBytes(a.dof,resources.dof_limits)+a.z_normal_bytes+a.ao_bytes==resources.allocated_bytes,"workspace sum");
        bool threw=false;try{(void)PlanNativeRenderResources(a,{1,1});}catch(const std::invalid_argument&){threw=true;}
        check(threw,"tiny device accepted");
        auto invalid=r;invalid.width=0;auto failed=PrepareNativeRenderPlan(header,addon,invalid);
        check(!failed.ready()&&failed.stages.size()==1,"invalid dimensions admitted");
        auto c=PrepareNativeRenderPlan(header,addon,r);descriptors(a,c);
        check(CurrentPrecisionMode()==PrecisionMode::Compatible,"precision after failed job");
        auto off=r;off.no_ao=true;auto ao=PrepareNativeRenderPlan(header,addon,off);
        check(ao.ready()&&!ao.ambient&&!ao.ao_bytes,"AO override");
        check(ao.scene.base.host->mct.bCalcAmbShadow==a.scene.base.host->mct.bCalcAmbShadow,"secondary AO changed");
        off=r;off.no_dof=off.no_reflections=true;auto effects=PrepareNativeRenderPlan(header,addon,off);
        check(effects.ready()&&!effects.dof.version&&!effects.reflections,"effect override");
        off=r;off.no_shadows=true;auto shadows=PrepareNativeRenderPlan(header,addon,off);
        check(!shadows.shadows,"shadow override");
        if(a.soft_shadows)check(!shadows.ready()&&shadows.firstFailure()==
            "Unsupported Native general soft-shadow color capability","soft mask guard lost");
        else check(shadows.ready(),"hard shadow override");
        if(a.reflections&&shadows.ready())same(a.shadow,shadows.shadow,"secondary shadow descriptor changed");
        // Soft no-shadows fails after GetMCT/light preparation, not just at the
        // request boundary. Repeated descriptors must survive that late failure.
        auto restored=PrepareNativeRenderPlan(header,addon,r);descriptors(a,restored);
        check(CurrentPrecisionMode()==PrecisionMode::Compatible,"precision after late failure");
        off=r;off.logical_workers=4097;auto many=PrepareNativeRenderPlan(header,addon,off);
        check(many.logical_workers==many.scene.base.device.height,"worker clamp");
    }
    std::cout<<"PASS render plan checks="<<checks<<" accepted="<<accepted<<" rejected="<<rejected
        <<" state_contracts="<<stateContracts<<" independent_contracts="<<independentContracts
        <<" resource_rejected="<<resourceRejected<<'\n';return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<'\n';return 1;}}
