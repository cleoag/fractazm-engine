// Integration control to enable only after the staged GPU pass is connected.
#include "render/backend.h"
#include "render/native_metal/general_renderer.h"
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include <filesystem>
#include <cstring>
#include <fstream>
#include <iostream>
#include <stdexcept>
namespace fs=std::filesystem;using namespace mb3d;
unsigned checks=0;
void require(bool ok,const std::string& why){++checks;if(!ok)throw std::runtime_error(why);}
size_t different(const RenderResult& a,const RenderResult& b){
    require(a.width==b.width&&a.height==b.height&&a.image.size()==b.image.size(),"Image extent changed");
    size_t result=0;for(size_t i=0;i<a.image.size();i+=3)result+=a.image[i]!=b.image[i]||a.image[i+1]!=b.image[i+1]||a.image[i+2]!=b.image[i+2];return result;
}
int main(int argc,char** argv){try{
    require(argc==4,"usage: native-zbuffer-render SOURCE_ROOT LIBRARY NEW_OUTPUT");
    const fs::path root=fs::absolute(argv[1]),library=fs::absolute(argv[2]),out=fs::absolute(argv[3]);
    require(!fs::exists(out),"Preserve earlier render output");fs::create_directories(out);
    RenderOptions options;options.precision=PrecisionMode::Native;options.profile_timing=true;
    options.width_override=64;options.height_override=48;options.thread_count=6;options.deterministic_mode=true;options.resource_root=root.string();
    const char* paths[]={"comparison/scenes/test_06_tricorn.m3p","comparison/benchmark_scenes/effects-v1/integerpower8-reflections.m3p",
                        "comparison/scenes/extall_3Da_ABoxVSWavy.m3p","comparison/benchmark_scenes/effects-v1/integerpower8-dof.m3p"};
    const char* names[]={"tricorn-ao","integer-reflections","external-ao","integer-dof"};
    std::ofstream report(out/"report.json");report<<"{\"schema\":\"mb3d.native-zbuffer-full-render.v1\",\"cases\":[";
    for(unsigned index=0;index<4;++index){
        auto input=LoadM3PFile((root/paths[index]).string());require(input.success,input.error);input.header.byCalcNsOnZBufAuto|=1;
        auto cpuInput=input;auto cpu=Render(cpuInput.header,cpuInput.addon,options);require(cpu.success,"CPU active failed: "+cpu.error);
        cpuInput=input;auto cpuRepeat=Render(cpuInput.header,cpuInput.addon,options);require(cpuRepeat.success&&cpuRepeat.image==cpu.image,"CPU RGB repeat changed");
        RenderResult first;MetalExecutionActivity activity;
        for(unsigned repeat=0;repeat<3;++repeat){
            auto gpuInput=input;auto gpu=native_metal::RenderGeneralNativeMetal(gpuInput.header,gpuInput.addon,options,library.string());
            require(gpu.status==NativeMetalStatus::Completed&&gpu.result.success,"GPU Z normals failed: "+gpu.reason);
            const auto& e=gpu.execution;require(e.validated&&e.engine=="general_formula"&&e.profile_kind==2&&e.arithmetic_bits==32&&!e.faults&&!e.nonfinite&&!e.work_exhausted,"Invalid GPU execution");
            require(e.znormal_records==64*48&&e.znormal_foreground>0&&e.znormal_changed>0&&e.znormal_changed<=e.znormal_foreground&&e.znormal_neighbors>0&&e.znormal_dispatches==3,"Inactive/unbounded Z normal work");
            const auto& stage=gpu.result.profile.stages[size_t(RenderProfileStage::Normals)];require(std::string(stage.state)=="completed"&&stage.seconds>0,"Missing Z normal stage");
            if(index==1)require(gpu.reflections.rays&&gpu.reflections.changed_pixels&&gpu.reflections.ao_rays&&gpu.reflections.shadow_rays,"Z normals dropped active secondary effects");
            if(index==3)require(e.dof_records&&e.dof_contributing_pairs&&e.dof_changed_pixels,"Z normals dropped DOF");
            if(!repeat){first=std::move(gpu.result);activity=e;}
            else{
                require(first.image==gpu.result.image&&first.silight.size()==gpu.result.silight.size()&&
                        !std::memcmp(first.silight.data(),gpu.result.silight.data(),first.silight.size()*sizeof(TsiLight5)),"GPU RGB/surface repeat changed");
                require(activity.znormal_records==e.znormal_records&&activity.znormal_foreground==e.znormal_foreground&&activity.znormal_changed==e.znormal_changed&&
                        activity.znormal_neighbors==e.znormal_neighbors&&activity.znormal_dispatches==e.znormal_dispatches,"GPU Z work repeat changed");
            }
        }
        auto disabledInput=input;disabledInput.header.byCalcNsOnZBufAuto&=uint8_t(~1u);
        auto cpuOffInput=disabledInput;auto cpuOff=Render(cpuOffInput.header,cpuOffInput.addon,options);
        require(cpuOff.success&&different(cpu,cpuOff)>0,"CPU Z control inactive");
        auto off=native_metal::RenderGeneralNativeMetal(disabledInput.header,disabledInput.addon,options,library.string());
        require(off.result.success&&off.execution.engine=="general_formula"&&!off.execution.znormal_records&&!off.execution.znormal_dispatches&&
                std::string(off.result.profile.stages[size_t(RenderProfileStage::Normals)].state)=="not_run","GPU Z-off control failed");
        const auto changedImage=different(first,off.result);require(changedImage>0,"GPU Z control did not change image");
        require(first.silight.size()==off.result.silight.size(),"Z control surface size changed");
        size_t changedNormals=0;
        for(size_t i=0;i<first.silight.size();++i){const auto& a=first.silight[i];const auto& b=off.result.silight[i];
            changedNormals+=a.NormalX!=b.NormalX||a.NormalY!=b.NormalY||a.NormalZ!=b.NormalZ;
            require(a.RoughZposFine==b.RoughZposFine&&a.Zpos==b.Zpos&&a.SIgradient==b.SIgradient&&a.OTrap==b.OTrap,
                    "Z normals changed packed primary depth/color data");
            if(a.Zpos>=32768||a.SIgradient>=32768)require(a.NormalX==b.NormalX&&a.NormalY==b.NormalY&&a.NormalZ==b.NormalZ,"Z normals changed background/interior normals");
        }
        require(changedNormals==activity.znormal_changed,"Reported changed-normal count differs from same-engine off control");
        for(auto backend:{RenderBackend::Metal,RenderBackend::Auto}){
            auto mainInput=input;BackendOptions select;select.requested=backend;select.metal_library=library.string();
            const auto main=RenderWithBackend(mainInput.header,mainInput.addon,options,select);
            require(main.render.success&&main.backend.effective==RenderBackend::Metal&&!main.backend.fallback&&main.backend.execution.engine=="general_formula"&&main.render.image==first.image,
                    "Main Z backend fallback/image differs: "+main.render.error);
        }
        for(auto entry:{std::pair<const char*,const RenderResult*>{"cpu",&cpu},{"metal",&first},{"metal-off",&off.result}})
            require(WritePNG((out/(std::string(names[index])+"-"+entry.first+".png")).string(),entry.second->image.data(),64,48,3),"Failed to retain Z normal image");
        if(index)report<<',';
        report<<"{\"name\":\""<<names[index]<<"\",\"cpu_exact_repeats\":2,\"gpu_exact_repeats\":3,\"width\":64,\"height\":48"
              <<",\"changed_normals\":"<<changedNormals<<",\"effect_changed_pixels\":"<<changedImage<<",\"cpu_gpu_differing_pixels\":"<<different(cpu,first)
              <<",\"records\":"<<activity.znormal_records<<",\"foreground\":"<<activity.znormal_foreground<<",\"neighbors\":"<<activity.znormal_neighbors<<'}';
        report.flush();std::cout<<names[index]<<" complete\n";
    }
    report<<"],\"status\":\"PASS\",\"checks\":"<<checks<<"}\n";report.close();require(bool(report),"Report write failed");
    std::cout<<checks<<" checks passed\n";return 0;
}catch(const std::exception& error){std::cerr<<"FAIL: "<<error.what()<<'\n';return 1;}}
