#include "generic_color_host.h"
#include "generic_color_base.h"
#include "comparison/metal_exact/scene_host_internal.h"
#include "core/colorizer.h"
#include <bit>
#include <cmath>
#include <cstring>
#include <stdexcept>
namespace mb3d::native_metal {
namespace {
// The loader's seam layout ((width+1)*(height+1) pixels), one FP32 raw sample
// per channel, as the Native formula map table. GetLightMapPixel returns black
// for an empty map; its descriptor then keeps width zero and no samples.
void appendMaterialMap(mb3d_native::NativeMaterialMap& out,const TLightMap& map,std::vector<float>& samples) {
    auto bits=[](float value){
        if(!std::isfinite(value))throw std::invalid_argument("Nonfinite Native material map parameter");
        return std::bit_cast<uint32_t>(value);
    };
    out.intensity=bits(map.sIntensity);
    for(unsigned c=0;c<3;++c){out.average[c]=bits(map.LMavrgCol[c]);out.average_squared[c]=bits(map.LMavrgColSqr[c]);}
    for(unsigned i=0;i<3;++i)for(unsigned j=0;j<3;++j)out.rotation[i][j]=bits(map.PicRotMatrix[i][j]);
    if(!map.LMa||map.LMWidth<=0||map.LMHeight<=0)return;
    if(map.LMWidth>16384||map.LMHeight>16384||(map.iMapType!=0&&map.iMapType!=1)||
       map.sLMXfactor!=float(map.LMWidth)||map.sLMYfactor!=float(map.LMHeight))
        throw std::invalid_argument("Native material map layout is unsupported");
    const std::size_t pixels=(std::size_t(map.LMWidth)+1)*(std::size_t(map.LMHeight)+1);
    if(samples.size()+pixels*3>(1u<<26))throw std::invalid_argument("Native material map resource work bound exceeded");
    out.width=uint32_t(map.LMWidth);out.height=uint32_t(map.LMHeight);out.offset=uint32_t(samples.size());
    out.word_map=map.iMapType==1;out.x_factor=bits(map.sLMXfactor);out.y_factor=bits(map.sLMYfactor);
    const auto* bytes=reinterpret_cast<const unsigned char*>(map.LMa);
    samples.reserve(samples.size()+pixels*3);
    for(std::size_t pixel=0;pixel<pixels;++pixel)for(unsigned channel=0;channel<3;++channel) {
        if(map.iMapType==1) {
            std::uint16_t value;std::memcpy(&value,bytes+pixel*6+(2-channel)*2,2);
            samples.push_back(float(value));
        } else samples.push_back(float((map.LMa[pixel]>>(8*(2-channel)))&255u));
    }
}
}
PreparedGeneralColor PrepareGeneralColor(const metal_exact::PreparedExactScene& scene,
                                        bool hard_shadow_completed, Lattice lattice) {
    if(!scene.host)throw std::invalid_argument("Missing Native general color scene owner");
    ScopedRenderContext binding(scene.host->context);
    PreparedGeneralColor result;
    // Native-only scene preparation; no per-pixel CPU color evaluation.
    const bool soft=hard_shadow_completed && (scene.host->file.header.bCalc1HSsoft&1);
    float object_y[3]{};
    result.base=PrepareGeneralColorBase(scene,soft,object_y,&result.lights,lattice);
    TMandHeader10 header=scene.host->file.header;
    TLightVals lights{};TLValigned aligned{};lights.PLValigned=&aligned;
    MakeLightValsFromHeaderLight(&header,&lights,1.0,0);
    CompleteLightValsSetup(&header,&lights);
    if(hard_shadow_completed && (header.bCalculateHardShadow&1)) {
        const uint8_t completed=(header.bHScalculated&(soft?1:0xfd))|header.bCalculateHardShadow;
        for(int x=0;x<6;++x) {
            const int y=lights.SortTab[x];
            lights.iHScalced[y]=(completed>>(x+2))&1;
            lights.iHSmask[y]=0x400<<x;
            if(soft && lights.iHScalced[y])lights.iHSmask[y]=-1;
            if((completed&2)&&((header.bCalculateHardShadow>>(x+2))&1)) lights.iLightFuncDiff[y]=0;
        }
    }
    auto bits=[](float value){
        if(!std::isfinite(value))throw std::invalid_argument("Nonfinite Native interior color parameter");
        return std::bit_cast<uint32_t>(value);
    };
    auto& d=result.interior;d.version=soft?7:6;d.depth_function=lights.iDfunc;
    d.flags=(lights.bColCycling?1u:0u)|(lights.bNoColIpol?2u:0u)|
        (lights.bFarFog?4u:0u)|((lights.iColOnOT&1)?8u:0u)|
        ((lights.bDFogOptions&2)?16u:0u)|((lights.bDFogOptions&1)?32u:0u)|
        (lights.bAmbRelObj?64u:0u);
    if(lights.iExModes)d.flags|=mb3d_native::kNativeTotalLight2Flag;
    if(lights.bVolLight)d.flags|=mb3d_native::kNativeVolumeLightFlag;
    if(lights.bAmbRelObj)for(unsigned i=0;i<3;++i)d.object_y[i]=bits(object_y[i]);
    d.gamma_mode=lights.iGammaH;d.gamma=bits(lights.sGamma);d.squared=lights.bCalcPixColSqr?1u:0u;
    if(d.gamma_mode < -1 || d.gamma_mode > 1 || lights.sGamma<0 || lights.sGamma>1)
        throw std::invalid_argument("Invalid Native gamma descriptor");
    for(unsigned i=0;i<6;++i) {
        auto& q=d.lights[i];q.enabled=lights.iLightOption[i]==0;
        if(!q.enabled)continue;
        if(lights.iLightPowFunc[i]<1||lights.iLightPowFunc[i]>1024||(lights.iLightPowFunc[i]&(lights.iLightPowFunc[i]-1))||
           lights.iLightFuncDiff[i]<0||lights.iLightFuncDiff[i]>3||
           ((!soft||lights.iHSmask[i]!=-1)&&(lights.iHSmask[i]<0||lights.iHSmask[i]>65535))||
           lights.iHScalced[i]<0||lights.iHScalced[i]>1||lights.iHSenabled[i]<0||lights.iHSenabled[i]>1)
            throw std::invalid_argument("Invalid Native directional light descriptor");
        if(lights.iLightPos[i]) {
            d.flags|=mb3d_native::kNativeLightGeometryFlag;
            if(!std::isfinite(lights.sLmaxL[i])||!std::isfinite(lights.sPosLP[i])||
               !std::isfinite(lights.sPosLightZpos[i])||!std::isfinite(lights.sZZstmitDif))
                throw std::invalid_argument("Nonfinite Native light position descriptor");
        }
        q.power=lights.iLightPowFunc[i];q.function=lights.iLightFuncDiff[i];
        q.hs_mask=lights.iHSmask[i];q.hs_calculated=lights.iHScalced[i];q.hs_enabled=lights.iHSenabled[i];
        for(unsigned j=0;j<4;++j){q.direction[j]=bits(aligned.LN[i][j]);q.color[j]=bits(aligned.sLCols[i][j]);}
    }
    // Material maps: the shared loader decided which maps are effective; a
    // failed load already restored iColOnOT&1 and light option 1.
    auto& maps=result.maps;maps.version=mb3d_native::kNativeMaterialMapVersion;
    if(lights.iColOnOT>1&&lights.DiffColLightMap) {
        if(lights.iColOnOT>9)throw std::invalid_argument("Invalid Native diffuse map mode");
        maps.diffuse_mode=uint32_t(lights.iColOnOT);maps.yc_combine=lights.bYCcomb?1u:0u;
        maps.offset_x=bits(lights.DCLMapOffX);maps.offset_y=bits(lights.DCLMapOffY);
        maps.rotation_sin=bits(lights.DCLMapRotSin);maps.rotation_cos=bits(lights.DCLMapRotCos);
        maps.scale=bits(lights.lvMapScale);
        for(unsigned i=0;i<3;++i)maps.mid[i]=bits(float(lights.lvMidPos[i]));
        appendMaterialMap(maps.diffuse,*lights.DiffColLightMap,result.map_samples);
        // GetDiffMapWrap3D reads CalcObjPos and the paint matrix.
        if(lights.iColOnOT>5)d.flags|=mb3d_native::kNativeLightGeometryFlag;
    }
    for(unsigned i=0;i<6;++i)if(lights.iLightOption[i]==2&&lights.LLightMaps[i]) {
        maps.light_mask|=1u<<i;appendMaterialMap(maps.lights[i],*lights.LLightMaps[i],result.map_samples);
    }
    // The background map (BGsmallLM shares it) along AbsViewVec and, for the
    // ambient light, around the paint-rotated normal: both need the geometry.
    if(lights.bBackBMP&&lights.BGLightMap) {
        maps.background=mb3d_native::kNativeBackgroundMap|(lights.bAddBGlight?mb3d_native::kNativeBackgroundAddLight:0u)|
            (lights.bUseSmallBGpicForAmb?mb3d_native::kNativeBackgroundAmbient:0u);
        appendMaterialMap(maps.background_map,*lights.BGLightMap,result.map_samples);
        d.flags|=mb3d_native::kNativeLightGeometryFlag;
    } else if(lights.bUseSmallBGpicForAmb) {
        // Without a loaded map the ambient gradient follows the paint-rotated normal.
        maps.background=mb3d_native::kNativeBackgroundRotatedAmbient;
        d.flags|=mb3d_native::kNativeLightGeometryFlag;
    }
    maps.sample_count=uint32_t(result.map_samples.size());
    if(maps.diffuse_mode||maps.light_mask||maps.background)d.flags|=mb3d_native::kNativeMaterialMapFlag;
    d.palette_start=bits(lights.sCiStart);d.palette_mul=bits(lights.sCimul);
    for(unsigned i=0;i<4;++i) {
        if(lights.IColPos[i]<0||lights.IColPos[i]>32767||
           (i&&lights.IColPos[i]<lights.IColPos[i-1]))
            throw std::invalid_argument("Invalid Native interior palette positions");
        d.positions[i]=lights.IColPos[i];d.divisors[i]=bits(lights.sICDiv[i]);
        for(unsigned channel=0;channel<4;++channel)d.colors[i][channel]=bits(aligned.ColInt[i][channel]);
    }
    // The camera-aware reflection material (CalcPixelColorSvec) shades every
    // enabled light, positional lights, squared color with the gamma output,
    // CalcTotalLight2, material maps, soft-shadow intensity and visible light
    // shapes from each ray's CamPos and AbsViewVec. Inside an absorbing
    // material (bDivOptions) visible lights take a separate CPU branch.
    if(header.bCalcSRautomatic&1) {
        const auto& m=scene.host->mct;
        for(unsigned i=0;i<6;++i)if((lights.iLightPos[i]&14)&&(m.bInsideRendering||m.bInAndOutside))
            throw std::invalid_argument("Unsupported Native reflection visible light inside a material");
        if(lights.bBackBMP&&(m.bInsideRendering||m.bInAndOutside))
            throw std::invalid_argument("Unsupported Native reflection background inside a material");
        for(unsigned i=0;i<6;++i) {
            const auto mask=d.lights[i].hs_mask;
            if(d.lights[i].enabled&&!(soft&&mask==0xffffffffu)&&(mask<1024||mask>32768||(mask&(mask-1))))
                throw std::invalid_argument("Unsupported Native reflection directional material");
        }
        auto& r=result.reflection;r.version=mb3d_native::kNativeReflectionMaterialVersion;
        // SR_CalcLightZPos: LVals.sStepWidth is the header step width.
        r.step_width=bits(float(header.dStepWidth));r.z_c_mul=bits(float(m.ZcMul));r.z_corr=bits(float(m.Zcorr));
        if(!(header.dStepWidth>0)||!(m.ZcMul>0)||!(m.Zcorr>0))
            throw std::invalid_argument("Invalid Native reflection light depth constants");
        const double mid[3]={m.Xmit,m.Ymit,m.Zmit};
        for(unsigned i=0;i<3;++i)r.mid[i]=bits(float(mid[i]));
        r.geometry=result.lights;r.maps=result.maps;
    }
    return result;
}
} // namespace mb3d::native_metal
