#include "shadow_host.h"
#include "shadow_trace_math.h"
#include "scene_host_internal.h"
#include "core/renderer.h"
#include <array>
#include <bit>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
using namespace mb3d_metal_exact;
struct Checks {
    unsigned long long count=0,failures=0;
    void operator()(bool ok,const char* what,unsigned pixel=0) {
        ++count;
        if(!ok&&++failures<=24)std::fprintf(stderr,"%s pixel %u\n",what,pixel);
    }
};
F64 number(double value) { return D(std::bit_cast<ExactU64>(value)); }
double native(F64 value) { return std::bit_cast<double>(value.bits); }
bool same_state(const ShadowRowState& a,const ShadowRowState& b) {
    if(a.last_z.bits!=b.last_z.bits)return false;
    for(unsigned light=0;light<6;++light)
        if(a.threshold[light]!=b.threshold[light]||a.last_found[light].bits!=b.last_found[light].bits)return false;
    return true;
}
ShadowRowState seed_state(int counter,double found,double z) {
    ShadowRowState state{};
    state.threshold[0]=counter;state.last_found[0]=number(found);state.last_z=number(z);
    for(unsigned light=1;light<6;++light){
        state.threshold[light]=int(light%4);
        state.last_found[light]=F64(int(100+light));
    }
    return state;
}
void compare_transition(Checks& check,ExactScene s,ExactShadowParams p,
                        ExactShadowRay ray,ExactShadowTrace trace,ExactSurface surface,
                        ShadowRowState initial,const char* what,unsigned pixel) {
    auto ordered=initial,traced=initial;
    auto expected=shadow_pixel_prepared(s,p,ray,surface,ordered);
    auto actual=shadow_pixel_traced(s,p,ray,trace,surface,traced);
    check(!std::memcmp(&actual,&expected,sizeof(actual)),what,pixel);
    check(same_state(traced,ordered),"complete row state equals ordered march",pixel);
}

void synthetic_cases(Checks& check,ExactScene scene,ExactShadowParams shadow) {
    shadow.enabled=1;
    ExactSurface surface{};surface.shadow=0x7ff;surface.amb_shadow=5000;
    ExactShadowRay ray{};ray.version=1;ray.active=1;ray.length=F64(10).bits;
    ray.threshold=F64(1000000).bits;ray.initial_stop=F64(1).to_f32_bits();
    ray.initial_de=F64(1).bits;
    ExactShadowTrace trace{};trace.version=1;trace.active=1;trace.has_loop=1;
    trace.min_remaining=F64(10).bits;trace.final_remaining=F64(5).bits;
    check(valid_prepared_shadow_ray(ray)&&valid_shadow_trace(trace),"synthetic boundary fixture is valid");

    // The preamble subtracts |7-0| and one from last_found. The skip comparison
    // must remain strict and must use the actual loop-entry checkpoint at 10,
    // not the post-step terminal remaining value at 5.
    for(double incoming:{17.0,18.0,19.0,
                         std::nextafter(18.0,0.0),
                         std::nextafter(18.0,std::numeric_limits<double>::infinity())}){
        auto state=seed_state(3,incoming,7);
        auto output=shadow_pixel_traced(scene,shadow,ray,trace,surface,state);
        bool skipped=incoming>18.0;
        check(output.fault==0&&bool(output.shadow&1024)==!skipped,"strict checkpoint boundary");
        check(state.threshold[0]==(skipped?2:0),"boundary preserves or clears counter");
        check(state.last_found[0].bits==number(incoming-8.0).bits,"boundary preserves adjusted last_found");
        check(state.last_z.bits==F64(0).bits,"successful boundary updates last_z");
    }
    for(unsigned fault:{2u,4u}){
        auto failing=trace;failing.terminal_fault=fault;
        failing.final_threshold=3;failing.writes_last_found=1;failing.last_found=F64(6).bits;
        check(valid_shadow_trace(failing),"partial history fault summary is valid");
        auto no_skip=seed_state(0,123,7);
        auto output=shadow_pixel_traced(scene,shadow,ray,failing,surface,no_skip);
        check(output.fault==fault&&!(output.shadow&1024),"terminal fault keeps cleared shadow");
        check(no_skip.threshold[0]==3&&no_skip.last_found[0].bits==F64(6).bits,
              "terminal fault retains preceding no-skip writes");
        check(no_skip.last_z.bits==F64(7).bits,"terminal fault leaves last_z unchanged");
        auto equality=seed_state(3,18,7);
        output=shadow_pixel_traced(scene,shadow,ray,failing,surface,equality);
        check(output.fault==fault&&equality.last_z.bits==F64(7).bits,
              "entry equality does not suppress terminal fault");
        check(equality.threshold[0]==2&&equality.last_found[0].bits==F64(10).bits,
              "skip mode ignores no-skip history even when it faults");
        auto between=seed_state(3,15,7); // Adjusted 7 is between terminal 5 and entry 10.
        output=shadow_pixel_traced(scene,shadow,ray,failing,surface,between);
        check(output.fault==fault&&between.last_z.bits==F64(7).bits,
              "no invented checkpoint after terminal step or budget");
        auto early=seed_state(3,19,7);
        output=shadow_pixel_traced(scene,shadow,ray,failing,surface,early);
        check(output.fault==0&&!(output.shadow&1024),"earlier shortcut suppresses latent fault");
        check(early.threshold[0]==2&&early.last_found[0].bits==F64(11).bits&&early.last_z.bits==F64(0).bits,
              "shortcut keeps adjusted history and advances last_z");
    }

    auto no_write=trace;no_write.final_remaining=F64(-1).bits;
    auto stale=seed_state(0,123,7);
    auto output=shadow_pixel_traced(scene,shadow,ray,no_write,surface,stale);
    check(output.fault==0&&stale.last_found[0].bits==F64(123).bits,
          "no last_found write preserves stale incoming bits");
    auto cleared=no_write;cleared.writes_last_found=1;cleared.last_found=F64(6).bits;
    stale=seed_state(0,123,7);
    output=shadow_pixel_traced(scene,shadow,ray,cleared,surface,stale);
    check(output.fault==0&&stale.threshold[0]==0&&stale.last_found[0].bits==F64(6).bits,
          "last_found write survives later counter clear");

    for(unsigned mutation=0;mutation<12;++mutation){
        auto invalid=trace;
        if(mutation==0)invalid.version=2;
        if(mutation==1)invalid.reserved[0]=1;
        if(mutation==2)invalid.reserved[2]=1;
        if(mutation==3)invalid.min_remaining=0x7ff8000000000000ULL;
        if(mutation==4)invalid.final_remaining=0x7ff0000000000000ULL;
        if(mutation==5)invalid.has_loop=2;
        if(mutation==6)invalid.final_threshold=1;
        if(mutation==7)invalid.writes_last_found=2;
        if(mutation==8)invalid.terminal_fault=8;
        if(mutation==9)invalid.active=0;
        if(mutation==10)invalid.min_remaining=F64(11).bits; // Valid record, inconsistent with this ray.
        if(mutation==11){invalid={};invalid.version=1;invalid.fault=1;}
        auto state=seed_state(3,19,7),before=state;
        output=shadow_pixel_traced(scene,shadow,ray,invalid,surface,state);
        check(output.fault==1&&output.shadow==surface.shadow,"invalid trace rejected before pixel mutations",mutation);
        check(same_state(state,before),"invalid trace preserves every state field",mutation);
    }
    for(unsigned preparation_fault:{1u,4u,8u}){
        auto invalid_ray=ray;invalid_ray.fault=preparation_fault;
        auto prepared_trace=trace_shadow_ray(scene,shadow,invalid_ray);
        compare_transition(check,scene,shadow,invalid_ray,prepared_trace,surface,
                           seed_state(3,19,7),"preparation fault matches ordered behavior",preparation_fault);
    }

    auto disabled=shadow;disabled.enabled=0;
    auto disabled_trace=trace_shadow_ray(scene,disabled,ray);
    auto state=seed_state(3,19,7),before=state;
    output=shadow_pixel_traced(scene,disabled,ray,disabled_trace,surface,state);
    check(!std::memcmp(&output,&surface,sizeof(output))&&same_state(state,before),"disabled pass has no mutations");
    auto background=surface;background.depth_packed=0x80000000u;
    ExactShadowRay empty{};empty.version=1;
    auto empty_trace=trace_shadow_ray(scene,shadow,empty);
    state=seed_state(3,19,7);
    output=shadow_pixel_traced(scene,shadow,empty,empty_trace,background,state);
    check(output.fault==0&&!(output.shadow&1024)&&state.threshold[0]==2&&
          state.last_found[0].bits==F64(19).bits&&state.last_z.bits==F64(7).bits,
          "background decrements counters without applying active-pixel history");
    compare_transition(check,scene,shadow,empty,empty_trace,background,before,"background ordered transition",0);
    auto backface=ray;backface.backface=1;
    auto backface_trace=trace_shadow_ray(scene,shadow,backface);
    state=seed_state(3,19,7);
    output=shadow_pixel_traced(scene,shadow,backface,backface_trace,surface,state);
    check(output.fault==0&&(output.shadow&1024)&&state.threshold[0]==0&&
          state.last_found[0].bits==F64(11).bits&&state.last_z.bits==F64(0).bits,
          "backface applies history before blocking and advances last_z");
    compare_transition(check,scene,shadow,backface,backface_trace,surface,before,"backface ordered transition",0);
    for(int length:{0,-1}){
        auto short_ray=ray;short_ray.length=F64(length).bits;
        auto short_trace=trace_shadow_ray(scene,shadow,short_ray);
        check(!short_trace.has_loop&&short_trace.final_remaining==F64(length).bits,"nonpositive ray has no checkpoints");
        compare_transition(check,scene,shadow,short_ray,short_trace,surface,before,"nonpositive length ordered transition",0);
    }

    // A finite but extreme coordinate produces a real DE fault after the first
    // checkpoint. A row shortcut must be able to suppress that later fault.
    auto overflowing=ray;overflowing.position[0]=number(std::numeric_limits<double>::max()).bits;
    auto overflow_trace=trace_shadow_ray(scene,shadow,overflowing);
    check(overflow_trace.fault==0&&overflow_trace.terminal_fault==4&&overflow_trace.has_loop,
          "actual overflowing DE produces a latent numerical fault");
    for(int counter:{0,3})
        compare_transition(check,scene,shadow,overflowing,overflow_trace,surface,
                           seed_state(counter,19,7),"actual numerical fault shortcut equivalence",counter);

    // Strictly decreasing remaining length is not guaranteed by the accepted
    // scalar types: this positive Single limiter underflows to a zero step.
    auto tiny=scene;tiny.step_limiter=1;tiny.de_stop=number(0.001).to_f32_bits();
    tiny.de_stop_factor=0;tiny.de_sub=0;tiny.z_step_div=F64(1).to_f32_bits();
    tiny.de_scale=F64(1).to_f32_bits();tiny.step_width_103=F64(1).to_f32_bits();
    auto bounded=shadow;bounded.max_steps=3;
    auto stalled=ray;stalled.position[0]=F64(100).bits;stalled.position[1]=F64(100).bits;
    stalled.position[2]=F64(100).bits;stalled.threshold=F64(1).bits;
    stalled.initial_stop=tiny.de_stop;
    auto initial=distance({D(stalled.position[0]),D(stalled.position[1]),D(stalled.position[2])},S(tiny.de_stop),tiny);
    stalled.initial_de=initial.distance.bits;stalled.initial_iterations=initial.iteration;
    auto stalled_trace=trace_shadow_ray(tiny,bounded,stalled);
    check(valid_scene(tiny)&&valid_prepared_shadow_ray(stalled),"zero-progress fixture remains structurally valid");
    check(stalled_trace.terminal_fault==2&&stalled_trace.min_remaining==F64(10).bits&&
          stalled_trace.final_remaining==F64(10).bits,"non-strict remaining sequence preserves finite work bound");
    compare_transition(check,tiny,bounded,stalled,stalled_trace,surface,
                       seed_state(0,123,0),"zero progress exact exhaustion",0);
}

void scene_cases(Checks& check,const char* path,unsigned width,unsigned height) {
    auto prepared=mb3d::metal_exact::PrepareExactScene(path,width,height);
    auto primary=mb3d::metal_exact::ReferencePrimary(prepared,4);
    auto surfaces=mb3d::metal_exact::ReferenceSurface(prepared,4);
    const auto scene=prepared.device;
    std::vector<ExactShadowRay> rays(primary.size());
    std::vector<ExactShadowTrace> traces(primary.size());
    for(bool enabled:{false,true}){
        auto shadow=mb3d::metal_exact::PrepareShadow(prepared,enabled);
        auto reference=mb3d::metal_exact::ReferenceShadow(prepared,surfaces,enabled);
        std::vector<unsigned> marching;
        for(unsigned i=0;i<primary.size();++i){
            rays[i]=prepare_shadow_ray(scene,shadow,primary[i],surfaces[i],i/width);
            traces[i]=trace_shadow_ray(scene,shadow,rays[i]);
            check(valid_shadow_trace(traces[i])&&!traces[i].fault,"real-scene trace ABI is valid",i);
            if(traces[i].has_loop)marching.push_back(i);
        }
        std::vector<ExactSurface> actual(primary.size());
        for(unsigned y=0;y<height;++y){
            ShadowRowState ordered{},traced{};
            for(unsigned x=0;x<width;++x){
                unsigned i=y*width+x;
                auto expected=shadow_pixel_prepared(scene,shadow,rays[i],surfaces[i],ordered);
                actual[i]=shadow_pixel_traced(scene,shadow,rays[i],traces[i],surfaces[i],traced);
                check(!std::memcmp(&actual[i],&expected,sizeof(ExactSurface)),"full traced row equals ordered march",i);
                check(same_state(traced,ordered),"full row state after every pixel",i);
                check(!std::memcmp(&actual[i],&reference[i],sizeof(ExactSurface)),"traced shadow equals production stage",i);
            }
        }
        auto header=prepared.host->file.header;auto addon=prepared.host->file.addon;
        if(enabled)header.bCalculateHardShadow|=1;else header.bCalculateHardShadow&=~1;
        mb3d::RenderOptions options;options.precision=mb3d::PrecisionMode::Compatible;options.no_ao=true;options.thread_count=4;
        mb3d::ScopedRenderContext binding(prepared.host->context);
        auto production=mb3d::Render(header,addon,options);
        if(!production.success)throw std::runtime_error(production.error);
        for(unsigned i=0;i<actual.size();++i){
            auto expected=mb3d::metal_exact::ExportSurface(production.silight[i]);
            check(!std::memcmp(&actual[i],&expected,sizeof(expected)),"traced shadow equals full production render",i);
        }
        if(!enabled)continue;
        check(!marching.empty(),"real scene exercises full ray marching");
        // Sample rays spread over the image. Expected transitions still execute
        // the original ordered march, with independently varied incoming state.
        for(unsigned sample=0;sample<8&&sample<marching.size();++sample){
            unsigned i=marching[size_t(sample)*marching.size()/8];
            double z=native(D(rays[i].z));
            const double adjustment=1+z*native(S(scene.de_stop_factor));
            for(double target:{native(D(traces[i].min_remaining)),native(D(traces[i].final_remaining))}){
                for(double edge:{std::nextafter(target,-std::numeric_limits<double>::infinity()),target,
                                 std::nextafter(target,std::numeric_limits<double>::infinity())}){
                    for(int counter=0;counter<4;++counter)
                        compare_transition(check,scene,shadow,rays[i],traces[i],surfaces[i],
                                           seed_state(counter,edge+adjustment,z),"incoming checkpoint-neighbor state",i);
                }
            }
            unsigned random=0x91e10da5u^(i+width);
            for(unsigned trial=0;trial<4;++trial){
                random=random*1664525u+1013904223u;
                double delta=(double((random>>8)%2001)-1000)*0.001;
                double found=native(D(rays[i].length))*(double(random&255)/128.0);
                compare_transition(check,scene,shadow,rays[i],traces[i],surfaces[i],
                                   seed_state(int(random%4),found,z+delta),"deterministic mixed incoming state",i);
            }
        }
        auto bounded=shadow;bounded.max_steps=1;
        unsigned exhausted=0;
        for(unsigned i=0;i<rays.size();++i)traces[i]=trace_shadow_ray(scene,bounded,rays[i]);
        for(unsigned y=0;y<height;++y){
            ShadowRowState ordered{},traced{};
            for(unsigned x=0;x<width;++x){
                unsigned i=y*width+x;
                auto expected=shadow_pixel_prepared(scene,bounded,rays[i],surfaces[i],ordered);
                auto output=shadow_pixel_traced(scene,bounded,rays[i],traces[i],surfaces[i],traced);
                check(!std::memcmp(&output,&expected,sizeof(output)),"actual one-step bound output parity",i);
                check(same_state(traced,ordered),"actual one-step bound partial-state parity",i);
                exhausted+=(output.fault&2)!=0;
            }
        }
        check(exhausted>0,"one-step bound exercises exhausted rays");
        if(width==64)synthetic_cases(check,scene,shadow);
    }
}
} // namespace

int main(int argc,char** argv) {
    const mb3d::ScopedNumericPrecision precision(mb3d::PrecisionMode::Compatible);
    if(argc!=2)return 2;
    Checks check;
    try{
        for(const auto size:{std::pair{64u,48u},std::pair{63u,47u},std::pair{127u,95u}})
            scene_cases(check,argv[1],size.first,size.second);
    }catch(const std::exception& error){
        std::fprintf(stderr,"shadow trace exception: %s\n",error.what());return 2;
    }
    std::printf("exact parallel shadow trace: %llu checks, %llu failures\n",check.count,check.failures);
    return check.failures?1:0;
}
