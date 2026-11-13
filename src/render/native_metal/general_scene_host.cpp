#include "general_scene_host.h"
#include "general_normals_host.h"
#include "native_inside_host.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/formula_registry.h"
#include "core/math3d.h"
#include "core/numeric_precision.h"
#include "core/ray_marcher.h"
#include "core/reflection.h"
#include "core/render_context.h"
#include <bit>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <stdexcept>

namespace mb3d::native_metal {
namespace {
uint64_t bits(double value) { return std::bit_cast<uint64_t>(value); }
uint32_t bits(float value) { return std::bit_cast<uint32_t>(value); }
void finite32(double value, const char* label) {
    if (!std::isfinite(value) || !std::isfinite(static_cast<float>(value)))
        throw std::invalid_argument(std::string("Native Metal scene FP32 range: ") + label);
}
ExactTrig trig(double angle) {
    finite32(angle,"camera angle");
    return {bits(std::sin(angle)),bits(std::cos(angle)),0,0,0,0,{0,0}};
}
float single(double value, const char* label) {
    finite32(value,label); return static_cast<float>(value);
}
// CalcVolLightMap's geometry from the render header and parameters, with the
// LVals Render builds before it. A disabled light keeps version 1 with no
// faces: CPU computes no map and its sub-steps add nothing.
mb3d_native::NativeVolumeLight PrepareVolume(const TMandHeader10& header, const TMCTparameter& m) {
    mb3d_native::NativeVolumeLight v{}; v.version=mb3d_native::kNativeVolumeLightVersion;
    auto h=header;
    TLightVals lights{}; TLValigned aligned{}; lights.PLValigned=&aligned;
    MakeLightValsFromHeaderLight(&h,&lights,1.0,0);
    CompleteLightValsSetup(&h,&lights);
    TVolumeLightMap map; TVolumeLightMapSetup setup;
    if (!PrepareVolLightMap(&h,&lights,&m,map,setup)) return v;
    const auto& p=setup.params;
    if (map.CubeSize<3 || map.CubeSize>16384 || map.SideCount!=(map.IsPosLight?6:1) ||
        (p.bInsideRendering!=0 && p.bInsideRendering!=1 && p.bInsideRendering!=-1))
        throw std::invalid_argument("Native volumetric light map geometry is not implemented");
    v.positional=map.IsPosLight; v.side_count=map.SideCount; v.cube_size=map.CubeSize; v.half_size=map.HalfSize;
    v.inside=p.bInsideRendering; v.in_and_out=p.bInAndOutside!=0;
    v.stretch=map.StretchSide1; v.size_factor=map.SizeFactor; v.csize=map.CsizeS; v.hsize=map.HSizeS;
    v.min_distance=map.MinDistance; v.max_distance=setup.maxDist; v.cell_step=setup.sgl;
    v.step_width=single(m.StepWidth,"volumetric step width");
    v.vl_mul=single(m.VLmul,"volumetric multiplier"); v.vl_step_mul=single(m.VLstepmul,"volumetric step multiplier");
    for (unsigned i=0;i<3;++i) {
        v.light[i]=single(map.LightPos[i],"volumetric light position");
        v.origin[i]=single(p.Ystart[i],"volumetric map origin");
        v.forward[i]=single(p.Vgrads[2][i],"volumetric map direction");
        for (unsigned j=0;j<3;++j) {
            v.rotation[i][j]=single(map.RotMatrix[i][j],"volumetric rotation");
            v.inverse[i][j]=single(setup.invMs[i][j],"volumetric inverse rotation");
        }
    }
    for (double value : {double(v.stretch),double(v.size_factor),double(v.min_distance),
            double(v.max_distance),double(v.cell_step)}) finite32(value,"volumetric map scalar");
    if (v.step_width<=0 || v.vl_step_mul<=0 || v.stretch<=0 || v.max_distance<=0)
        throw std::invalid_argument("Native volumetric light map scalar is invalid");
    return v;
}
bool VolumeFog(const TMandHeader10& header, const TMCTparameter& m) {
    return (header.bVolLightNr&7) && m.DFogOnIt==65535;
}
}
PreparedGeneralScene PrepareGeneralSceneWithMaps(const TMandHeader10& header,
    const THeaderCustomAddon& addon, unsigned width, unsigned height,
    const std::string& root, const PreparedFormulaProgram* snapshot, unsigned max_steps, Lattice lattice) {
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    if (!width || !height || width>4096 || height>4096 || !max_steps || max_steps>65536)
        throw std::invalid_argument("Native Metal general dimensions/work bounds");
    // Admit features as their actual generic GPU passes land. In particular a
    // different formula is not permission to run the old bulb shadow function.
    if (!header.bCalc3D ||
        (header.MCoptions&1))
        throw std::invalid_argument("Native Metal general scene pass is not implemented");
    for (const auto& formula:addon.Formulas) {
        if (IsInternalFormula(formula.iFnr)) continue;
        const auto* first=reinterpret_cast<const char*>(formula.CustomFname);
        std::string name(first,std::find(first,first+sizeof(formula.CustomFname),'\0'));
        std::transform(name.begin(),name.end(),name.begin(),[](unsigned char c){return char(std::tolower(c));});
        if (name.find("jit")!=std::string::npos)
            throw std::invalid_argument("Deferred formula input excluded");
    }
    PreparedGeneralScene result;
    auto& scene=result.base;
    scene.precision=PrecisionMode::Native;
    scene.scene_id="native_general";
    scene.original_shadow_bits=header.bCalculateHardShadow;
    scene.host=std::make_shared<metal_exact::SceneHostState>();
    auto& host=*scene.host;
    host.context=std::make_shared<RenderContext>(root);
    const auto caller=CaptureRenderContext();
    host.context->maps.sequences=caller->maps.sequences;
    host.context->maps.frame_number=caller->maps.frame_number;
    host.context->precision=PrecisionMode::Native;
    ScopedRenderContext binding(host.context);
    host.file.header=header; host.file.addon=addon; host.file.success=true;
    host.file.header.Width=static_cast<int>(width);
    host.file.header.Height=static_cast<int>(height);
    host.mct=GetMCTparasFromHeader(host.file.header,&host.file.addon,0);
    const auto& m=host.mct;
    if (!m.bMCTisValid ||
        (m.DFogOnIt>255 && !VolumeFog(host.file.header,m)) || m.iMinIt<1 || m.iMinIt>m.iMaxIt ||
        !mb3d_native::valid_general_primary_limits(
            static_cast<unsigned>(m.iDEAddSteps),static_cast<unsigned>(m.iMaxIt)))
        throw std::invalid_argument("Native Metal general primary feature is not implemented");
    (void)PrepareNativeInside(m);
    result.camera=PrepareNativeCamera(host.file.header,m,lattice);
    width=result.camera.device.width;height=result.camera.device.height;
    result.formulas=PrepareFormulaProgramWithMaps(m,host.file.addon,snapshot);
    (void)PrepareGeneralNormals(m,result.formulas.device);
    auto& d=scene.device;
    d.version=4; d.width=width; d.height=height; d.max_steps=max_steps;
    d.formula_kind=kGeneralFormulaKind; d.capability_id=kGeneralCapability;
    d.max_iterations=m.iMaxIt; d.slot_count=result.formulas.device.slot_count;
    d.min_iterations=m.iMinIt; d.refinement_steps=m.iDEAddSteps;
    d.fog_iteration=static_cast<uint32_t>(m.DFogOnIt);
    if (VolumeFog(host.file.header,m)) result.volume=PrepareVolume(host.file.header,m);
    // RMdoColorOnIt 1 only copies C to xyz, which ColorOption 0/1 never read.
    if (m.ColorOnIt>1) {
        const auto& program=result.formulas.device;
        if (m.DEoption==20 || program.driver>=mb3d_native::HybridDifs || program.formula_type)
            throw std::invalid_argument("Native general ColorOnIt with dIFS or a DE combination is not implemented");
        if (m.ColorOnIt-1>m.iMaxIt+1)
            throw std::invalid_argument("Native general ColorOnIt exceeds the iteration maximum");
        // TIteration3Dext.RStop is Single: Single(RStop*RStop*64) as in RMdoColorOnIt.
        const float stop=static_cast<float>(m.dRStop*m.dRStop*64.0f);
        if (!std::isfinite(stop) || stop<=0)
            throw std::invalid_argument("Native general ColorOnIt radius stop is invalid");
        d.color_on_it=static_cast<uint32_t>(m.ColorOnIt-1); d.color_radius_stop=bits(stop);
    }
    // Shared image-space stages consume this descriptor; only generic geometry
    // consumes the formula program, which is a separate constant MTLBuffer.
    for (unsigned i=0;i<3;++i) {
        finite32(m.Ystart[i],"camera origin"); d.y_start[i]=bits(m.Ystart[i]);
        for (unsigned j=0;j<3;++j) {
            finite32(m.Vgrads[i][j],"camera matrix");
            d.vgrads[i*3+j]=bits(m.Vgrads[i][j]);
        }
    }
    for (double value : {m.Zend,double(m.DEstop),double(m.mctDEstopFactor),
            double(m.sZstepDiv),double(m.mctMH04ZSD),double(m.dDEscale),
            double(m.sStepWm103),double(m.msDEsub)}) finite32(value,"march scalar");
    if (m.Zend<=0 || m.DEstop<=0 || m.mctDEstopFactor<0 || m.sZstepDiv<=0 ||
        m.mctMH04ZSD<=0 || m.dDEscale<=0 || m.sStepWm103<=0)
        throw std::invalid_argument("Native Metal general march scalar is invalid");
    d.max_ray_length=bits(m.Zend); d.de_stop=bits(m.DEstop);
    d.de_stop_factor=bits(m.mctDEstopFactor); d.z_step_div=bits(m.sZstepDiv);
    d.step_limiter=bits(m.mctMH04ZSD); d.de_scale=bits(m.dDEscale);
    d.step_width_103=bits(m.sStepWm103); d.de_sub=bits(m.msDEsub);
    scene.columns.reserve(width); scene.rows.reserve(height);
    const int stride=int(result.camera.device.stride_extra)+1;
    for (unsigned x=0;x<width;++x)
        scene.columns.push_back(trig((double(m.FOVXoff)-double(int(x)*stride+result.camera.device.left+1))*double(m.FOVXmul)));
    for (unsigned y=0;y<height;++y)
        scene.rows.push_back(trig(CalcCameraRowAngle(int(y)*stride+result.camera.device.top,m.iMandHeight,m.FOVy)));
    return result;
}
PreparedGeneralScene PrepareGeneralScene(const TMandHeader10& header,
    const THeaderCustomAddon& addon, unsigned width, unsigned height,
    const std::string& root, unsigned max_steps, Lattice lattice) {
    return PrepareGeneralSceneWithMaps(header,addon,width,height,root,nullptr,max_steps,lattice);
}
mb3d_native::NativeSurfaceParams PrepareGeneralSurface(const PreparedGeneralScene& scene) {
    if (!scene.base.host) throw std::invalid_argument("Missing Native general scene owner");
    const auto& m=scene.base.host->mct;
    if (m.ColorOption>1 || (m.DFogOnIt>255 && !VolumeFog(scene.base.host->file.header,m)) ||
        (m.ColorOption && (m.DEoption==20 || scene.formulas.device.driver>=mb3d_native::HybridDifs)))
        throw std::invalid_argument("Native general surface mode is not implemented");
    mb3d_native::NativeSurfaceParams p{}; p.version=mb3d_native::kNativeSurfaceVersion;
    p.color_option=m.ColorOption;
    if (p.color_option) {
        finite32(m.mctColorMul,"orbit color multiplier");
        p.orbit_color_multiplier=float(m.mctColorMul);
    }
    for (double v : {m.Zcorr,m.ZcMul,double(m.dColPlus),double(m.mctsM),
                    double(m.mctColVarDEstopMul),m.StepWidth}) finite32(v,"surface scalar");
    p.z_corr=bits(m.Zcorr); p.z_c_mul=bits(m.ZcMul);
    p.color_plus=bits(double(m.dColPlus)); p.color_multiplier=bits(double(m.mctsM));
    p.color_stop_multiplier=bits(double(m.mctColVarDEstopMul));
    p.step_width=bits(m.StepWidth);
    return p;
}
} // namespace mb3d::native_metal
