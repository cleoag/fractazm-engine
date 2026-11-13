#include "dof_host.h"
#include "core/header_trafos.h"
#include "core/numeric_precision.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mb3d::native_metal {
mb3d_dof::Params PrepareNativeDOF(const TMandHeader10& h) {
    mb3d_dof::Params p{};
    if(!(h.bCalcDOFtype&1)||!h.bCalc3D)return p;
    if(h.Width<1||h.Height<1||h.Width>4096||h.Height>4096||h.TilingOptions)
        throw std::invalid_argument("Native DOF dimensions/tiling unsupported");
    for(double v:{double(h.sDOFaperture),double(h.sDOFclipR),double(h.sDOFZsharp),
                  double(h.sDOFZsharp2),h.dFOVy,h.dZoom,h.dZstart,h.dZend})
        if(!std::isfinite(v))throw std::invalid_argument("Nonfinite Native DOF parameter");
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    auto copy=h;double zcorr,zmul,unused;CalcPPZvals(copy,zcorr,zmul,unused);
    p.version=mb3d_dof::Version;p.width=h.Width;p.height=h.Height;
    p.plain=(h.bCalcDOFtype>>3)&1;p.passes=((h.bCalcDOFtype>>1)&3)+1;
    p.inverse_z_mul=float(1/zmul);p.inverse_z_corr=float(1/zcorr);
    p.fov_factor=float(std::max(0.,h.dFOVy*0.017453292519943295769236907684886))/float(h.Height);
    p.focus_low=std::min(h.sDOFZsharp,h.sDOFZsharp2)*float(h.Width);
    p.focus_high=std::max(h.sDOFZsharp,h.sDOFZsharp2)*float(h.Width);
    p.aperture=std::clamp(h.sDOFaperture,.0001f,2.f);
    p.clip_radius=std::clamp(h.sDOFclipR,.1f,1000.f);
    for(float v:{p.inverse_z_mul,p.inverse_z_corr,p.fov_factor,p.focus_low,p.focus_high,p.aperture,p.clip_radius})
        if(!std::isfinite(v))throw std::invalid_argument("Native DOF FP32 range");
    const float lowDen=1+p.focus_low*p.fov_factor,highDen=1+p.focus_high*p.fov_factor;
    if(p.inverse_z_mul<=0||p.inverse_z_corr<=0||!std::isfinite(lowDen)||!std::isfinite(highDen)||lowDen<=0||highDen<=0)
        throw std::invalid_argument("Native DOF invalid depth/focus denominator");
    return p;
}
}
