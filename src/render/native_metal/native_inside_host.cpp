#include "native_inside_host.h"
#include "native_inside_math.h"
#include "native_inside_reflection_math.h"
#include <cmath>
#include <stdexcept>
namespace mb3d::native_metal {
mb3d_inside::Params PrepareNativeInside(const TMCTparameter& m) {
    mb3d_inside::Params p{};
    p.version=mb3d_inside::Version;
    p.initial_inside=m.bInsideRendering;
    p.in_and_out=m.bInAndOutside!=0;
    p.volume_light=m.DFogOnIt==65535;
    if(m.iMandWidth<=0||m.bCalcInside!=m.bInsideRendering)
        throw std::invalid_argument("Native inside primary policy mismatch");
    p.full_width=static_cast<unsigned>(m.iMandWidth);
    if(!mb3d_inside::valid(p))throw std::invalid_argument("Native inside policy ABI/range");
    return p;
}
mb3d_inside::ReflectionConfig PrepareNativeInsideReflection(const TMCTparameter& m,const TMandHeader10& header,
                                                            const THeaderCustomAddon& addon) {
    mb3d_inside::ReflectionConfig p{};p.version=1;p.is_ifs=m.bIsIFS!=0;
    // CPU reflection.cpp:1092/1095. This is constant scene preparation only.
    const double exponent=header.dStepWidth*static_cast<double>(header.sTransmissionAbsorption);
    if(!std::isfinite(exponent))throw std::invalid_argument("Native inside absorption coefficient overflow");
    p.absorption=static_cast<float>(exponent);p.scattering=header.sTRscattering/330.f;
    if(header.bCalcSRautomatic&2) {
        // SR_CalcRay toggles bInsideRendering per refraction and compares it
        // with the initial raw LongBool; only the outside start (zero) keeps
        // NotInsideXor true on every ray. Bit 4 restricts transmission to
        // dIFS parts, which needs each surface's DEoptionResult.
        if(header.bCalcSRautomatic&4)
            throw std::invalid_argument("Unsupported Native reflection transmission restricted to dIFS");
        if(m.bInsideRendering||m.bCalcInside||m.bInAndOutside)
            throw std::invalid_argument("Unsupported Native reflection transmission from inside rendering");
        // reflection.cpp:1084: TransDiConst.
        float index=(addon.bOptions2&6)==4?1.0f:header.sTRIndex;
        if(index<=0.0f)index=1.5f;
        if(!std::isfinite(index))throw std::invalid_argument("Invalid Native reflection refraction index");
        p.transmission=1;p.refraction=index;
    }
    if(!mb3d_inside::valid_reflection(p))throw std::invalid_argument("Native inside reflection config ABI/range");
    return p;
}
}
