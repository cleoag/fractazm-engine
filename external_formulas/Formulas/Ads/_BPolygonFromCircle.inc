// _BPolygonFromCircle.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BPolygonFromCircle
// Hand-ported from ASM: 347 bytes, DEoption=-1

constexpr unsigned int FORMULA__BPOLYGONFROMCIRCLE = 7050;
#include "extended_polygon_sector.h"

// ASM trace: Same as _BPolygonToCircle but multiplies by cos instead of 1/cos.
//   rCyz = abs(atan2(z, y))
//   while (rCyz > pi/Sides): rCyz -= 2*pi/Sides
//   D9FF       fcos(rCyz) → directly used as scale factor (NOT 1/cos)
//   D9C0/DC0A  y * cos(rCyz) → store y'
//   DC09       z * cos(rCyz) → store z'
//
// Polygon-to-circle transform (inverse of _BPolygonToCircle).

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BPolygonFromCircleOriginal_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__BPOLYGONFROMCIRCLE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[9] = {p.x,p.y,p.z};
    BPolygonFromCircleOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    FormulaWorkBudget budget("_BPolygonFromCircle");
    const E sector=E(params[1])/params[0];
    const E angle=extendedPolygonSector(abs(extendedAtan2(E(p.z),E(p.y))),
                                       double(sector),double(sector+sector),budget);
    const E factor=extendedSinCos(angle).cos;
    return cpu_double3(p.x,double(factor*p.y),double(factor*p.z));
#else

    const double Sides = params[0];  // default: 4.0

    double half_step = M_PI / Sides;
    double step = 2.0 * half_step;

    double rCyz = abs(atan2(p.z, p.y));

    // Reduce rCyz into (-half_step, half_step] range
    rCyz = fmod(rCyz + half_step, step) - half_step;

    double scale = cos(rCyz);

    return cpu_double3(p.x, p.y * scale, p.z * scale);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__BPolygonFromCircle = {
    .id = FORMULA__BPOLYGONFROMCIRCLE,
    .name = "_BPolygonFromCircle",
    .param_count = 1,
    .complexity = 95,
    .uses_transcendental = true
};

/*
Original M3 Description:
Polygon to circle transform centered on the x axis (transforms yz plane).
Inverse of _BPolygonToCircle.

rCyz = abs(atan2(z, y))
while (rCyz > pi/Sides): rCyz -= 2*pi/Sides
y' = y * cos(rCyz)
z' = z * cos(rCyz)
x' unchanged

By M. Benesi.

Parameters:
  [0] Sides: double (default: 4.0)
*/
