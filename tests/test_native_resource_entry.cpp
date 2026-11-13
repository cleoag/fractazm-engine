#include "render/metal_renderer.h"
#include "render/native_metal/general_renderer.h"
#include "core/numeric_precision.h"
#include "core/render_context.h"
#include "io/m3p_parser.h"
#include <cstring>
#include <filesystem>
#include <iostream>
#include <stdexcept>

using namespace mb3d;
static unsigned checks=0;
static void check(bool ok,const char* message){++checks;if(!ok)throw std::runtime_error(message);}
int main(int argc,char** argv){try{
    if(argc!=2)throw std::invalid_argument("usage: native_resource_entry SOURCE_ROOT");
    const auto root=std::filesystem::absolute(argv[1]);
    const auto missing=root/".native-resource-guard-no-library.metallib";
    check(!std::filesystem::exists(missing),"Negative library path exists");
    auto file=LoadM3PFile((root/"comparison/benchmark_scenes/effects-v1/integerpower8-ao-shadows.m3p").string());
    check(file.success,"Fixture load failed");file.header.bCalc3D=1;
    RenderOptions options;options.precision=PrecisionMode::Native;options.deterministic_mode=true;
    options.width_override=64;options.height_override=48;options.thread_count=6;
    options.resource_root=(root/"comparison/fixtures").string();
    const ScopedNumericPrecision precision(PrecisionMode::Compatible);
    const auto context=CaptureRenderContext();
    // An absent library is reached only after successful preparation, before
    // creating a device or dispatching. Unsupported must win for resource requests,
    // except diffuse, light and background maps: general Native camera color and
    // its reflection material sample them, and a missing file keeps the loader's
    // inactive state.
    for(auto render:{RenderNativeMetal,native_metal::RenderGeneralNativeMetal})for(int kind=0;kind<13;++kind) {
        const bool general=render==native_metal::RenderGeneralNativeMetal;
        auto h=file.header;auto addon=file.addon;
        if(kind==1){h.Light.bColorMap=55;h.Light.Lights[1].AdditionalByteEx=0;}
        if(kind==2){h.Light.bColorMap=97;h.Light.Lights[1].AdditionalByteEx=6;}
        if(kind==3)std::memset(h.Light.BGbmp,'x',sizeof(h.Light.BGbmp));
        if(kind>=4&&kind<10){h.Light.Lights[kind-4].Loption=2;h.Light.Lights[kind-4].LightMapNr=65534;}
        if(kind==10){h.Light.Lights[5].Loption=3;h.Light.Lights[5].LightMapNr=1633;}
        if(kind==11){h.Light.Lights[5].Loption=2;h.Light.Lights[5].LightMapNr=0;}
        if(kind==12){h.Light.bColorMap=55;h.Light.Lights[1].AdditionalByteEx=0;h.bCalcSRautomatic|=1;}
        const auto before=h;const auto beforeAddon=addon;
        const auto result=render(h,addon,options,missing.string());
        const bool required=!general&&((kind>0&&kind<10)||kind==12);
        check(result.status==(required?NativeMetalStatus::Unsupported:NativeMetalStatus::Unavailable),
            "Entry-point resource admission mismatch");
        if(required)check(result.reason.find("Unsupported Native requested ")==0,"Resource cause lost");
        check(!result.result.success&&result.result.image.empty()&&result.result.silight.empty(),"Rejected entry published image");
        check(!result.execution.validated&&result.device.empty(),"Preflight executed GPU");
        check(!std::memcmp(&h,&before,sizeof(h))&&!std::memcmp(&addon,&beforeAddon,sizeof(addon)),"Preflight changed caller inputs");
        check(CurrentPrecisionMode()==PrecisionMode::Compatible&&CaptureRenderContext()==context,"Preflight leaked context/precision");
    }
    std::cout<<"PASS native resource entry checks="<<checks<<" GPU dispatches=0\n";return 0;
}catch(const std::exception& e){std::cerr<<"FAIL "<<e.what()<<" checks="<<checks<<'\n';return 1;}}
