#include "shadow_host.h"
#include "render/native_metal/general_geometry_abi.h"
#include "scene_host_internal.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include "core/packed_math.h"
#include "core/renderer.h"
#include "core/shadow.h"
#include <bit>
#include <cstring>
#include <stdexcept>
namespace mb3d::metal_exact {
static uint64_t shadow_bits(double value) {return std::bit_cast<uint64_t>(value);}
ExactSurface ExportSurface(const TsiLight5& src) {
    ExactSurface out{};
    out.normal[0]=src.NormalX;out.normal[1]=src.NormalY;out.normal[2]=src.NormalZ;
    std::memcpy(&out.depth_packed,&src.RoughZposFine,4);
    out.si_gradient=src.SIgradient;out.o_trap=src.OTrap;out.shadow=src.Shadow;out.amb_shadow=src.AmbShadow;
    return out;
}
TsiLight5 ImportSurface(const ExactSurface& src) {
    if(src.fault||src.reserved[0]||src.reserved[1]||src.reserved[2]||src.si_gradient>65535||src.o_trap>65535||src.shadow>65535||src.amb_shadow>65535)
        throw std::invalid_argument("Invalid exact surface buffer");
    TsiLight5 out{};
    for(int i=0;i<3;++i)if(src.normal[i]<-32768||src.normal[i]>32767)throw std::invalid_argument("Invalid exact surface normal");
    out.NormalX=src.normal[0];out.NormalY=src.normal[1];out.NormalZ=src.normal[2];
    std::memcpy(&out.RoughZposFine,&src.depth_packed,4);
    out.SIgradient=src.si_gradient;out.OTrap=src.o_trap;out.Shadow=src.shadow;out.AmbShadow=src.amb_shadow;
    return out;
}
ExactSurfaceParams PrepareSurface(const PreparedExactScene& scene) {
    if(!scene.host)throw std::invalid_argument("Missing scene owner");
    const auto& m=scene.host->mct;
    if(m.iSmNormals||m.ColorOnIt||m.ColorOption||
       (scene.device.formula_kind!=3&&m.mctColVarDEstopMul!=0)||
       m.DFogOnIt||m.bInsideRendering||m.bInAndOutside)
        throw std::invalid_argument("Unsupported exact surface packing capability");
    ExactSurfaceParams p{};p.version=2;
    p.z_corr=shadow_bits(m.Zcorr);p.z_c_mul=shadow_bits(m.ZcMul);
    p.color_plus=shadow_bits(m.dColPlus);p.color_multiplier=shadow_bits(m.mctsM);
    p.color_stop_multiplier=shadow_bits(m.mctColVarDEstopMul);
    p.step_width=shadow_bits(m.StepWidth);
    return p;
}
struct ShadowHost {
    TMandHeader10 header{};TMCTparameter mct{};TLightVals light{};TLValigned aligned{};
};
static void PrepareShadowHost(const PreparedExactScene& scene,bool diagnostic,ShadowHost& host,bool native_soft=false) {
    host.header=scene.host->file.header;
    if(diagnostic)host.header.bCalculateHardShadow|=1;
    const auto base=host.header.bHScalculated;
    if((host.header.bCalc1HSsoft&1)&&!native_soft)throw std::invalid_argument("Soft shadow unsupported by exact capability");
    host.header.bHScalculated=(base&0xfd)|host.header.bCalculateHardShadow;
    if(host.header.bHScalculated&2)
        for(int i=0;i<6;++i)if((host.header.bCalculateHardShadow>>(i+2))&1)host.header.Light.Lights[i].LFunction&=0xcf;
    auto addon=scene.host->file.addon;
    host.mct=GetMCTparasFromHeader(host.header,&addon,1);
    host.light.PLValigned=&host.aligned;
    auto original=scene.host->file.header;
    MakeLightValsFromHeaderLight(&original,&host.light,1,0);
    CompleteLightValsSetup(&original,&host.light);
    CalcHSVecsFromLights(&host.light,&host.mct);
    host.mct.calcHardShadow=host.header.bCalculateHardShadow;
    host.mct.sHSmaxLengthMultiplier=host.header.HSmaxLengthMultiplier>0?host.header.HSmaxLengthMultiplier:1;
}
ExactShadowParams PrepareShadow(const PreparedExactScene& scene,bool diagnostic,unsigned max_steps,bool camera_inside,bool native_soft) {
    if(!scene.host||!max_steps||max_steps>65536)throw std::invalid_argument("Invalid exact shadow preparation");
    ScopedRenderContext binding(scene.host->context);
    const ScopedNumericPrecision precision(scene.precision);
    if(native_soft && (scene.precision!=PrecisionMode::Native ||
        scene.device.formula_kind!=mb3d_native::kGeneralFormulaKind ||
        scene.device.capability_id!=mb3d_native::kGeneralCapability || !camera_inside ||
        !(scene.host->file.header.bCalc1HSsoft&1)))
        throw std::invalid_argument("Invalid Native soft-shadow profile");
    ShadowHost host;PrepareShadowHost(scene,diagnostic,host,native_soft);
    // Generic Native primary already validates and packs the selected normal
    // algorithm. Shadow preparation consumes those normals without recomputing
    // them. Keep the compatible and specialized capability restrictions intact.
    const bool generalFormula=scene.precision==PrecisionMode::Native&&
        scene.device.formula_kind==mb3d_native::kGeneralFormulaKind&&
        scene.device.capability_id==mb3d_native::kGeneralCapability;
    // Only the explicit camera/inside GPU entry point consumes these policies.
    if((camera_inside&&!generalFormula)||!host.mct.bMCTisValid||
       (!camera_inside&&(host.mct.iCutOptions||host.mct.bInsideRendering||host.mct.bInAndOutside||host.mct.MCTCameraOptic))||
       (host.mct.iSmNormals&&!generalFormula))
        throw std::invalid_argument("Unsupported exact shadow capability");
    ExactShadowParams p{};p.version=1;p.enabled=(host.mct.calcHardShadow&1)!=0;
    p.light_mask=(host.mct.calcHardShadow>>2)&63;p.max_steps=max_steps;
    p.z_corr=shadow_bits(host.mct.Zcorr);p.z_c_mul=shadow_bits(host.mct.ZcMul);
    p.fov_y=shadow_bits(host.mct.FOVy);p.length_multiplier=shadow_bits(host.mct.sHSmaxLengthMultiplier);
    p.max_iterations_result=host.mct.iMaxIt+(host.mct.DEoption==20?1:0);
    for(int light=0;light<6;++light){const int sorted=host.light.SortTab[light];
        // A disabled descriptor marches nothing; its mask only mirrors the header.
        if(p.enabled&&(p.light_mask&(1u<<light))&&(host.light.iLightPos[sorted]&1)){
            // Native general shadows march position lights per pixel
            // (calcHShard, CalcHShardPoint, calcHSsoft, CalcHSsoftPoint).
            if(!camera_inside||!generalFormula)
                throw std::invalid_argument("Position light unsupported by exact shadow capability");
            p.position_mask|=1u<<light;
            TVec3D position;AddSVecToPackedVec3D(host.light.PLValigned->LN[sorted],&host.mct.Xmit,position);
            for(int i=0;i<3;++i)p.light_positions[light*3+i]=shadow_bits(position[i]);
            const float reach=host.light.sLmaxL[sorted]*host.mct.sHSmaxLengthMultiplier;
            p.light_reach[light]=std::bit_cast<uint32_t>(reach);
            p.step_width=shadow_bits(host.mct.StepWidth);
            continue;
        }
        for(int i=0;i<3;++i)p.light_vectors[light*3+i]=shadow_bits(host.mct.HSvecs[sorted][i]);
    }
    return p;
}
unsigned NativeSoftShadowNumerator(const PreparedExactScene& scene) {
    if(!scene.host||!(scene.host->file.header.bCalc1HSsoft&1))
        throw std::invalid_argument("Invalid Native soft-shadow profile");
    ScopedRenderContext binding(scene.host->context);
    const ScopedNumericPrecision precision(scene.precision);
    ShadowHost host;PrepareShadowHost(scene,false,host,true);
    // calcHSsoft: the last enabled shadow light (iHSnr) owns the intensity.
    int light=0;
    for(int i=0;i<6;++i)if(host.mct.calcHardShadow&(4<<i))light=host.light.SortTab[i];
    const int position=host.light.iLightPos[light];
    return (position&1)?((position&6)==2?70u:40u):80u;
}
std::vector<ExactSurface> ReferenceSurface(const PreparedExactScene& scene,unsigned threads) {
    if(!scene.host)throw std::invalid_argument("Missing scene owner");
    auto header=scene.host->file.header;auto addon=scene.host->file.addon;
    RenderOptions options;options.precision=PrecisionMode::Compatible;options.thread_count=threads;options.no_shadows=true;options.no_ao=true;
    auto result=Render(header,addon,options);
    if(!result.success)throw std::runtime_error("Reference surface: "+result.error);
    std::vector<ExactSurface> output;output.reserve(result.silight.size());
    for(const auto& pixel:result.silight)output.push_back(ExportSurface(pixel));
    return output;
}
std::vector<ExactSurface> ReferenceShadow(const PreparedExactScene& scene,const std::vector<ExactSurface>& surface,bool diagnostic) {
    if(!scene.host||surface.size()!=size_t(scene.device.width)*scene.device.height)throw std::invalid_argument("Invalid reference shadow surface");
    ScopedRenderContext binding(scene.host->context);
    const ScopedNumericPrecision precision(PrecisionMode::Compatible);
    ShadowHost host;PrepareShadowHost(scene,diagnostic,host);
    if(!(host.mct.calcHardShadow&1))return surface;
    std::vector<TsiLight5> buffer;buffer.reserve(surface.size());
    for(const auto& p:surface)buffer.push_back(ImportSurface(p));
    CalcHardShadows(buffer.data(),scene.device.width,scene.device.height,scene.device.width,&host.mct,&host.light);
    std::vector<ExactSurface> output;output.reserve(buffer.size());
    for(const auto& p:buffer)output.push_back(ExportSurface(p));return output;
}
} // namespace mb3d::metal_exact
