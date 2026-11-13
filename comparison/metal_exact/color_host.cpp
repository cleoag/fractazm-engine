#include "color_host.h"
#include "scene_host_internal.h"
#include "core/colorizer.h"
#include "core/header_trafos.h"
#include "core/math3d.h"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <future>
#include <stdexcept>
namespace mb3d::metal_exact {
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
    explicit PaintSetup(const PreparedExactScene& scene) {
        header=scene.host->file.header;
        lights.PLValigned=&aligned;
        MakeLightValsFromHeaderLight(&header,&lights,1.0,0);
        CompleteLightValsSetup(&header,&lights);
        // Production Render applies this state after completing the hard-shadow
        // pass, even when every light ray happened to remain unobstructed.
        const uint8_t completed=(header.bHScalculated&0xfd)|header.bCalculateHardShadow;
        if(header.bCalculateHardShadow&1) for(int x=0;x<6;++x) {
            const int y=lights.SortTab[x];
            lights.iHScalced[y]=(completed>>(x+2))&1;
            lights.iHSmask[y]=0x400<<x;
            if((completed&2)&&((header.bCalculateHardShadow>>(x+2))&1))
                lights.iLightFuncDiff[y]=0;
        }
        if(header.bCalc1HSsoft&1 || header.bPlanarOptic!=0 || header.TilingOptions ||
           lights.bCalcPixColSqr || lights.bAmbRelObj || lights.bBackBMP ||
           lights.bUseSmallBGpicForAmb || lights.bVolLight || lights.bYCcomb ||
           lights.iGammaH || lights.iDfunc || lights.iExModes || lights.iColOnOT ||
           !lights.bColCycling || lights.bNoColIpol || !lights.bFarFog ||
           lights.bDFogOptions || lights.DiffColLightMap || lights.BGLightMap || lights.BGsmallLM)
            throw std::runtime_error("Unsupported exact Metal color capability");
        for(int i=0;i<6;++i) if(lights.iLightOption[i]!=(i==0?0:1)||lights.iLightPos[i]||lights.LLightMaps[i])
            throw std::runtime_error("Unsupported exact Metal light capability");
        if(lights.iLightPowFunc[0]!=64||lights.iLightFuncDiff[0]!=((header.bCalculateHardShadow&1)?0:3)||
           lights.iHScalced[0]!=((header.bCalculateHardShadow&1)?1:0)||lights.iHSenabled[0]!=1||lights.iHSmask[0]!=1024)
            throw std::runtime_error("Unsupported exact Metal completed shadow state");
        paint.ppWidth=header.Width;paint.ppHeight=header.Height;paint.ppYinc=1;
        paint.PLVals=&lights;
        CalcStepWidth(&header);paint.StepWidth=header.dStepWidth;
        TMatrix3 source{};std::memcpy(source,header.hVGrads,sizeof(source));
        NormaliseMatrixTo(gradients,header.dStepWidth,source);paint.pVgrads=&gradients;
        paint.sFOVy=float(header.dFOVy*Pid180);
        CalcPPZvals(header,paint.Zcorr,paint.ZcMul,paint.ZZstmitDif);
        const double fov=std::min(1.5,std::max(0.001,double(paint.sFOVy)*0.5));
        paint.ppPlOpticZ=float(std::cos(fov)*fov/std::sin(fov));
        paint.ppXOff=CalcXoff(&header);paint.ppPaintWidth=header.Width;paint.ppPaintHeight=header.Height;
        GetStartSPosAndAddVecs(view,paint,start,y_add,x_add);
        aspect=float(header.Width)/float(header.Height);
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
PreparedExactColor PrepareExactColor(const PreparedExactScene& scene) {
    if(!scene.host)throw std::invalid_argument("Missing exact color scene owner");
    ScopedRenderContext binding(scene.host->context);
    PaintSetup p(scene);const auto& l=p.lights;const auto& a=p.aligned;
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
    for(int i=0;i<10;++i){d.palette_positions[i]=l.ColPos[i];d.palette_divisors[i]=bits(l.sCDiv[i]);}
    BuildDiffuseTable(d);
    result.columns.resize(d.width);result.rows.resize(d.height);
    for(unsigned x=0;x<d.width;++x) {
        float position=float(x+1)/float(d.width);
        double angle=double((p.view.xOff-position)*p.view.sFOVy*p.aspect);
        result.columns[x]={bits(std::sin(angle)),bits(std::cos(angle))};
    }
    for(unsigned y=0;y<d.height;++y) {
        float position=float(y)/float(d.height);
        double angle=double((position-0.5f)*p.view.sFOVy);
        result.rows[y]={bits(std::sin(angle)),bits(std::cos(angle))};
    }
    return result;
}

std::vector<ExactColorPixel> ReferenceColor(const PreparedExactScene& scene,
                                          const std::vector<ExactSurface>& surface,unsigned threads) {
    if(!scene.host||surface.size()!=size_t(scene.device.width)*scene.device.height)
        throw std::invalid_argument("Invalid exact color reference inputs");
    threads=std::clamp(threads,1u,scene.device.height);
    std::vector<ExactColorPixel> output(surface.size());
    std::vector<std::future<void>> tasks;
    for(unsigned lane=0;lane<threads;++lane) tasks.push_back(std::async(std::launch::async,[&,lane] {
        ScopedRenderContext binding(scene.host->context);PaintSetup p(scene);
        for(unsigned y=lane;y<scene.device.height;y+=threads) {
            TSVec row{};for(int c=0;c<3;++c)row[c]=p.start[c]+p.y_add[c]*float(y);
            p.view.yPos=float(y)/p.header.Height;PreCalcDepthCol(p.lights.iDfunc,&p.view,p.lights.PLValigned);
            for(unsigned x=0;x<scene.device.width;++x) {
                size_t i=size_t(y)*scene.device.width+x;const auto& s=surface[i];
                TsiLight5 psi{};
                psi.NormalX=s.normal[0];psi.NormalY=s.normal[1];psi.NormalZ=s.normal[2];
                psi.RoughZposFine=s.depth_packed&65535;psi.Zpos=s.depth_packed>>16;psi.Shadow=s.shadow;
                psi.AmbShadow=s.amb_shadow;psi.SIgradient=s.si_gradient;psi.OTrap=s.o_trap;
                TSVec pos{};for(int c=0;c<3;++c)pos[c]=row[c]+p.x_add[c]*float(x);
                p.view.xPos=float(x+1)/p.header.Width;CalcViewVec(&p.view,p.aspect);
                CalcObjPos(p.view,p.paint,&psi,pos);
                uint32_t dib=0;
                CalcPixelColor2(&dib,&psi,&p.lights,&p.view);
                output[i].rgb=((dib>>16)&255)|(dib&0xff00)|((dib&255)<<16);
            }
        }
    }));
    for(auto& t:tasks)t.get();return output;
}
}
