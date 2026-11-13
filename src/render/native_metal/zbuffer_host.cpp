#include "zbuffer_host.h"
#include "core/header_trafos.h"
#include "core/numeric_precision.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>
namespace mb3d::native_metal {
mb3d_zn::Params PrepareNativeZNormals(const TMandHeader10& header) {
    const ScopedNumericPrecision precision(PrecisionMode::Native);
    mb3d_zn::Params p{};
    if(!header.bCalc3D||!(header.byCalcNsOnZBufAuto&1))return p;
    int width,height,left,top;GetPaintTileSizes(&header,width,height,left,top);
    if(width<1||height<1||width>4096||height>4096||header.Width<1||header.Height<1||
       header.Width>4096||header.Height>4096||left< -4096||left>4096||top< -4096||top>4096)
        throw std::invalid_argument("Native Z normals rectangle outside bounds");
    p.version=mb3d_zn::Version;p.width=width;p.height=height;p.count=width*height;
    p.full_width=header.Width;p.full_height=header.Height;p.left=left;p.top=top;p.mode=header.bPlanarOptic&3;
    p.step=float(header.dStepWidth);
    p.aspect=header.bPlanarOptic==2?2.f:float(header.Width)/float(header.Height);
    p.fov=float(header.bPlanarOptic==2?3.14159265358979323846:header.dFOVy*0.017453292519943295769);
    const double halfFov=std::clamp(double(p.fov*.5f),.01,1.5);
    p.optic_z=float(std::cos(halfFov)*halfFov/std::sin(halfFov));
    auto copy=header;double correction,multiplier,unused;CalcPPZvals(copy,correction,multiplier,unused);
    p.inverse_z_mul=float(1/multiplier);p.depth_correction=float(double(p.step)/correction);
    p.stop_factor=float(GetDEstopFactor(&copy));
    for(float v:{p.step,p.aspect,p.fov,p.optic_z,p.inverse_z_mul,p.depth_correction,p.stop_factor})
        if(!std::isfinite(v))throw std::invalid_argument("Native Z normals nonfinite FP32 descriptor");
    if(p.step<=0||p.aspect<=0||p.fov<0||p.optic_z<=0||p.inverse_z_mul<=0||p.depth_correction<=0)
        throw std::invalid_argument("Native Z normals invalid projection");
    return p;
}
}
