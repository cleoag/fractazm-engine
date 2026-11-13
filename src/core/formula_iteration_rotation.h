#pragma once
#include "formula_counter.h"

// Shared by the two identical shipped aliases and their power-law variant.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void IterationRotation_x87(double* xyz, const double* params,
                                      double counter, int powerLaw);
#endif

inline cpu_double3 formulaIterationRotation(cpu_double3 p, double* params, bool powerLaw) {
    const int32_t count = incrementFormulaCounter(params);
    double xyz[] = {p.x, p.y, p.z};
#ifdef MB3D_SOFTWARE_EXT80
    if(!powerLaw) {
        using namespace mb3d::numeric;
        using E=Extended80;
        ExtendedScope scope;
        for(int row=0;row<3;++row) {
            const double* m=params+row*3;
            xyz[row]=double((E(p.x)*float(m[0])+E(p.y)*float(m[1]))+E(p.z)*float(m[2]));
        }
        const E angle=(E(count)*params[9])*params[10];
        const auto sc=extendedSinCos(angle);
        const E x=xyz[0],y=xyz[1];
        // Missing FSINCOS push exhausts both rotation outputs in this caller.
        const bool missing=!mb3d::NativePrecisionEnabled() &&
            abs(angle)>=E::fromBits(16383+63,0x8000000000000000ULL);
        const E rx=missing ? extendedNaN() : E(double(sc.cos*x+sc.sin*y));
        const E ry=missing ? extendedNaN() : E(double(sc.cos*y-sc.sin*x));
        const E z=xyz[2];
        for(int col=0;col<3;++col)
            xyz[col]=double((rx*float(params[col])+ry*float(params[3+col]))+z*float(params[6+col]));
        return cpu_double3(xyz[0],xyz[1],xyz[2]);
    }
#endif
#if defined(_MSC_VER) && defined(_M_X64)
    IterationRotation_x87(xyz, params, static_cast<double>(count), powerLaw ? 1 : 0);
#else
    for (int row = 0; row < 3; ++row) {
        const double* m = params + row * 3;
        xyz[row] = double((static_cast<long double>(p.x) * m[0]
                        + static_cast<long double>(p.y) * m[1])
                        + static_cast<long double>(p.z) * m[2]);
    }
    long double angle = count;
    if (powerLaw) angle = pow(fabs(angle - params[10]), static_cast<long double>(params[11]));
    angle = (angle * params[9]) * params[powerLaw ? 12 : 10];
    const long double c = cos(angle), s = sin(angle);
    const double x = double(xyz[0] * c + xyz[1] * s);
    const double y = double(xyz[1] * c - xyz[0] * s);
    for (int col = 0; col < 3; ++col)
        xyz[col] = double((static_cast<long double>(x) * params[col]
                         + static_cast<long double>(y) * params[3 + col])
                         + static_cast<long double>(xyz[2]) * params[6 + col]);
#endif
    return cpu_double3(xyz[0], xyz[1], xyz[2]);
}
