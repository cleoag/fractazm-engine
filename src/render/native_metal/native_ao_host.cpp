#include "native_ao_host.h"
#include "native_ao_math.h"
#include "native_ao_de_math.h"
#include "core/header_trafos.h"
#include "core/numeric_precision.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>

namespace mb3d {
NativeAOPreparation PrepareNativeScreenAO(const TMandHeader10& h,
    unsigned output_width,unsigned output_height,unsigned workers,unsigned work_limit) {
    using namespace mb3d_native_ao;
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    if(h.Width<=0||h.Height<=0||h.Width>4096||h.Height>4096||
       !output_width||!output_height||output_width>4096||output_height>4096||!workers)
        throw std::invalid_argument("Native AO dimensions/workers");
    const std::uint64_t output_count=std::uint64_t(output_width)*output_height;
    if(output_count<std::uint64_t(h.Width)*h.Height)
        throw std::invalid_argument("Native screen AO full-frame prefix exceeds tile allocation");
    if(!(h.bCalcAmbShadowAutomatic&1))throw std::invalid_argument("Native AO disabled");
    const U32 algorithm=dispatch(h.bCalcAmbShadowAutomatic,h.bPlanarOptic);
    if(algorithm>9u)throw std::invalid_argument("Native screen AO preparation requires image-space algorithm");
    if(!std::isfinite(h.sAmbShadowThreshold))throw std::invalid_argument("Native AO threshold");
    NativeAOPreparation result;
    auto& p=result.params;
    p.version=Version;p.width=unsigned(h.Width);p.height=unsigned(h.Height);p.algorithm=algorithm;
    p.output_count=unsigned(output_count);p.logical_lanes=std::min(workers,p.height);
    p.work_limit=work_limit;p.quality=(h.bCalcAmbShadowAutomatic>>4)&3;
    p.repeats=random_mode(algorithm)?std::max(1u,unsigned(h.SSAORcount)):1u;
    p.threshold=std::max(0.01f,h.sAmbShadowThreshold);
    p.first_step_random=(h.bCalcAmbShadowAutomatic>>7)&1;p.dither=h.AODEdithering;
    const float border=std::min(h.bSSAO24BorderMirrorSize*0.01f,0.9f);
    p.w_lo=I32(std::nearbyint(float(p.width)*border));p.w_hi=I32(p.width)-1-p.w_lo;
    p.h_lo=I32(std::nearbyint(float(p.height)*border));p.h_hi=I32(p.height)-1-p.h_lo;
    const int radius=int(std::nearbyint(std::sqrt(float(p.width*p.width+p.height*p.height))*0.5f));
    p.levels=1;int bound=5;
    do {++p.levels;bound*=2;}while(p.levels<15u&&bound<=radius);
    constexpr double pi=3.141592653589793238462643383279502884;
    for(unsigned i=0;i<32;++i){p.directions[i*2]=float(std::sin(double(i)*pi/16));p.directions[i*2+1]=float(std::cos(double(i)*pi/16));}
    TMandHeader10 copy=h;double zcorr,zcmul,zdiff;CalcPPZvals(copy,zcorr,zcmul,zdiff);
    p.depth_scale=float((std::pow(256.0/zcmul+1.0,2.0)-1.0)/zcorr);
    if(algorithm<=Threshold15T0) {
        p.levels=1;int reduced=h.Width/16;
        do{++p.levels;reduced>>=1;}while(p.levels<8u&&reduced>=4);
        const bool t0=algorithm==Threshold15T0;
        const double a=std::atan(double(p.threshold)*(t0?0.8:1.0)*std::sqrt(std::sqrt(double(p.levels))));
        p.sum_multiplier=float(1.35*32767.0/(pi*32.0*std::pow(a,t0?0.9:0.8)));
        if(!valid(p)||p.sum_multiplier<=0.0f)throw std::invalid_argument("Native threshold AO invalid preparation");
        return result;
    }
    for(unsigned repeat=0;repeat<p.repeats;++repeat) for(unsigned index=1;index<=p.levels;++index) {
        Level l{};l.version=Version;l.index=index;l.step=1u<<(index-1);l.samples=index==1?5u:3u;
        l.repeat_index=repeat;l.add_limit=random_mode(algorithm)?16383u/p.repeats:16383u;
        l.minimum_radius=index==1?1.0f:3.25f*float(l.step);
        l.maximum_radius=int(std::nearbyint(double(l.minimum_radius)+double(l.step*l.samples)+0.1));
        for(unsigned j=0;j<l.samples;++j)l.reciprocal_radius[j]=float(1.0/(double(l.minimum_radius)+double(j*l.step)+0.1));
        l.sit=float(double(p.depth_scale)/22000.0*4096.0);
        const double factor=std::sqrt(std::sqrt(double(p.levels)/index));
        const bool t0=threshold_mode(algorithm),random=random_mode(algorithm);
        const double prefix=double(p.threshold)*(t0?1.0:0.7)*4096.0/double(l.sit)*factor;
        l.z_threshold=float(t0?prefix*prefix:prefix);
        const double angle=std::atan(double(p.threshold)*(random?(t0?0.64:0.65):0.6)*std::sqrt(std::sqrt(double(p.levels))));
        l.sum_multiplier=float(1.5*32767.0/(pi*32.0*(t0?angle:std::pow(angle,0.9)))/(random?p.repeats:1));
        if(!random)l.sit*=l.z_threshold;
        if(!valid(p,l))throw std::invalid_argument("Native AO nonfinite or invalid prepared level");
        result.levels.push_back(l);
    }
    if(!valid(p))throw std::invalid_argument("Native AO invalid preparation");
    return result;
}

mb3d_native_ao::DEParams PrepareNativeDistanceAO(const TMandHeader10& h,
    const TMCTparameter& m,unsigned width,unsigned height,unsigned workers,
    unsigned max_steps,unsigned work_limit) {
    using namespace mb3d_native_ao;
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    if(!(h.bCalcAmbShadowAutomatic&1)||dispatch(h.bCalcAmbShadowAutomatic,h.bPlanarOptic)!=DistanceEstimate||
       !width||!height||width>4096||height>4096||!workers||workers>4096||
       m.iMandWidth<=0||m.iMandHeight<=0||m.iMandWidth>4096||m.iMandHeight>4096)
        throw std::invalid_argument("Native DE AO dimensions/selection/workers");
    DEParams p{};p.version=Version;p.width=width;p.height=height;
    p.task_rows=workers<=1?height:std::max(1u,height/(workers*4u));
    p.task_count=(height+p.task_rows-1)/p.task_rows;
    p.quality=(h.bCalcAmbShadowAutomatic>>4)&3;p.dither=h.AODEdithering;
    p.first_step_random=(h.bCalcAmbShadowAutomatic>>7)&1;
    p.max_steps=max_steps;p.work_limit=work_limit;
    p.vary_stop=m.bVaryDEstop!=0;p.initial_inside=U32(m.bInsideRendering);
    p.in_and_out=m.bInAndOutside!=0;
    // Ordinary main copies this MCT into each task; it does not assign a new
    // iThreadId for AO (unlike the primary renderer). Preserve its actual seed.
    p.initial_seed=(U32(std::max(1,int(m.iThreadId)))+1u)*0x324594a1u+0x24563487u;
    p.cut_options=U32(m.iCutOptions);p.base_stop=m.DEstop;p.stop_factor=m.mctDEstopFactor;
    p.step_width=float(m.StepWidth);p.z_corr=float(m.Zcorr);p.z_mul=float(m.ZcMul);
    p.full_width=U32(m.iMandWidth);p.full_height=U32(m.iMandHeight);
    p.left=m.CalcRect.Left;p.top=m.CalcRect.Top;
    p.max_distance_base=m.DEAOmaxL*.5f*std::sqrt(float(p.full_width*p.full_width+p.full_height*p.full_height));
    constexpr float pi=3.14159265358979323846f;
    if(!p.quality){p.angular_spacing=60.f*(pi/180.f);p.step_multiplier=1.8f;p.minimum_dot=-1.f;p.correction_weight=.3f;}
    else {
        p.angular_spacing=(pi*.5f)/(float(p.quality)+.9f);
        p.step_multiplier=1.f+std::sin(p.angular_spacing);
        p.minimum_dot=std::cos(p.angular_spacing*1.2f);
        p.correction_weight=p.quality==1?.2f:.1666f;
        for(unsigned row=1;row<=p.quality;++row){
            const double count=std::sin(float(row)*p.angular_spacing)*3.14159265358979323846*2.0/p.angular_spacing;
            if(!std::isfinite(count)||count<.5||count>32.49)throw std::invalid_argument("Native DE AO ray count");
            p.row_counts[row]=U32(std::lround(count));
        }
    }
    for(unsigned i=0;i<3;++i){
        const double x=m.Vgrads[i][0],y=m.Vgrads[i][1],z=m.Vgrads[i][2];
        const double length=std::sqrt((x*x+y*y)+z*z);
        const double scale=length>1e-30?double(p.step_width)/length:double(p.step_width);
        p.view_matrix[i][0]=float(x*scale);p.view_matrix[i][1]=float(y*scale);p.view_matrix[i][2]=float(z*scale);
    }
    p.cut[0]=float(m.dCOX);p.cut[1]=float(m.dCOY);p.cut[2]=float(m.dCOZ);
    if(!valid_de(p))throw std::invalid_argument("Native DE AO nonfinite or invalid preparation");
    return p;
}
}
