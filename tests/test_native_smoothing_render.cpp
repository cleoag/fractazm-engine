// Full main-engine control. No fallback is possible through this direct API.
// Policy0 is the previous expression on the SAME general engine/library.
#include "render/backend.h"
#include "render/native_metal/general_renderer.h"
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <array>
#include <tuple>
#include <cmath>
#include <cstring>
#include <map>
namespace fs=std::filesystem;
using namespace mb3d;
static unsigned checks;
static void need(bool b,const std::string&s){++checks;if(!b)throw std::runtime_error(s);}
static auto work(const MetalExecutionActivity&e){std::map<std::string,std::string> out;
#define W(x) out[#x]=std::to_string(e.x)
    W(validated);W(profile_kind);W(storage_bytes);W(arithmetic_bits);
    W(primary_records);W(normal_records);W(surface_records);W(shadow_records);W(color_records);W(reflection_records);
    W(faults);W(nonfinite);W(work_exhausted);W(distance_calls);W(formula_work);
    W(smoothing_record_version);W(smoothing_policy);W(smoothing_calls);W(smoothing_recovered);W(smoothing_stagnant);W(smoothing_invalid);
    W(normal_samples);W(normal_de_calls);W(normal_orbit_calls);W(normal_evaluations);W(primary_rough_pixels);
    W(shadow_rays);W(shadow_hits);W(shadow_misses);W(shadow_steps);W(shadow_changed);
    W(dof_records);W(dof_passes);W(dof_positive_radius_sources);W(dof_sort_work);W(dof_candidate_scan_checks);
    W(dof_candidate_checks);W(dof_contributing_pairs);W(dof_changed_pixels);W(dof_maximum_radius);W(dof_dispatches);
    W(znormal_records);W(znormal_foreground);W(znormal_changed);W(znormal_neighbors);W(znormal_dispatches);
    W(camera_records);W(camera_cut_starts);W(camera_cut_misses);W(camera_planar_rays);W(camera_spherical_rays);
    W(camera_full_width);W(camera_full_height);W(camera_left);W(camera_top);
    W(inside_records);W(inside_attempts);W(inside_retries);W(inside_initial_hits);W(inside_interior_surfaces);W(inside_smooth_calls);
    W(reflection_inside_records);W(reflection_inside_selected_pixels);W(reflection_inside_ao_rays);W(reflection_inside_absorption_bounces);
    W(reflection_inside_initial_primary);W(reflection_inside_cut_ao_skips);W(reflection_inside_cut_shadow_limits);
    W(inside_initial_policy);W(inside_in_and_out);W(color_abi_version);W(color_flags);
    W(ao_record_version);W(ao_enabled);W(ao_algorithm);W(ao_quality);W(ao_requested_repeats);W(ao_records);
    W(ao_active);W(ao_changed);W(ao_samples);W(ao_depth_reads);W(ao_border_rejects);W(ao_rng_advances);W(ao_levels);
    W(ao_repeats);W(ao_de_queries);W(ao_de_steps);W(ao_cut_skips);W(ao_rays);W(ao_refinements);W(ao_formula_work);W(ao_dispatches);
#undef W
    return out; // All declared non-timing numerical fields; engine checked separately.
}
static auto reflection_work(const ReflectionActivity&r){return std::make_tuple(r.enabled,r.active_pixels,r.rays,r.hits,r.misses,
    r.changed_pixels,r.faults,r.max_bounces,r.ray_steps,r.primary_skipped,r.ao_rays,r.ao_steps,r.shadow_rays,r.shadow_steps);}
static bool same_surface(const RenderResult&a,const RenderResult&b){return a.silight.size()==b.silight.size()&&
    std::memcmp(a.silight.data(),b.silight.data(),a.silight.size()*sizeof(TsiLight5))==0;}
static void complete(const RenderResult&r,int workers){
    need(r.success&&r.status==RenderStatus::Completed&&r.width==64&&r.height==48&&r.image.size()==64*48*3&&r.silight.size()==64*48,"complete RGB/surface extent");
    need(r.profile.enabled&&r.profile.precision==PrecisionMode::Native&&r.profile.workers==workers,"effective Native profile/workers");
    for(const auto&s:r.profile.stages)need(std::isfinite(s.seconds)&&s.seconds>=0&&std::string(s.state)!="interrupted","no incomplete/nonfinite stage");
    for(auto stage:{RenderProfileStage::Preparation,RenderProfileStage::Primary,RenderProfileStage::Color})
        need(std::string(r.profile.stages[size_t(stage)].state)=="completed","required render stage complete");
    bool varied=false;for(size_t i=3;i<r.image.size();i+=3)if(r.image[i]!=r.image[0]||r.image[i+1]!=r.image[1]||r.image[i+2]!=r.image[2])varied=true;
    need(varied,"nonconstant whole image");
}
static void save(const fs::path&p,const RenderResult&r){need(WritePNG(p.string(),r.image.data(),r.width,r.height,3),"save control image");
    auto surface=p;surface.replace_extension(".silight");std::ofstream file(surface,std::ios::binary);
    file.write(reinterpret_cast<const char*>(r.silight.data()),std::streamsize(r.silight.size()*sizeof(TsiLight5)));file.close();need(bool(file),"save raw surface");}
static void observe(const fs::path&output,const std::string&label,const RenderResult&r,const MetalExecutionActivity*e=nullptr){
    // Separate valid JSON document per attempt, including a failed legacy run.
    std::ofstream file(output/(label+".json"));file<<"{\"label\":\""<<label<<"\",\"success\":"<<(r.success?"true":"false")
        <<",\"rgb_bytes\":"<<r.image.size()<<",\"surface_records\":"<<r.silight.size()<<",\"stages\":{";
    for(size_t i=0;i<r.profile.stages.size();++i){if(i)file<<',';const auto&s=r.profile.stages[i];
        file<<'"'<<RenderProfileStageName(RenderProfileStage(i))<<"\":{\"state\":\""<<s.state<<"\",\"seconds\":";
        if(std::isfinite(s.seconds))file<<s.seconds;else file<<"null";file<<'}';}
    file<<'}';if(e){file<<",\"engine\":\""<<e->engine<<"\",\"work\":{";bool first=true;for(const auto&[k,v]:work(*e)){if(!first)file<<',';first=false;file<<'"'<<k<<"\":"<<v;}file<<'}';}
    file<<"}\n";file.close();need(bool(file),"retain attempt records");}
int main(int argc,char**argv){try{
    need(argc==4||argc==5,"usage: native-smoothing-render SOURCE_ROOT LIBRARY NEW_OUTPUT [kali|ordinary|all]");
    const std::string selection=argc==5?argv[4]:"all";need(selection=="all"||selection=="kali"||selection=="ordinary","known selection");
    const fs::path root=fs::absolute(argv[1]),library=fs::absolute(argv[2]),output=fs::absolute(argv[3]);
    need(!fs::exists(output),"preserve prior output");fs::create_directories(output);
    const std::array<std::string,5> scenes={
        "comparison/benchmark_scenes/effects-v1/integerpower8-ao-shadows.m3p",
        "comparison/benchmark_scenes/effects-v1/sphereifs-ao-shadows.m3p",
        "comparison/benchmark_scenes/effects-v1/boxifs-ao-shadows.m3p",
        "comparison/scenes/extall_3D_KaliDucks.m3p",
        "comparison/scenes/extall_3D_Kalisets1.m3p"};
    RenderOptions opts;opts.precision=PrecisionMode::Native;opts.profile_timing=true;
    opts.width_override=64;opts.height_override=48;opts.thread_count=6;
    opts.deterministic_mode=true;opts.resource_root=root.string();
    std::ofstream report(output/"report.json");
    report<<"{\"schema\":\"mb3d.native-smoothing-full-render.v2\",\"selection\":\""<<selection<<"\",\"width\":64,\"height\":48,\"raw_surface_record_bytes\":"<<sizeof(TsiLight5)<<",\"cases\":[";
    unsigned selected=0;
    for(unsigned i=0;i<scenes.size();++i){
        if((selection=="kali"&&i<3)||(selection=="ordinary"&&i>=3))continue;
        auto input=LoadM3PFile((root/scenes[i]).string());need(input.success,input.error);
        need(!(input.header.MCoptions&1),"ordinary fixture");input.header.bCalc3D=1;
        const bool kali=i>=3;RenderResult first;MetalExecutionActivity activity;ReflectionActivity reflection;
        for(unsigned repeat=0;repeat<3;++repeat){auto local=input;opts.native_metal_smoothing_policy=1;
            auto gpu=native_metal::RenderGeneralNativeMetal(local.header,local.addon,opts,library.string());
            const auto label=std::to_string(i)+"-stable-"+std::to_string(repeat);observe(output,label,gpu.result,&gpu.execution);
            need(gpu.status==NativeMetalStatus::Completed&&gpu.result.success,"stable general failed: "+gpu.reason);
            complete(gpu.result,0);
            const auto&e=gpu.execution;
            need(e.validated&&e.engine=="general_formula"&&e.profile_kind==2&&e.storage_bytes==4&&e.arithmetic_bits==32&&
                e.smoothing_record_version==1&&e.smoothing_policy==1&&!e.faults&&!e.nonfinite&&!e.work_exhausted&&
                !e.smoothing_invalid,"valid observed smoothing profile/activity");
            need(e.primary_records==3072&&e.normal_records==3072&&e.surface_records==6144&&e.color_records==3072&&e.distance_calls&&e.formula_work,"actual whole-frame stage records");
            need(std::string(gpu.result.profile.stages[size_t(RenderProfileStage::Primary)].state)=="completed","primary stage complete");
            if(kali)need(e.smoothing_calls>0&&e.smoothing_stagnant>0,"actual equal-radius fallback must execute");
            else need(!e.smoothing_stagnant&&!e.smoothing_recovered,"ordinary control unexpectedly requires repaired interpolation");
            save(output/(std::to_string(i)+"-stable-"+std::to_string(repeat)+".png"),gpu.result);
            if(repeat){need(first.image==gpu.result.image&&same_surface(first,gpu.result),"stable RGB/surface exact repeat");
                need(work(activity)==work(e)&&reflection_work(reflection)==reflection_work(gpu.reflections),"all non-timing work exact repeat");}
            else{first=std::move(gpu.result);activity=e;reflection=gpu.reflections;}
        }
        auto local=input;opts.native_metal_smoothing_policy=0;
        auto legacy=native_metal::RenderGeneralNativeMetal(local.header,local.addon,opts,library.string());
        observe(output,std::to_string(i)+"-legacy",legacy.result,&legacy.execution);
        need(legacy.execution.engine=="general_formula"&&legacy.execution.smoothing_policy==0&&
            legacy.execution.smoothing_record_version==1,"same general engine and actual legacy policy");
        if(kali){need(legacy.status==NativeMetalStatus::Failed&&!legacy.result.success&&legacy.result.image.empty()&&
            legacy.result.silight.empty()&&legacy.execution.smoothing_invalid>0&&legacy.execution.faults>0&&legacy.execution.nonfinite>0,"old expression visibly fails with no published image");}
        else{need(legacy.status==NativeMetalStatus::Completed&&legacy.result.success&&first.image==legacy.result.image&&same_surface(first,legacy.result),
            "ordinary stable/legacy RGB/surface exact");save(output/(std::to_string(i)+"-legacy.png"),legacy.result);}
        // Native CPU is a descriptive comparison, not an FP32 equality oracle.
        local=input;opts.native_metal_smoothing_policy=1;auto cpu=Render(local.header,local.addon,opts);
        observe(output,std::to_string(i)+"-cpu",cpu);need(cpu.success,"Native CPU reference failed: "+cpu.error);complete(cpu,6);save(output/(std::to_string(i)+"-cpu.png"),cpu);
        local=input;auto cpuRepeat=Render(local.header,local.addon,opts);observe(output,std::to_string(i)+"-cpu-repeat",cpuRepeat);complete(cpuRepeat,6);
        need(cpu.image==cpuRepeat.image&&same_surface(cpu,cpuRepeat),"Native CPU RGB/surface exact repeat");save(output/(std::to_string(i)+"-cpu-repeat.png"),cpuRepeat);
        for(size_t stage=0;stage<first.profile.stages.size();++stage)need(std::string(first.profile.stages[stage].state)==cpu.profile.stages[stage].state,"CPU/Metal active stage inventory differs");
        if(kali)for(auto backend:{RenderBackend::Metal,RenderBackend::Auto}){local=input;BackendOptions selection;selection.requested=backend;selection.metal_library=library.string();
            auto main=RenderWithBackend(local.header,local.addon,opts,selection);const auto label=std::to_string(i)+"-"+RenderBackendName(backend);
            observe(output,label,main.render,&main.backend.execution);
            need(main.render.success&&main.backend.requested==backend&&main.backend.effective==RenderBackend::Metal&&!main.backend.fallback&&main.backend.execution.engine=="general_formula","main route must stay on general Metal");
            need(main.render.image==first.image&&same_surface(main.render,first)&&work(main.backend.execution)==work(activity)&&reflection_work(main.backend.reflections)==reflection_work(reflection),"main route image/surface/work differs");
            save(output/(label+".png"),main.render);}
        need(cpu.image.size()==first.image.size(),"CPU/GPU extent");size_t differing=0;uint64_t absolute=0;
        for(size_t p=0;p<first.image.size();p+=3){bool different=false;for(unsigned c=0;c<3;++c){
            const int delta=int(cpu.image[p+c])-int(first.image[p+c]);absolute+=uint64_t(std::abs(delta));different|=delta!=0;}differing+=different;}
        if(selected++)report<<',';report<<"{\"scene\":\""<<scenes[i]<<"\",\"stable_repeats\":3,\"cpu_repeats\":2,\"main_metal_auto_checked\":"<<(kali?"true":"false")<<",\"legacy_failed\":"<<(kali?"true":"false")
            <<",\"calls\":"<<activity.smoothing_calls<<",\"recovered\":"<<activity.smoothing_recovered
            <<",\"stagnant\":"<<activity.smoothing_stagnant<<",\"invalid\":"<<activity.smoothing_invalid
            <<",\"legacy_invalid\":"<<legacy.execution.smoothing_invalid<<",\"cpu_different_pixels\":"<<differing
            <<",\"cpu_absolute_channel_error\":"<<absolute<<'}';report.flush();
    }
    auto input=LoadM3PFile((root/scenes[0]).string());opts.native_metal_smoothing_policy=2;
    need(input.success,input.error);input.header.bCalc3D=1;
    auto bad=native_metal::RenderGeneralNativeMetal(input.header,input.addon,opts,library.string());
    observe(output,"unknown-policy",bad.result,&bad.execution);
    need(bad.status==NativeMetalStatus::Failed&&bad.execution.engine=="none"&&bad.result.image.empty()&&bad.result.silight.empty(),"unknown policy rejected before dispatch");
    report<<"],\"status\":\"PASS\",\"checks\":"<<checks<<"}\n";report.close();need(bool(report),"retain final report");
    std::cout<<"PASS "<<checks<<" main general smoothing controls; Native comparison only\n";return 0;
}catch(const std::exception&e){std::cerr<<"FAIL "<<e.what()<<'\n';return 1;}}
