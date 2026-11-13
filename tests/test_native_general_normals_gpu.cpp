#include "render/native_metal/general_renderer.h"
#include "render/native_metal/general_scene_host.h"
#include "render/native_metal/general_normals_host.h"
#include "render/metal_renderer.h"
#include "render/backend.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "core/numeric_precision.h"
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include <array>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;
using namespace mb3d;
static void require(bool condition,const std::string& text){if(!condition)throw std::runtime_error(text);}
static std::string quote(const std::string& text){std::string out="\"";for(unsigned char c:text){
    if(c=='\\'||c=='\"')out+='\\';if(c>=32)out+=char(c);else out+=' ';}return out+'\"';}
static bool sameSurface(const RenderResult& a,const RenderResult& b){return a.silight.size()==b.silight.size()&&
    std::memcmp(a.silight.data(),b.silight.data(),a.silight.size()*sizeof(TsiLight5))==0;}
static bool sameWork(const MetalExecutionActivity& a,const MetalExecutionActivity& b){
    return a.distance_calls==b.distance_calls&&a.formula_work==b.formula_work&&a.normal_samples==b.normal_samples&&
        a.normal_de_calls==b.normal_de_calls&&a.normal_orbit_calls==b.normal_orbit_calls&&
        a.normal_evaluations==b.normal_evaluations&&a.primary_rough_pixels==b.primary_rough_pixels&&
        a.shadow_rays==b.shadow_rays&&a.shadow_steps==b.shadow_steps&&a.shadow_hits==b.shadow_hits;}
int main(int argc,char** argv){fs::path output;bool owned=false;try{
    require(argc==4,"usage: native-general-normals-test SOURCE_ROOT LIBRARY NEW_OUTPUT_DIRECTORY");
    const fs::path root=fs::absolute(argv[1]),library=fs::absolute(argv[2]);output=fs::absolute(argv[3]);
    require(!fs::exists(output),"Output must be new");fs::create_directories(output);owned=true;
    const fs::path source=root/"comparison/benchmark_scenes/effects-v1/integerpower8-reflections.m3p";
    const auto loaded=LoadM3PFile(source.string());require(loaded.success,loaded.error);
    require(!(loaded.header.MCoptions&1),"Deferred input");
    RenderOptions options;options.precision=PrecisionMode::Native;options.profile_timing=true;
    options.width_override=64;options.height_override=48;options.thread_count=6;options.deterministic_mode=true;
    options.resource_root=root.string();
    unsigned checks=0,caseCount=0;std::array<std::array<RenderResult,2>,2> baselines;
    std::ofstream rows(output/"cases.jsonl");require(bool(rows),"Cannot create case log");
    // Every requested algorithm is exercised in an ordinary complete render,
    // including two opaque secondary bounces, AO and directional shadows.
    for(unsigned reflection=0;reflection<2;++reflection)for(unsigned direct=0;direct<2;++direct)
    for(unsigned smooth:{0u,1u,8u,15u}){
        const std::string name=std::string(direct?"smooth-orbit":"de")+"-s"+std::to_string(smooth)+
            (reflection?"-reflections":"-primary");
        auto input=loaded;input.header.bCalc3D=1;input.header.bNormalsOnDE=direct?0:1;
        input.header.iOptions=(input.header.iOptions&~(15u<<6))|(smooth<<6);
        if(!reflection)input.header.bCalcSRautomatic&=uint8_t(~1u);
        {ScopedNumericPrecision precision(PrecisionMode::Native);
            auto prepared=native_metal::PrepareGeneralScene(input.header,input.addon,64,48,root.string());
            const auto params=native_metal::PrepareGeneralNormals(prepared.base.host->mct,prepared.formulas.device);
            require(params.normals_on_de==unsigned(!direct)&&params.smoothing==smooth,"Requested normal algorithm was silently replaced");++checks;}
        auto cpuInput=input;auto cpu=Render(cpuInput.header,cpuInput.addon,options);
        require(cpu.success,"Native CPU control: "+cpu.error);++checks;
        auto cpuRepeatInput=input;auto cpuRepeat=Render(cpuRepeatInput.header,cpuRepeatInput.addon,options);
        require(cpuRepeat.success&&cpu.image==cpuRepeat.image&&sameSurface(cpu,cpuRepeat),
            "Native CPU RGB/surface repeat changed");++checks;
        require(WritePNG((output/(name+"-cpu.png")).string(),cpu.image.data(),64,48,3),"CPU image write failed");
        RenderResult first;MetalExecutionActivity work;ReflectionActivity reflectionWork;
        for(unsigned repeat=0;repeat<3;++repeat){auto frame=input;
            auto actual=native_metal::RenderGeneralNativeMetal(frame.header,frame.addon,options,library.string());
            require(actual.status==NativeMetalStatus::Completed&&actual.result.success,"GPU "+name+": "+actual.reason);++checks;
            const auto& e=actual.execution;
            require(e.engine=="general_formula"&&e.validated&&e.profile_kind==2&&e.arithmetic_bits==32&&
                !e.faults&&!e.nonfinite&&!e.work_exhausted,"Invalid GPU numeric/activity record");++checks;
            require(e.primary_records==64*48&&e.normal_records==64*48&&e.color_records==64*48&&
                e.surface_records==2*64*48,"Incomplete full frame");++checks;
            require(e.normal_evaluations&&e.normal_samples==e.normal_evaluations*
                mb3d_native::GeneralNormalSamples(!direct,smooth),"Wrong actual normal sampling count");++checks;
            require(direct?(e.normal_orbit_calls==e.normal_samples&&!e.normal_de_calls):
                (e.normal_de_calls==e.normal_samples&&!e.normal_orbit_calls),"Wrong DE/orbit normal execution");++checks;
            require(std::string(actual.result.profile.stages[size_t(RenderProfileStage::AO)].state)=="completed"&&
                std::string(actual.result.profile.stages[size_t(RenderProfileStage::Shadows)].state)=="completed"&&
                e.shadow_rays&&e.shadow_steps,"Active ordinary effects were dropped");++checks;
            require(actual.reflections.enabled==bool(reflection)&&e.reflection_records==(reflection?64*48:0),"Wrong reflection mode");++checks;
            if(reflection){require(actual.reflections.hits&&actual.reflections.rays&&actual.reflections.changed_pixels&&
                actual.reflections.ao_rays&&actual.reflections.shadow_rays&&!actual.reflections.faults,
                "Missing actual secondary shading/normal work");++checks;}
            if(!repeat){first=std::move(actual.result);work=e;reflectionWork=actual.reflections;}
            else {require(first.image==actual.result.image&&sameSurface(first,actual.result)&&sameWork(work,e),"RGB/surface/work repeat changed");++checks;
                require(reflectionWork.hits==actual.reflections.hits&&reflectionWork.rays==actual.reflections.rays&&
                    reflectionWork.ray_steps==actual.reflections.ray_steps&&reflectionWork.ao_steps==actual.reflections.ao_steps&&
                    reflectionWork.shadow_steps==actual.reflections.shadow_steps,"Secondary work repeat changed");++checks;}
        }
        if(smooth){require(work.primary_rough_pixels>0,"Smoothing did not produce actual packed roughness");++checks;}
        // Exercise the production selector as well as the direct general API.
        // The old default specialized engine remains an independent baseline;
        // its native arithmetic need not match the general engine's output.
        RenderResult selectedBaseline;
        const bool advanced=direct||smooth;
        if(advanced)selectedBaseline=first;
        else {auto frame=input;auto specialized=RenderNativeMetal(frame.header,frame.addon,options,library.string());
            require(specialized.status==NativeMetalStatus::Completed&&specialized.result.success&&
                specialized.execution.engine=="exact_capability","Default specialized native path was lost");++checks;
            selectedBaseline=std::move(specialized.result);
            require(WritePNG((output/(name+"-specialized-control.png")).string(),selectedBaseline.image.data(),64,48,3),
                "Specialized control image write failed");}
        for(auto route:{RenderBackend::Metal,RenderBackend::Auto})for(unsigned repeat=0;repeat<2;++repeat){
            auto frame=input;BackendOptions backend;backend.requested=route;backend.metal_library=library.string();
            auto selected=RenderWithBackend(frame.header,frame.addon,options,backend);
            require(selected.render.success&&selected.backend.effective==RenderBackend::Metal&&!selected.backend.fallback,
                "Main Metal/auto path fell back or failed: "+selected.backend.fallback_reason+selected.render.error);++checks;
            require(selected.backend.execution.validated&&
                selected.backend.execution.engine==(advanced?"general_formula":"exact_capability")&&
                !selected.backend.execution.faults&&!selected.backend.execution.nonfinite&&!selected.backend.execution.work_exhausted,
                "Main selector numeric profile/engine changed");++checks;
            require(selected.render.image==selectedBaseline.image&&sameSurface(selected.render,selectedBaseline),
                "Main selector RGB/surface differs from selected engine baseline or repeat");++checks;
        }
        size_t changedCPU=0,changedBase=0,changedSurface=0,foreground=0,background=0;unsigned maxDelta=0;uint64_t absoluteError=0;
        std::set<uint32_t> colors;
        for(size_t i=0;i<first.image.size();++i){const unsigned delta=unsigned(std::abs(int(first.image[i])-int(cpu.image[i])));
            maxDelta=std::max(maxDelta,delta);absoluteError+=delta;}
        for(size_t i=0;i<first.silight.size();++i){const auto* a=&first.image[i*3];const auto* b=&cpu.image[i*3];
            changedCPU+=std::memcmp(a,b,3)!=0;colors.insert(a[0]|(uint32_t(a[1])<<8)|(uint32_t(a[2])<<16));
            foreground+=first.silight[i].Zpos<32768;background+=first.silight[i].Zpos>=32768;
            if(smooth){const auto& base=baselines[reflection][direct];changedBase+=std::memcmp(a,&base.image[i*3],3)!=0;
                changedSurface+=std::memcmp(&first.silight[i],&base.silight[i],sizeof(TsiLight5))!=0;}}
        require(foreground&&background&&colors.size()>16,"Uninformative full-frame control");++checks;
        if(smooth){require(changedBase&&changedSurface,"Smoothing control did not change image and surface");++checks;}
        else baselines[reflection][direct]=first;
        require(WritePNG((output/(name+"-metal.png")).string(),first.image.data(),64,48,3),"Metal image write failed");
        rows<<"{\"name\":"<<quote(name)<<",\"normal_mode\":"<<quote(direct?"smooth_iterations":"distance_estimate")
            <<",\"smoothing\":"<<smooth<<",\"reflection\":"<<(reflection?"true":"false")<<",\"exact_repeats\":3"
            <<",\"cpu_exact_repeats\":2,\"main_routes\":[\"metal\",\"auto\"],\"main_repeats_per_route\":2"
            <<",\"main_engine\":"<<quote(advanced?"general_formula":"exact_capability")
            <<",\"normal_samples\":"<<work.normal_samples<<",\"normal_de_calls\":"<<work.normal_de_calls
            <<",\"normal_orbit_calls\":"<<work.normal_orbit_calls<<",\"normal_evaluations\":"<<work.normal_evaluations
            <<",\"primary_rough_pixels\":"<<work.primary_rough_pixels<<",\"reflection_hits\":"<<reflectionWork.hits
            <<",\"changed_from_unsmoothed_pixels\":"<<changedBase<<",\"changed_from_unsmoothed_surface\":"<<changedSurface
            <<",\"cpu_metal_different_pixels\":"<<changedCPU<<",\"cpu_metal_max_channel_error\":"<<maxDelta
            <<",\"cpu_metal_mean_absolute_error\":"<<double(absoluteError)/first.image.size()<<"}\n";
        rows.flush();require(bool(rows),"Case log write failed");++caseCount;
    }
    // The two algorithms must actually differ on this discriminating scene.
    for(unsigned reflection=0;reflection<2;++reflection){require(baselines[reflection][0].image!=baselines[reflection][1].image&&
        !sameSurface(baselines[reflection][0],baselines[reflection][1]),"Direct orbit and DE normal controls are indistinguishable");++checks;}
    std::ofstream summary(output/"summary.json");summary<<"{\"schema\":\"mb3d.native-general-normal-render-test.v1\",\"status\":\"PASS_SCOPED\","
        <<"\"cases\":"<<caseCount<<",\"checks\":"<<checks<<",\"width\":64,\"height\":48,\"threads\":6,\"repeats\":3,\"cpu_repeats\":2,\"main_repeats_per_route\":2,"
        <<"\"limits\":[\"Native FP32 GPU versus Native FP64 CPU differences are reported, not parity-qualified.\","
        <<"\"Only this ordinary opaque fixture and normal settings; no full corpus or parameter coverage inferred.\"]}\n";
    require(bool(summary),"Summary write failed");std::cout<<checks<<" full-render normal checks, "<<caseCount<<" cases\n";return 0;
}catch(const std::exception& e){if(owned){std::ofstream failure(output/"FAILED.json");failure<<"{\"status\":\"FAILED\",\"reason\":"<<quote(e.what())<<"}\n";}
    std::cerr<<e.what()<<'\n';return 1;}}
