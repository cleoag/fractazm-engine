#include "general_normals_host.h"
#include "core/formulas.h"
#include "core/numeric_precision.h"
#include <stdexcept>
namespace mb3d::native_metal {
mb3d_native::NativeNormalParams PrepareGeneralNormals(const TMCTparameter& m,
    const mb3d_native::NativeFormulaProgram& program) {
    using namespace mb3d_native;
    if(!NativePrecisionEnabled()||!m.bMCTisValid||m.iSmNormals>15||
       m.iMaxIt!=program.max_iterations)
        throw std::invalid_argument("Native normal scene/state contract");
    if(!m.NormalsOnDE){
        const bool same=(program.driver==Hybrid3D&&m.mMandFunction==doHybridPas)||
            (program.driver==Hybrid4D&&m.mMandFunction==doHybrid4DPas)||
            (program.driver==Interpolated3D&&m.mMandFunction==doInterpolHybridPas)||
            (program.driver==Interpolated4D&&m.mMandFunction==doInterpolHybridPas4D);
        if(m.IsCustomDE||!same)throw std::invalid_argument("Native direct normal orbit driver mismatch");
    }
    NativeNormalParams out{};out.version=kGeneralNormalVersion;
    out.normals_on_de=m.NormalsOnDE?1:0;out.smoothing=m.iSmNormals;
    out.inside=m.bInsideRendering?1:0;return out;
}
}
