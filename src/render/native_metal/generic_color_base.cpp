#include "generic_color_base.h"
#include "resource_requirements.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <future>
#include <stdexcept>
namespace mb3d::native_metal {
using namespace metal_exact;
using namespace mb3d_metal_exact;
namespace {
uint32_t bits(float x) { return std::bit_cast<uint32_t>(x); }
uint64_t bits(double x) { return std::bit_cast<uint64_t>(x); }
struct PaintSetup {
    TMandHeader10 header{};
    TLightVals lights{};
    TLValigned aligned{};
    TPaintParameter paint{};
    TPaintLightVals view{};
    TMatrix3 gradients{};
    TSVec start{}, y_add{}, x_add{};
    float aspect{};
    explicit PaintSetup(const PreparedExactScene& scene,bool soft) {
        header=scene.host->file.header;
        // Also protect direct host preparation callers outside PreparedRenderPlan.
        ValidateNativeLightingRequirements(NativeLightingRequirements(header),true);
        lights.PLValigned=&aligned;
        MakeLightValsFromHeaderLight(&header,&lights,1.0,0);
        CompleteLightValsSetup(&header,&lights);
        // Reject requested and previously calculated soft shadows before the
        // completed-hard-shadow bookkeeping below replaces their signed masks.
        if(!soft && (header.bCalc1HSsoft&1) && (header.bCalculateHardShadow&1) &&
           (header.bCalculateHardShadow&0xfc))
            throw std::runtime_error("Unsupported Native general soft-shadow color capability");
        for(int i=0;i<6;++i) if(!soft && lights.iLightOption[i]==0 && lights.iHSmask[i]<0)
            throw std::runtime_error("Unsupported Native general soft-shadow color capability");
        // Production Render applies this state after completing the hard-shadow
        // pass, even when every light ray happened to remain unobstructed.
        const uint8_t completed=(header.bHScalculated&(soft?1:0xfd))|header.bCalculateHardShadow;
        if(header.bCalculateHardShadow&1) for(int x=0;x<6;++x) {
            const int y=lights.SortTab[x];
            lights.iHScalced[y]=(completed>>(x+2))&1;
            lights.iHSmask[y]=0x400<<x;
            if(soft && lights.iHScalced[y])lights.iHSmask[y]=-1;
            if((completed&2)&&((header.bCalculateHardShadow>>(x+2))&1))
                lights.iLightFuncDiff[y]=0;
        }
        // A dormant saved soft-shadow option has no shading contribution.
        // Diffuse color maps (iColOnOT 2..9, bYCcomb) and a loaded background
        // map shade on the GPU through NativeMaterialMaps; iExModes through
        // kNativeTotalLight2Flag.
        if(lights.iDfunc<0 || lights.iDfunc>3 || lights.iColOnOT<0 || lights.iColOnOT>9 ||
           ((lights.iColOnOT & ~1)!=0)!=(lights.DiffColLightMap!=nullptr) ||
           (lights.bDFogOptions & ~3) || bool(lights.bBackBMP)!=(lights.BGLightMap!=nullptr) ||
           (lights.BGsmallLM!=nullptr)!=(lights.BGLightMap!=nullptr))
            throw std::runtime_error("Unsupported Native general Metal color capability");
        // A failed background load leaves bUseSmallBGpicForAmb with a rotated
        // normal ambient (no map); image coordinates sample by the screen position.
        if(lights.bBackBMP && lights.bDirectImageCoord)
            throw std::runtime_error("Unsupported Native background map image coordinates");
        // Positional lights and visible light shapes (iLightPos) shade on the
        // GPU through NativeLightGeometry; loaded light maps (option 2) through
        // NativeMaterialMaps.
        for(int i=0;i<6;++i) if(lights.iLightOption[i]<0||lights.iLightOption[i]>2||
            (lights.iLightOption[i]==2)!=(lights.LLightMaps[i]!=nullptr)||
            (lights.iLightOption[i]==0&&(lights.iLightPos[i]&~15)))
            throw std::runtime_error("Unsupported Native general Metal light capability");
        paint.ppWidth=header.Width;paint.ppHeight=header.Height;paint.ppYinc=1;
        paint.PLVals=&lights;
        CalcStepWidth(&header);paint.StepWidth=header.dStepWidth;
        TMatrix3 source{};std::memcpy(source,header.hVGrads,sizeof(source));
        NormaliseMatrixTo(gradients,header.dStepWidth,source);paint.pVgrads=&gradients;
        paint.sFOVy=header.bPlanarOptic==2?float(Pi):float(header.dFOVy*Pid180);
        paint.ppPlanarOptic=header.bPlanarOptic&3;
        CalcPPZvals(header,paint.Zcorr,paint.ZcMul,paint.ZZstmitDif);
        const double fov=std::min(1.5,std::max(0.001,double(paint.sFOVy)*0.5));
        paint.ppPlOpticZ=float(std::cos(fov)*fov/std::sin(fov));
        paint.ppXOff=CalcXoff(&header);
        GetPaintTileSizes(&header,paint.ppPaintWidth,paint.ppPaintHeight,paint.ppXplus,paint.ppYplus);
        GetStartSPosAndAddVecs(view,paint,start,y_add,x_add);
        aspect=header.bPlanarOptic==2?2.f:float(header.Width)/float(header.Height);
    }
};
// BuildDiffCosTabs is private production preparation, reproduced here as a
// fixed scene-independent table. Per-pixel spline interpolation remains GPU.
void BuildDiffuseTable(ExactColorScene& out) {
    float table[8][128]{};
    for(int i=0;i<128;++i) {
        double d=1.0-(double(i)-2.0)/60.0;
        if(d>0.15)table[0][i]=float((d-0.08)*1.0869565);
        else if(d<=0)table[0][i]=0;
        else table[0][i]=float(std::pow(d,std::max(1.0,(0.505-d)*3.8)));
        table[1][i]=float(std::max(0.0,d));table[1][i]*=table[1][i];
        table[2][i]=float(d*0.5+0.5);table[3][i]=table[2][i]*table[2][i];
    }
    for(int k=0;k<4;++k) {
        float temporary[128];for(int j=0;j<128;++j)temporary[j]=std::sqrt(std::max(0.0f,table[k][j]));
        for(int j=0;j<128;++j) {
            double e=0;for(int i=0;i<=60;++i){int l=std::abs(j+i-30);if(l<128)e+=temporary[l];}
            double e007=e*0.007,inner=e*0.011+e007*e007;table[k+4][j]=float(inner*inner);
        }
    }
    for(int k=0;k<8;++k)for(int j=0;j<128;++j)out.diffuse_table[k][j]=bits(table[k][j]);
}
}
PreparedExactColor PrepareGeneralColorBase(const PreparedExactScene& scene,bool native_soft_completed,float* object_y,
    mb3d_native::NativeLightGeometry* light_geometry,Lattice lattice) {
    if(!scene.host)throw std::invalid_argument("Missing exact color scene owner");
    ScopedRenderContext binding(scene.host->context);
    if(scene.precision!=PrecisionMode::Native||scene.device.formula_kind!=4||
       scene.device.capability_id!=0x4e415431u||scene.host->file.header.bPlanarOptic>2)
        throw std::invalid_argument("Native camera color profile mismatch");
    const auto& header=scene.host->file.header;
    if(native_soft_completed && (!(header.bCalc1HSsoft&1) ||
       !(header.bCalculateHardShadow&1) || !(header.bCalculateHardShadow&0xfc)))
        throw std::invalid_argument("Invalid Native completed soft-shadow color policy");
    PaintSetup p(scene,native_soft_completed);const auto& l=p.lights;const auto& a=p.aligned;
    if(object_y)for(unsigned i=0;i<3;++i)object_y[i]=p.paint.m[i][1];
    if(light_geometry) {
        auto& g=*light_geometry;g=mb3d_native::NativeLightGeometry{};g.version=mb3d_native::kNativeLightGeometryVersion;
        g.zz_mid_difference=bits(l.sZZstmitDif);
        for(unsigned i=0;i<3;++i) {
            g.start[i]=bits(p.start[i]);g.y_add[i]=bits(p.y_add[i]);g.x_add[i]=bits(p.x_add[i]);
            for(unsigned j=0;j<3;++j)g.matrix[i][j]=bits(p.paint.m[i][j]);
        }
        for(unsigned i=0;i<6;++i) {
            auto& q=g.lights[i];q.options=unsigned(l.iLightPos[i]);q.max_length2=bits(l.sLmaxL[i]);
            q.depth_position=bits(l.sPosLP[i]);q.z_position=bits(l.sPosLightZpos[i]);
        }
    }
    PreparedExactColor result;auto& d=result.device;
    d.version=1;d.width=scene.device.width;d.height=scene.device.height;
    d.z_corr=bits(p.paint.Zcorr);d.z_mul=bits(p.paint.ZcMul);d.z_mid_difference=bits(p.paint.ZZstmitDif);
    d.step_width=bits(p.paint.StepWidth);d.back_distance=bits(p.paint.BackDist);
    d.depth=bits(l.sDepth);d.shadow_start=bits(l.sShad);d.shadow_z_mul=bits(l.sShadZmul);d.shadow_gradient=bits(l.sShadGr);
    d.ambient_amplitude=bits(l.sAmbShad);d.palette_start=bits(l.sCStart);d.palette_mul=bits(l.sCmul);d.palette_z_mul=bits(l.sColZmul);
    d.diffuse=bits(l.sDiff);d.specular=bits(l.sSpec);d.indirect_reflect=bits(l.sIndLightReflect);d.diffuse_shadowing=bits(l.sDiffuseShadowing);
    d.roughness=bits(l.sRoughnessFactor);d.dynamic_fog_mul=bits(l.sDynFogMul);d.light_pow=l.iLightPowFunc[0];d.light_function=l.iLightFuncDiff[0];
    d.hs_mask=l.iHSmask[0];d.hs_calculated=l.iHScalced[0];d.hs_enabled=l.iHSenabled[0];
    for(int c=0;c<4;++c) {
        d.depth_colors[0][c]=bits(a.sDepthCol[c]);d.depth_colors[1][c]=bits(a.sDepthCol2[c]);
        d.ambient_colors[0][c]=bits(a.sAmbCol[c]);d.ambient_colors[1][c]=bits(a.sAmbCol2[c]);
        d.fog_colors[0][c]=bits(a.sDynFogCol[c]);d.fog_colors[1][c]=bits(a.sDynFogCol2[c]);
        d.light_direction[c]=bits(a.LN[0][c]);d.light_color[c]=bits(a.sLCols[0][c]);
        for(int i=0;i<10;++i){d.palette_diffuse[i][c]=bits(a.ColDif[i][c]);d.palette_specular[i][c]=bits(a.ColSpe[i][c]);}
    }
    // The palette transparency (ColSpe w) steers SR transmission.
    for(int i=0;i<10;++i)d.palette_specular[i][3]=bits(a.ColSpe[i][3]);
    for(int i=0;i<10;++i){d.palette_positions[i]=l.ColPos[i];d.palette_divisors[i]=bits(l.sCDiv[i]);}
    BuildDiffuseTable(d);
    result.columns.resize(d.width);result.rows.resize(d.height);
    for(unsigned x=0;x<d.width;++x) {
        float position=float(int(x)*lattice.stride+lattice.x+p.paint.ppXplus+1)/float(p.header.Width);
        double angle=double((p.view.xOff-position)*p.view.sFOVy*p.aspect);
        result.columns[x]={bits(std::sin(angle)),bits(std::cos(angle))};
    }
    for(unsigned y=0;y<d.height;++y) {
        float position=float(int(y)*lattice.stride+lattice.y+p.paint.ppYplus)/float(p.header.Height);
        double angle=double((position-0.5f)*p.view.sFOVy);
        result.rows[y]={bits(std::sin(angle)),bits(std::cos(angle))};
    }
    return result;
}

}
