#include "render/backend.h"
#include "render/native_metal/general_renderer.h"
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include <filesystem>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>

namespace fs=std::filesystem;
using namespace mb3d;
void require(bool ok,const std::string& message){if(!ok)throw std::runtime_error(message);}
size_t different(const RenderResult& a,const RenderResult& b){
    require(a.width==b.width&&a.height==b.height&&a.image.size()==b.image.size(),"Image extent changed");
    size_t n=0;for(size_t i=0;i<a.image.size();i+=3)
        n+=a.image[i]!=b.image[i]||a.image[i+1]!=b.image[i+1]||a.image[i+2]!=b.image[i+2];
    return n;
}
int main(int argc,char** argv){try{
    require(argc==4,"usage: native-dof-render SOURCE_ROOT LIBRARY NEW_OUTPUT");
    const fs::path root=fs::absolute(argv[1]),library=fs::absolute(argv[2]),out=fs::absolute(argv[3]);
    require(!fs::exists(out),"Preserve previous test output");fs::create_directories(out);
    const auto scenes=root/"comparison/benchmark_scenes/effects-v1";
    const auto dof=LoadM3PFile((scenes/"integerpower8-dof.m3p").string());
    require(dof.success&&bool(dof.header.bCalcDOFtype&1),"Missing active ordinary DOF fixture");
    RenderOptions options;options.precision=PrecisionMode::Native;options.profile_timing=true;
    options.width_override=64;options.height_override=48;options.thread_count=6;
    options.deterministic_mode=true;options.resource_root=root.string();
    std::ofstream report(out/"report.json");
    report<<"{\"schema\":\"mb3d.native-dof-full-render.v1\",\"width\":64,\"height\":48,\"cases\":[";
    unsigned checks=0;
    for(unsigned index=0;index<6;++index){
        const bool plain=index==2||index==3,reflection=index==4,external=index==5;
        const unsigned passes=(index==1||index==3)?4:1;
        const std::string name=external?"external-sorted":reflection?"reflections-sorted":
            std::string(plain?"plain-":"sorted-")+std::to_string(passes);
        auto input=reflection?LoadM3PFile((scenes/"integerpower8-reflections.m3p").string()):
            external?LoadM3PFile((root/"comparison/scenes/extall_3Da_ABoxVSWavy.m3p").string()):dof;
        require(input.success,input.error);
        input.header.bCalcDOFtype=uint8_t(1u|((passes-1)<<1)|(plain?8u:0u));
        input.header.sDOFaperture=dof.header.sDOFaperture;input.header.sDOFclipR=dof.header.sDOFclipR;
        input.header.sDOFZsharp=dof.header.sDOFZsharp;input.header.sDOFZsharp2=dof.header.sDOFZsharp2;
        auto cpuInput=input;const auto cpu=Render(cpuInput.header,cpuInput.addon,options);
        require(cpu.success,"Native CPU DOF failed: "+cpu.error);++checks;
        cpuInput=input;const auto cpuRepeat=Render(cpuInput.header,cpuInput.addon,options);
        require(cpuRepeat.success&&cpu.image==cpuRepeat.image,"CPU DOF repeat differs");++checks;
        RenderResult first;MetalExecutionActivity activity;
        for(unsigned repeat=0;repeat<3;++repeat){
            auto gpuInput=input;auto gpu=native_metal::RenderGeneralNativeMetal(gpuInput.header,gpuInput.addon,options,library.string());
            require(gpu.status==NativeMetalStatus::Completed&&gpu.result.success,"Generic DOF failed: "+gpu.reason);++checks;
            const auto& e=gpu.execution;
            require(e.validated&&e.engine=="general_formula"&&e.profile_kind==2&&e.arithmetic_bits==32&&
                !e.faults&&!e.nonfinite&&!e.work_exhausted,"DOF invalid GPU execution");++checks;
            require(e.dof_passes==passes&&e.dof_records==64*48*passes&&e.dof_positive_radius_sources>0&&
                e.dof_candidate_scan_checks>0&&e.dof_candidate_checks>0&&e.dof_contributing_pairs>0&&
                e.dof_changed_pixels>0&&e.dof_changed_pixels<=64*48&&e.dof_maximum_radius>0&&
                e.dof_dispatches>0,"DOF flag did not produce actual bounded blur");++checks;
            require(std::string(gpu.result.profile.stages[size_t(RenderProfileStage::DOF)].state)=="completed"&&
                gpu.result.profile.stages[size_t(RenderProfileStage::DOF)].seconds>0,"DOF timing/stage absent");++checks;
            if(reflection){require(gpu.reflections.rays&&gpu.reflections.changed_pixels&&gpu.reflections.ao_rays&&
                gpu.reflections.shadow_rays,"DOF dropped active reflections");++checks;}
            if(!repeat){first=std::move(gpu.result);activity=e;}
            else {require(first.image==gpu.result.image,"DOF GPU RGB repeat differs");++checks;
                require(activity.dof_candidate_scan_checks==e.dof_candidate_scan_checks&&activity.dof_candidate_checks==e.dof_candidate_checks&&
                    activity.dof_sort_work==e.dof_sort_work&&activity.dof_contributing_pairs==e.dof_contributing_pairs&&
                    activity.dof_changed_pixels==e.dof_changed_pixels,"DOF work repeat differs");++checks;}
        }
        auto disabledOptions=options;disabledOptions.no_dof=true;
        auto disabledInput=input;const auto disabled=native_metal::RenderGeneralNativeMetal(disabledInput.header,disabledInput.addon,disabledOptions,library.string());
        require(disabled.result.success&&!disabled.execution.dof_records&&!disabled.execution.dof_passes&&
            std::string(disabled.result.profile.stages[size_t(RenderProfileStage::DOF)].state)=="not_run","DOF override did not disable stage");++checks;
        const auto changed=different(first,disabled.result);
        require(changed==activity.dof_changed_pixels&&changed>0,"Reported DOF change differs from same-engine off control");++checks;
        require(first.silight.size()==disabled.result.silight.size()&&
            std::memcmp(first.silight.data(),disabled.result.silight.data(),first.silight.size()*sizeof(TsiLight5))==0,
            "DOF changed original geometry/surface");++checks;
        auto cpuOffInput=input;const auto cpuOff=Render(cpuOffInput.header,cpuOffInput.addon,disabledOptions);
        require(cpuOff.success&&different(cpu,cpuOff)>0,"CPU DOF control inactive");++checks;
        for(auto backend:{RenderBackend::Metal,RenderBackend::Auto}){
            auto mainInput=input;BackendOptions select;select.requested=backend;select.metal_library=library.string();
            auto main=RenderWithBackend(mainInput.header,mainInput.addon,options,select);
            require(main.render.success&&main.backend.effective==RenderBackend::Metal&&!main.backend.fallback&&
                main.backend.execution.engine=="general_formula"&&main.render.image==first.image,
                "Main renderer DOF fallback or image differs: "+main.render.error);++checks;
        }
        require(WritePNG((out/(name+"-cpu.png")).string(),cpu.image.data(),64,48,3)&&
            WritePNG((out/(name+"-metal.png")).string(),first.image.data(),64,48,3)&&
            WritePNG((out/(name+"-metal-off.png")).string(),disabled.result.image.data(),64,48,3),"Save DOF test image");
        if(index)report<<',';
        report<<"{\"name\":\""<<name<<"\",\"passes\":"<<passes<<",\"plain\":"<<(plain?"true":"false")
            <<",\"gpu_exact_repeats\":3,\"cpu_exact_repeats\":2,\"main_backends\":[\"metal\",\"auto\"]"
            <<",\"cpu_metal_differing_pixels\":"<<different(cpu,first)<<",\"dof_changed_pixels\":"<<changed
            <<",\"dof_records\":"<<activity.dof_records<<",\"candidate_scan_checks\":"<<activity.dof_candidate_scan_checks
            <<",\"candidate_checks\":"<<activity.dof_candidate_checks<<",\"contributing_pairs\":"<<activity.dof_contributing_pairs<<'}';
        report.flush();std::cout<<name<<" complete with actual DOF change "<<changed<<'\n';
    }
    report<<"],\"checks\":"<<checks<<",\"status\":\"PASS\"}\n";report.close();require(bool(report),"DOF report write failed");
    std::cout<<checks<<" complete DOF render checks passed\n";return 0;
}catch(const std::exception& e){std::cerr<<"Native DOF full render: "<<e.what()<<'\n';return 1;}}
