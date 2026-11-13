#include "render/native_metal/general_renderer.h"
#include "io/m3p_parser.h"
#include "io/image_writer.h"
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <set>
#include <stdexcept>

namespace fs=std::filesystem;
using namespace mb3d;
void require(bool value,const std::string& message){if(!value)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
    require(argc==4,"usage: native-general-test SOURCE_ROOT LIBRARY NEW_OUTPUT_DIRECTORY");
    const fs::path root=fs::absolute(argv[1]),library=fs::absolute(argv[2]),output=fs::absolute(argv[3]);
    require(!fs::exists(output),"test output must be new");fs::create_directories(output);
    std::ofstream report(output/"report.json");
    report<<"{\"schema\":\"mb3d.native-general-render-test.v1\",\"qualification\":\"eight effect configurations including initial-inside ABoxVSWavy through generic geometry, shadows and reflections; not full formula coverage\",\"cases\":[";
    const char* names[]={"integerpower8","sphereifs","boxifs"};unsigned checks=0;
    for(unsigned c=0;c<8;++c){
        const bool reflections=c==6;
        const bool initialInside=c==7;
        const bool shadows=c>=3&&c<7;
        const std::string name=initialInside?"aboxvswavy-initial-inside":reflections?"integerpower8-reflections":std::string(names[c%3])+(shadows?"-shadows":"-shadows-off");
        const std::string file=reflections?name+".m3p":std::string(names[c%3])+"-ao-shadows"+(shadows?"":"-shadows-off")+".m3p";
        const fs::path source=initialInside?root/"comparison/scenes/extall_3Da_ABoxVSWavy.m3p":root/"comparison/benchmark_scenes/effects-v1"/file;
        auto loaded=LoadM3PFile(source.string());
        require(loaded.success,loaded.error);
        RenderOptions options;options.precision=PrecisionMode::Native;options.profile_timing=true;
        options.width_override=128;options.height_override=96;options.thread_count=6;
        options.deterministic_mode=true;options.resource_root=root.string();
        auto cpuInput=loaded;const auto cpu=Render(cpuInput.header,cpuInput.addon,options);
        require(cpu.success,"CPU control failed: "+cpu.error);++checks;
        require(WritePNG((output/(name+"-cpu.png")).string(),cpu.image.data(),cpu.width,cpu.height,3),"CPU PNG write failed");
        RenderResult first;MetalExecutionActivity firstExecution;ReflectionActivity firstReflection;double seconds[3]{};
        for(unsigned repeat=0;repeat<3;++repeat){
            auto input=loaded;
            auto gpu=native_metal::RenderGeneralNativeMetal(input.header,input.addon,options,library.string());
            require(gpu.status==NativeMetalStatus::Completed&&gpu.result.success,"GPU failed: "+gpu.reason);++checks;
            require(gpu.execution.engine=="general_formula"&&gpu.execution.validated&&gpu.execution.profile_kind==2&&gpu.execution.arithmetic_bits==32&&
                !gpu.execution.faults&&!gpu.execution.nonfinite&&!gpu.execution.work_exhausted,"Invalid GPU execution record");++checks;
            require(gpu.result.width==128&&gpu.result.height==96&&gpu.execution.primary_records==128*96&&
                gpu.execution.surface_records==2*128*96&&gpu.execution.color_records==128*96,"Incomplete GPU frame");++checks;
            require(gpu.execution.distance_calls>128*96&&gpu.execution.formula_work>=gpu.execution.distance_calls,"No real formula work");++checks;
            require(std::string(gpu.result.profile.stages[size_t(RenderProfileStage::AO)].state)=="completed"&&
                std::string(gpu.result.profile.stages[size_t(RenderProfileStage::Shadows)].state)==(shadows?"completed":"not_run"),"Wrong effect activity");++checks;
            require(gpu.execution.shadow_records==(shadows?128*96:0),"Wrong shadow record count");++checks;
            if(shadows){require(gpu.execution.shadow_rays&&gpu.execution.shadow_steps&&gpu.execution.shadow_changed&&
                gpu.execution.shadow_hits+gpu.execution.shadow_misses==gpu.execution.shadow_rays,"No actual shadow work or invalid outcomes");++checks;}
            require(gpu.execution.reflection_records==(reflections?128*96:0),"Wrong reflection record count");++checks;
            if(reflections){require(gpu.reflections.enabled&&gpu.reflections.rays&&gpu.reflections.hits&&gpu.reflections.changed_pixels&&
                gpu.reflections.ao_rays&&gpu.reflections.ao_steps&&gpu.reflections.shadow_rays&&gpu.reflections.shadow_steps&&
                gpu.reflections.ray_steps&&gpu.reflections.max_bounces==2&&
                !gpu.reflections.faults,"No actual bounded secondary rays/AO/shadows");++checks;}
            seconds[repeat]=gpu.result.profile.total_seconds;
            if(repeat==0){first=std::move(gpu.result);firstExecution=gpu.execution;firstReflection=gpu.reflections;}
            else {require(first.image==gpu.result.image,"Generic GPU RGB repeat differs");++checks;
                require(firstExecution.distance_calls==gpu.execution.distance_calls&&firstExecution.formula_work==gpu.execution.formula_work&&
                    firstExecution.shadow_rays==gpu.execution.shadow_rays&&firstExecution.shadow_steps==gpu.execution.shadow_steps&&
                    firstExecution.shadow_hits==gpu.execution.shadow_hits&&firstExecution.shadow_misses==gpu.execution.shadow_misses&&
                    firstExecution.shadow_changed==gpu.execution.shadow_changed,"Generic GPU work repeat differs");++checks;
                require(firstReflection.active_pixels==gpu.reflections.active_pixels&&firstReflection.rays==gpu.reflections.rays&&
                    firstReflection.hits==gpu.reflections.hits&&firstReflection.misses==gpu.reflections.misses&&
                    firstReflection.changed_pixels==gpu.reflections.changed_pixels&&firstReflection.ray_steps==gpu.reflections.ray_steps&&
                    firstReflection.ao_rays==gpu.reflections.ao_rays&&firstReflection.ao_steps==gpu.reflections.ao_steps&&
                    firstReflection.shadow_rays==gpu.reflections.shadow_rays&&firstReflection.shadow_steps==gpu.reflections.shadow_steps&&
                    firstReflection.max_bounces==gpu.reflections.max_bounces,"Generic secondary GPU work repeat differs");++checks;}
        }
        std::set<uint32_t> colors;size_t foreground=0,background=0,differing=0,interiorCount=0,cpuInteriorCount=0;
        for(size_t i=0;i<first.silight.size();++i){
            if(first.silight[i].Zpos>32767)++background;else ++foreground;
            const auto* p=&first.image[i*3];colors.insert(p[0]|(uint32_t(p[1])<<8)|(uint32_t(p[2])<<16));
            differing+=p[0]!=cpu.image[i*3]||p[1]!=cpu.image[i*3+1]||p[2]!=cpu.image[i*3+2];
            interiorCount+=first.silight[i].SIgradient>32767;
            cpuInteriorCount+=cpu.silight[i].SIgradient>32767;
        }
        require(foreground&&background&&colors.size()>32,"Generic frame has no meaningful foreground/background/color coverage");++checks;
        if(initialInside){require(interiorCount&&cpuInteriorCount,"Initial-inside control did not reach both CPU and GPU interior paths");++checks;}
        require(WritePNG((output/(name+"-metal.png")).string(),first.image.data(),first.width,first.height,3),"Metal PNG write failed");
        if(c)report<<',';
        report<<"{\"name\":\""<<name<<"\",\"exact_rgb_repeats\":3,\"foreground\":"<<foreground<<",\"background\":"<<background
            <<",\"unique_rgb\":"<<colors.size()<<",\"cpu_metal_differing_pixels\":"<<differing
            <<",\"interior_pixels\":"<<interiorCount<<",\"cpu_interior_pixels\":"<<cpuInteriorCount
            <<",\"distance_calls\":"<<firstExecution.distance_calls<<",\"formula_work\":"<<firstExecution.formula_work
            <<",\"shadow_rays\":"<<firstExecution.shadow_rays<<",\"shadow_hits\":"<<firstExecution.shadow_hits
            <<",\"shadow_misses\":"<<firstExecution.shadow_misses<<",\"shadow_steps\":"<<firstExecution.shadow_steps
            <<",\"shadow_changed\":"<<firstExecution.shadow_changed
            <<",\"reflection_rays\":"<<firstReflection.rays<<",\"reflection_hits\":"<<firstReflection.hits
            <<",\"reflection_misses\":"<<firstReflection.misses<<",\"reflection_changed\":"<<firstReflection.changed_pixels
            <<",\"reflection_steps\":"<<firstReflection.ray_steps<<",\"reflection_ao_rays\":"<<firstReflection.ao_rays
            <<",\"reflection_ao_steps\":"<<firstReflection.ao_steps<<",\"reflection_shadow_rays\":"<<firstReflection.shadow_rays
            <<",\"reflection_shadow_steps\":"<<firstReflection.shadow_steps
            <<",\"full_render_seconds\":["<<seconds[0]<<','<<seconds[1]<<','<<seconds[2]<<"]}";
        // The override must disable the image-space AO pass without dropping
        // formula geometry. This control is distinct from the active scene.
        auto offInput=loaded;auto offOptions=options;offOptions.no_ao=true;
        auto off=native_metal::RenderGeneralNativeMetal(offInput.header,offInput.addon,offOptions,library.string());
        require(off.result.success&&std::string(off.result.profile.stages[size_t(RenderProfileStage::AO)].state)=="not_run"&&
            first.image!=off.result.image,"Generic AO control inactive or unimplemented");++checks;
        if(reflections){auto refInput=loaded;auto refOptions=options;refOptions.no_reflections=true;
            auto refOff=native_metal::RenderGeneralNativeMetal(refInput.header,refInput.addon,refOptions,library.string());
            require(refOff.result.success&&!refOff.reflections.enabled&&!refOff.execution.reflection_records&&
                first.image!=refOff.result.image,"Generic reflection off control inactive or unimplemented");++checks;}
        report.flush();
    }
    report<<"],\"checks\":"<<checks<<",\"status\":\"PASS\"}\n";
    report.close();require(bool(report),"report write failed");
    std::cout<<checks<<" generic GPU integration checks passed; 3 exact RGB repeats per scene\n";return 0;
}catch(const std::exception& e){std::cerr<<"native general integration: "<<e.what()<<'\n';return 1;}}
