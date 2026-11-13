// external_formula_table.cpp — First 19 external formula registrations
//
// Each .inc file is included directly in the mb3d namespace. The files define:
//   - A constexpr unsigned int FORMULA_XXX = <id>;
//   - A template<> cpu_double3 execute_formula<FORMULA_XXX>(...) specialization
//   - Possibly alternative direct functions (uniquely named per formula)
//   - A FormulaInfo metadata struct (uniquely named per formula)

#include "core/external_formulas.h"
#include "core/generated/external_formula_table_generated.h"
#include <cmath>
#include <cstring>
#include <cstdint>

// Scoped defines for .inc file compatibility
#define constant const

using std::abs;
using std::sqrt;
using std::sin;
using std::cos;
using std::atan2;
using std::asin;
using std::acos;
using std::log;
using std::exp;
using std::pow;
using std::sinh;
using std::cosh;
using std::tanh;
using std::max;
using std::min;
using std::rint;

namespace mb3d {

// FormulaInfo struct used by .inc metadata blocks (not used by the engine,
// but must be defined so the .inc files compile)
struct FormulaInfo {
    unsigned int id;
    const char* name;
    int param_count;
    int complexity;
    bool uses_transcendental;
};

// Alias for Sierpinski4.inc which calls buildRotMatrix4d instead of buildRotMatrix4
inline RotMatrix4 buildRotMatrix4d(double a1, double a2, double a3,
                                    double a4, double a5, double a6) {
    return buildRotMatrix4(a1, a2, a3, a4, a5, a6);
}

// ============================================================================
// Include all .inc formula files
// ============================================================================

#include "Ads/_Folding1.inc"
#include "Ads/_AbsX.inc"
#include "3D/BristorBrot.inc"
#include "3D/Benesi1Pow2.inc"
#include "Ads/_FlipXYc.inc"
#include "3Da/_AmazingBoxSSE2.inc"
#include "3Da/_AmazingBox.inc"
#include "3Da/ABoxMod1.inc"
#include "3Da/MengerIFS.inc"
#include "4D/CommQuat.inc"
#include "Ads/_Rotate.inc"
#include "dIFS/boxIFS.inc"
#include "dIFS/gearIFS.inc"
#include "dIFS/SphereIFS.inc"
#include "3Da/Sierpinski3.inc"
#include "3Da/ABoxVaryScale.inc"
#include "4D/GeneralQuat.inc"
#include "dIFSa/transformIFS.inc"

// Direct ports of the three Benesi x87 transforms and three Pascal JIT sources.
// Keep the decimal constants from the assets (not recomputed square roots).
constexpr unsigned FORMULA_BENESIT1_NATIVE = 95001;
constexpr unsigned FORMULA_BENESIT2_NATIVE = 95002;
constexpr unsigned FORMULA_BENESIT3_NATIVE = 95003;
constexpr unsigned FORMULA_JIT_IQ_NATIVE = 95004;
constexpr unsigned FORMULA_JIT_PINE_NATIVE = 95005;
constexpr unsigned FORMULA_JIT_TRIG_NATIVE = 95006;
static double nativeOption(TPIteration3D it, int index) {
    double value;
    std::memcpy(&value, static_cast<const unsigned char*>(it->PVar) - 16 - index * 8, 8);
    return value;
}
static void benesiTransform(double& x, double& y, double& z, int mode, double scale, double offset) {
    constexpr double a = 0.816496580927726, b = 0.5773502691896258, c = 0.7071067811865475;
    double tx = (x*a-z*b)*c, yp = y*c, zp = x*b+z*a;
    double u=tx-yp, v=tx+yp;
    if(mode==1) {
        x=std::abs(u); y=std::abs(v); z=std::abs(zp);
        tx=(x+y)*c; y=(-x+y)*c;
        x=(tx*a+z*b)*scale-offset;
        z=(-tx*b+z*a)*scale; y*=scale;
    } else {
        u=mode==2?u*u:std::abs(u); v=mode==2?v*v:std::abs(v); zp=mode==2?zp*zp:std::abs(zp);
        auto fold=[&](double value){return std::abs((mode==2?std::sqrt(value):value)-offset)*scale;};
        x=fold(v+zp)*c; y=fold(u+zp)*c; z=fold(u+v);
        tx=x+y; y=y-x;
        x=tx*a+z*b; z=-tx*b+z*a;
    }
}
static void nativeBenesi1(double& x,double& y,double& z,double&,TPIteration3D it) {benesiTransform(x,y,z,1,nativeOption(it,0),nativeOption(it,1));}
static void nativeBenesi2(double& x,double& y,double& z,double&,TPIteration3D it) {benesiTransform(x,y,z,2,nativeOption(it,0),nativeOption(it,1));}
static void nativeBenesi3(double& x,double& y,double& z,double&,TPIteration3D it) {benesiTransform(x,y,z,3,nativeOption(it,0),nativeOption(it,1));}
static void nativeJitIQ(double& x,double& y,double& z,double&,TPIteration3D it) {
    double power=nativeOption(it,0),r=std::pow(std::sqrt(x*x+y*y+z*z),power);
    double theta=std::atan2(std::sqrt(x*x+z*z),y)*power,phi=std::atan2(x,z)*power;
    x=std::sin(phi)*std::sin(theta)*r+it->J1;
    y=std::cos(theta)*r+it->J2;
    z=std::sin(theta)*std::cos(phi)*r+it->J3;
}
static void nativeJitPine(double& x,double& y,double& z,double&,TPIteration3D it) {
    constexpr double a=.816496580927726,b=.577350269189626,c=.707106781186548;
    double offset=nativeOption(it,0),scale=nativeOption(it,1),tx=(x*a-z*b);
    z=std::abs(x*b+z*a);
    double oldY=y; x=std::abs((tx-oldY)*c); y=std::abs((tx+oldY)*c);
    tx=(x+y)*c; y=(-x+y)*c; x=tx*a+z*b; z=-tx*b+z*a;
    x=x*scale-offset; y*=scale; z*=scale;
    double xx=x*x,yy=y*y,zz=z*z,t=2*x/std::sqrt(yy+zz);
    double newY=t*2*y*z; x=xx-yy-zz+it->J1; z=t*(yy-zz); y=newY;
}
static void nativeJitTrig(double& x,double&,double&,double&,TPIteration3D it) {x=std::sinh(x*nativeOption(it,0));}

// ============================================================================
// FormulaParamMap definitions — byte offsets from PVar for each param
// ============================================================================

// 1. _Folding1: 1 double param at [PVar-0x10]
static const FormulaParamMap map_Folding1 = {
    1, {{ -0x10, 0 }}
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FOLDING1, -1, map_Folding1)

// 2. _AbsX: 1 double param at [PVar-0x10]
static const FormulaParamMap map_AbsX = {
    1, {{ -0x10, 0 }}
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ABSX, -1, map_AbsX)

// 3. BristorBrot: 3 doubles, sequential
static const FormulaParamMap map_BristorBrot = {
    3, {{ -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }}
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BRISTORBROT, 0, map_BristorBrot)

// 4. Benesi1Pow2: no PVar params — C passed via AppendCValues (J1,J2,J3)
static const FormulaParamMap map_Benesi1Pow2 = { 0, {} };
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BENESI1POW2, 0, map_Benesi1Pow2)

// 5. _FlipXYc: 1 integer param at [PVar-0x0C]
static const FormulaParamMap map_FlipXYc = {
    1, {{ -0x0C, 2 }}
};
template<>
struct ExternalFormulaUsesWrittenCForSameIterationAdd<FORMULA__FLIPXYC> {
    static constexpr bool value = true;
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__FLIPXYC, -1, map_FlipXYc)

// 6. _AmazingBoxSSE2: Scale/MinR2 at -0x18, MinR2 at -0x20, Fold at -0x30
static const FormulaParamMap map_AmazingBoxSSE2 = {
    4, {{ -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x10, 0 }}
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__AMAZINGBOXSSE2, 11, map_AmazingBoxSSE2)

// 7. _AmazingBox: retain the original Scale independently of Scale/MinR2.
static const FormulaParamMap map_AmazingBox = {
    3, {{ -0x18, 0 }, { -0x20, 0 }, { -0x10, 0 }}
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__AMAZINGBOX, 2, map_AmazingBox)

// 8. ABoxMod1: Scale=-0x10, Scale/MinR^2=-0x18, MinR^2=-0x20, Fold=-0x28,
//    ScaleVary=-0x30, FoldXMod=-0x38, FoldYMod=-0x40, FoldZMod=-0x48
static const FormulaParamMap map_ABoxMod1 = {
    8, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x30, 0 }, { -0x38, 0 }, { -0x40, 0 }, { -0x48, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXMOD1, 2, map_ABoxMod1)

// 9. MengerIFS: Scale=-0x10, CScaleX=-0x18, CScaleY=-0x20, CScaleZ=-0x28,
//    Pre-computed rotation matrix: 9 floats at -0x2C..-0x4C (row-major, from .3SingleAngles)
static const FormulaParamMap map_MengerIFS = {
    13, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },  // rotation matrix row 0
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 },  // rotation matrix row 1
        { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 }   // rotation matrix row 2
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_MENGERIFS, 2, map_MengerIFS)

// 10. CommQuat: 5 doubles at -0x10..-0x30, C from AppendCValues
static const FormulaParamMap map_CommQuat = {
    5, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 }, { -0x30, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_COMMQUAT, 4, map_CommQuat)

// 11. _Rotate: 9 pre-computed float matrix elements at -0x0C..-0x2C
//    Pascal FillCustomVBufWithVars stores the rotation matrix, not raw angles
static const FormulaParamMap map_Rotate = {
    9, {
        { -0x0C, 1 }, { -0x10, 1 }, { -0x14, 1 },  // row 0
        { -0x18, 1 }, { -0x1C, 1 }, { -0x20, 1 },  // row 1
        { -0x24, 1 }, { -0x28, 1 }, { -0x2C, 1 }   // row 2
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA__ROTATE, -1, map_Rotate)

// 12. boxIFS: 13 params (doubles, floats, ints) per PVar layout
static const FormulaParamMap map_boxIFS = {
    13, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x30, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x50, 0 }, { -0x58, 0 },
        { -0x5C, 1 }, { -0x60, 2 }, { -0x64, 2 }, { -0x68, 1 }, { -0x6C, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_BOXIFS, 20, map_boxIFS)

// 13. gearIFS: 12 options followed by the three original constant slots.
static const FormulaParamMap map_gearIFS = {
    15, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x48, 0 }, { -0x4C, 2 }, { -0x54, 0 },
        { -0x5C, 0 }, { -0x64, 0 }, { -0x6C, 0 }, { -0x74, 0 },
        { 0x00, 0 }, { 0x08, 0 }, { 0x10, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_GEARIFS, 20, map_gearIFS)

// 14. SphereIFS: 13 params per PVar layout
static const FormulaParamMap map_SphereIFS = {
    13, {
        { -0x10, 0 }, { -0x20, 0 }, { -0x30, 0 }, { -0x38, 0 },
        { -0x40, 0 }, { -0x50, 0 }, { -0x58, 0 }, { -0x60, 0 },
        { -0x64, 2 }, { -0x68, 2 }, { -0x6C, 1 }, { -0x70, 1 }, { -0x78, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SPHEREIFS, 20, map_SphereIFS)

// 15. Sierpinski3: Scale + 3 CScale + 2 pre-computed rotation matrices (9+9 floats)
static const FormulaParamMap map_Sierpinski3 = {
    22, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 },
        // Rotation1 matrix: 9 floats at -0x2C..-0x4C
        { -0x2C, 1 }, { -0x30, 1 }, { -0x34, 1 },
        { -0x38, 1 }, { -0x3C, 1 }, { -0x40, 1 },
        { -0x44, 1 }, { -0x48, 1 }, { -0x4C, 1 },
        // Rotation2 matrix: 9 floats at -0x50..-0x70
        { -0x50, 1 }, { -0x54, 1 }, { -0x58, 1 },
        { -0x5C, 1 }, { -0x60, 1 }, { -0x64, 1 },
        { -0x68, 1 }, { -0x6C, 1 }, { -0x70, 1 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_SIERPINSKI3, 2, map_Sierpinski3)

// 17. ABoxVaryScale: Scale/MinR2=-0x18, MinR2=-0x20, Fold=-0x28,
//     Scale_vary=-0x30, R_power=-0x38
static const FormulaParamMap map_ABoxVaryScale = {
    6, {
        { -0x18, 0 }, { -0x20, 0 }, { -0x28, 0 }, { -0x30, 0 }, { -0x38, 0 }, { -0x10, 0 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_ABOXVARYSCALE, 11, map_ABoxVaryScale)

// 18. GeneralQuat: 8 int32 params at -0x0C..-0x28
static const FormulaParamMap map_GeneralQuat = {
    8, {
        { -0x0C, 2 }, { -0x10, 2 }, { -0x14, 2 }, { -0x18, 2 },
        { -0x1C, 2 }, { -0x20, 2 }, { -0x24, 2 }, { -0x28, 2 }
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_GENERALQUAT, 4, map_GeneralQuat)

// 19. transformIFS: AddX=-0x10, AddY=-0x18, AddZ=-0x20,
//     Pre-computed rotation matrix: 9 floats at -0x24..-0x44 (row-major, from .3SingleAngles)
//     AbsX=-0x48(int), AbsY=-0x4C(int), AbsZ=-0x50(int), Scale=-0x58
static const FormulaParamMap map_transformIFS = {
    16, {
        { -0x10, 0 }, { -0x18, 0 }, { -0x20, 0 },
        { -0x24, 1 }, { -0x28, 1 }, { -0x2C, 1 },  // rotation matrix row 0
        { -0x30, 1 }, { -0x34, 1 }, { -0x38, 1 },  // rotation matrix row 1
        { -0x3C, 1 }, { -0x40, 1 }, { -0x44, 1 },  // rotation matrix row 2
        { -0x48, 2 }, { -0x4C, 2 }, { -0x50, 2 },  // Abs flags
        { -0x58, 0 }                                 // Scale
    }
};
DEFINE_EXTERNAL_FORMULA_WRAPPER(FORMULA_TRANSFORMIFS, 21, map_transformIFS)

// ============================================================================
// Registry initialization — called from InitExternalFormulaRegistry
// ============================================================================

void RegisterExternalFormulaTable() {
    // 1. _Folding1: .Double
    RegisterExternalFormula({
        FORMULA__FOLDING1, "_Folding1", -1, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__FOLDING1, &map_Folding1,
        1, {0},
        0,
        0, nullptr
    });

    // 2. _AbsX: .Double
    RegisterExternalFormula({
        FORMULA__ABSX, "_AbsX", -1, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__ABSX, &map_AbsX,
        1, {0},
        0,
        0, nullptr
    });

    // 3. BristorBrot: 3 doubles
    RegisterExternalFormula({
        FORMULA_BRISTORBROT, "BristorBrot", 0, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BRISTORBROT, &map_BristorBrot,
        3, {0, 0, 0},
        0,
        0, nullptr
    });

    // 4. Benesi1Pow2: no params
    RegisterExternalFormula({
        FORMULA_BENESI1POW2, "Benesi1Pow2", 0, 3,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BENESI1POW2, &map_Benesi1Pow2,
        0, {},
        0,
        0, nullptr
    });

    // 5. _FlipXYc: .Integer
    RegisterExternalFormula({
        FORMULA__FLIPXYC, "_FlipXYc", -1, 1,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__FLIPXYC, &map_FlipXYc,
        1, {2},
        0,
        0, nullptr
    });

    // 6. _AmazingBoxSSE2: Double, Boxscale, Folding16
    static const FormulaConstEntry consts_audit__AmazingBoxSSE2[] = {
        { 0, UINT64_C(0x3FF0000000000000) },
        { 0, UINT64_C(0x3FF0000000000000) },
        { 0, UINT64_C(0xBFF0000000000000) },
        { 0, UINT64_C(0xBFF0000000000000) },
    };
    RegisterExternalFormula({
        FORMULA__AMAZINGBOXSSE2, "_AmazingBoxSSE2", 11, 4,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA__AMAZINGBOXSSE2, &map_AmazingBoxSSE2,
        3, {0, 7, 11},
        0,
        4, consts_audit__AmazingBoxSSE2 });

    // 7. _AmazingBox: Double, Boxscale
    RegisterExternalFormula({
        FORMULA__AMAZINGBOX, "_AmazingBox", 2, 3,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA__AMAZINGBOX, &map_AmazingBox,
        2, {0, 7},
        0,
        0, nullptr
    });

    // 8. ABoxMod1: Double, Boxscale, Double x5
    RegisterExternalFormula({
        FORMULA_ABOXMOD1, "ABoxMod1", 2, 8,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXMOD1, &map_ABoxMod1,
        7, {0, 7, 0, 0, 0, 0, 0},
        0,
        0, nullptr
    });

    // 9. MengerIFS: Double x4, 3SingleAngles
    RegisterExternalFormula({
        FORMULA_MENGERIFS, "MengerIFS", 2, 13,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_MENGERIFS, &map_MengerIFS,
        5, {0, 0, 0, 0, 6},
        0,
        0, nullptr
    });

    // 10. CommQuat: 5 doubles
    RegisterExternalFormula({
        FORMULA_COMMQUAT, "CommQuat", 4, 5,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_COMMQUAT, &map_CommQuat,
        5, {0, 0, 0, 0, 0},
        0,
        0, nullptr
    });

    // 11. _Rotate: 3SingleAngles
    RegisterExternalFormula({
        FORMULA__ROTATE, "_Rotate", -1, 9,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA__ROTATE, &map_Rotate,
        1, {6},
        0,
        0, nullptr
    });

    // 12. boxIFS: Double x3, 2Doubles x2, Double x3, Single, Integer x2, Single x2
    static const FormulaConstEntry consts_audit_boxIFS[] = {
        { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
        { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
    };
    RegisterExternalFormula({
        FORMULA_BOXIFS, "boxIFS", 20, 13,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_BOXIFS, &map_boxIFS,
        13, {0, 0, 0, 14, 14, 0, 0, 0, 1, 2, 2, 1, 1},
        0,
        2, consts_audit_boxIFS });

    // 13. gearIFS: Double, 2Doubles x2, Double x4, Integer, Double x4
    // .m3f [CONSTANTS]: 360/pi=114.591559, pi/360=0.008726646, 180.0
    static const FormulaConstEntry consts_gearIFS[] = {
        { 0, UINT64_C(0x405CA5DC1A47A9E3) },  // 114.591559 (360/pi)
        { 0, UINT64_C(0x3F81DF469963E11D) },  // 0.008726646 (pi/360)
        { 0, UINT64_C(0x4066800000000000) },  // 180.0
    };
    RegisterExternalFormula({
        FORMULA_GEARIFS, "gearIFS", 20, 15,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_GEARIFS, &map_gearIFS,
        12, {0, 14, 14, 0, 0, 0, 2, 0, 0, 0, 0, 0},
        0,
        3, consts_gearIFS
    });

    // 14. SphereIFS: Double, 2Doubles x2, Double, 2Doubles, DRecipro x3, Integer x2, Single x2, Double
    static const FormulaConstEntry consts_audit_SphereIFS[] = {
        { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
        { 3, UINT64_C(0x7FFFFFFFFFFFFFFF) },
    };
    RegisterExternalFormula({
        FORMULA_SPHEREIFS, "SphereIFS", 20, 13,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_SPHEREIFS, &map_SphereIFS,
        13, {0, 14, 14, 0, 14, 13, 13, 13, 2, 2, 1, 1, 0},
        0,
        2, consts_audit_SphereIFS });

    // 15. Sierpinski3: Double x4, 3SingleAngles, 3SingleAngles
    RegisterExternalFormula({
        FORMULA_SIERPINSKI3, "Sierpinski3", 2, 22,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_SIERPINSKI3, &map_Sierpinski3,
        6, {0, 0, 0, 0, 6, 6},
        0,
        0, nullptr
    });

    // 17. ABoxVaryScale: Double, Boxscale, Double x3
    RegisterExternalFormula({
        FORMULA_ABOXVARYSCALE, "ABoxVaryScale", 11, 6,
        0.2, 1.0, 2.0, 1024.0,
        ExternWrap_FORMULA_ABOXVARYSCALE, &map_ABoxVaryScale,
        5, {0, 7, 0, 0, 0},
        0,
        0, nullptr
    });

    // 18. GeneralQuat: 8 integers
    RegisterExternalFormula({
        FORMULA_GENERALQUAT, "GeneralQuat", 4, 8,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_GENERALQUAT, &map_GeneralQuat,
        8, {2, 2, 2, 2, 2, 2, 2, 2},
        0,
        0, nullptr
    });

    // 19. transformIFS: Double x3, 3SingleAngles, Integer x3, Double
    RegisterExternalFormula({
        FORMULA_TRANSFORMIFS, "transformIFS", 21, 16,
        1.0, 1.0, 2.0, 100.0,
        ExternWrap_FORMULA_TRANSFORMIFS, &map_transformIFS,
        8, {0, 0, 0, 6, 2, 2, 2, 0},
        0,
        0, nullptr
    });

    static const FormulaConstEntry consts_audit__BenesiT1[] = {
        { 0, UINT64_C(0x3FEA20BD700C2C3E) },
        { 0, UINT64_C(0x3FE279A74590331D) },
        { 0, UINT64_C(0x3FE6A09E667F3BCC) },
    };
    RegisterExternalFormula({ FORMULA_BENESIT1_NATIVE, "_BenesiT1", -1, 2, 1.0, 1.0, 2.0, 100.0, nativeBenesi1, nullptr, 2, {0}, 0, 3, consts_audit__BenesiT1 });
    static const FormulaConstEntry consts_audit__BenesiT2[] = {
        { 0, UINT64_C(0x3FEA20BD700C2C3E) },
        { 0, UINT64_C(0x3FE279A74590331D) },
        { 0, UINT64_C(0x3FE6A09E667F3BCC) },
    };
    RegisterExternalFormula({ FORMULA_BENESIT2_NATIVE, "_BenesiT2", -1, 2, 1.0, 1.0, 2.0, 100.0, nativeBenesi2, nullptr, 2, {0}, 0, 3, consts_audit__BenesiT2 });
    static const FormulaConstEntry consts_audit__BenesiT3[] = {
        { 0, UINT64_C(0x3FEA20BD700C2C3E) },
        { 0, UINT64_C(0x3FE279A74590331D) },
        { 0, UINT64_C(0x3FE6A09E667F3BCC) },
    };
    RegisterExternalFormula({ FORMULA_BENESIT3_NATIVE, "_BenesiT3", -1, 2, 1.0, 1.0, 2.0, 100.0, nativeBenesi3, nullptr, 2, {0}, 0, 3, consts_audit__BenesiT3 });
    RegisterExternalFormula({ FORMULA_JIT_IQ_NATIVE, "JIT_IQ_Bulb", 0, 1, 1.0, 1.0, 2.0, 100.0, nativeJitIQ, nullptr, 1, {0}, 0, 0, nullptr });
    static const FormulaConstEntry consts_audit_JITBenesiPineTree[] = {
        { 0, UINT64_C(0x3FE6A09E667F3BD1) },
        { 0, UINT64_C(0x3FE279A74590331E) },
        { 0, UINT64_C(0x3FEA20BD700C2C3E) },
    };
    RegisterExternalFormula({ FORMULA_JIT_PINE_NATIVE, "JITBenesiPineTree", 0, 2, 1.0, 1.0, 2.0, 100.0, nativeJitPine, nullptr, 2, {0}, 0, 3, consts_audit_JITBenesiPineTree });
    RegisterExternalFormula({ FORMULA_JIT_TRIG_NATIVE, "JITTrigExample", 0, 1, 1.0, 1.0, 2.0, 100.0, nativeJitTrig, nullptr, 1, {0}, 0, 0, nullptr });

    // Bulk-generated formula registrations (435 formulas across 7 categories)
    RegisterExternalFormulas_3D();
    RegisterExternalFormulas_3Da();
    RegisterExternalFormulas_4D();
    RegisterExternalFormulas_4Da();
    RegisterExternalFormulas_Ads();
    RegisterExternalFormulas_dIFS();
    RegisterExternalFormulas_dIFSa();
}

} // namespace mb3d

#undef constant
