// _BPolygonToCircle.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BPolygonToCircle
// Hand-ported from ASM: 589 bytes, DEoption=-1

constexpr unsigned int FORMULA__BPOLYGONTOCIRCLE = 7042;
#include "extended_polygon_sector.h"

// ASM trace:
//   Constants: [edi+0] = pi = 3.14159265358980
//   Params: [edi-0x10] = Sides = 4.0
//
//   DD07/DC77F0/DD55F8   pi/Sides → local (also kept on stack)
//   D8C0/DD5DF0          2*pi/Sides → local
//   DD01/DD02/D9F3/D9E1  rCyz = abs(atan2(z, y))
//
//   Loop: DC55F8/DFE0/80E441/7F05  compare rCyz with pi/Sides; exit if <=
//         DC65F0/EBF1              rCyz -= 2*pi/Sides; jump back
//
//   D9FF                 cos(rCyz)
//   D9E8/DEF1            1/cos(rCyz)
//   D9C0/DC0A/DD1A       y' = y / cos(rCyz)
//   D9D0/DC09/DD19       z' = z / cos(rCyz)
//   x' unchanged
//
// Circle-to-polygon transform on yz plane by M. Benesi.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BPolygonToCircleOriginal_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__BPOLYGONTOCIRCLE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[9] = {p.x,p.y,p.z};
    BPolygonToCircleOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    FormulaWorkBudget budget("_BPolygonToCircle");
    const E sector=E(params[1])/params[0];
    const E angle=extendedPolygonSector(abs(extendedAtan2(E(p.z),E(p.y))),
                                       double(sector),double(sector+sector),budget);
    const E factor=E(1)/extendedSinCos(angle).cos;
    return cpu_double3(p.x,double(factor*p.y),double(factor*p.z));
#else

    const double Sides = params[0];  // default: 4.0

    double half_step = M_PI / Sides;
    double step = 2.0 * half_step;

    double rCyz = abs(atan2(p.z, p.y));

    // Reduce rCyz into (-half_step, half_step] range
    // ASM uses loop: while (rCyz > half_step) rCyz -= step;
    // Metal equivalent using fmod:
    rCyz = fmod(rCyz + half_step, step) - half_step;

    double scale = 1.0 / cos(rCyz);

    return cpu_double3(p.x, p.y * scale, p.z * scale);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__BPolygonToCircle = {
    .id = FORMULA__BPOLYGONTOCIRCLE,
    .name = "_BPolygonToCircle",
    .param_count = 1,
    .complexity = 95,
    .uses_transcendental = true
};

/*
Original M3 Description:
Circle to polygon transform centered on the x axis (transforms yz plane).
Made for use before Pine Tree Mandelbulb, after transform 1.

rCyz = abs(atan2(z, y))
while (rCyz > pi/Sides): rCyz -= 2*pi/Sides
y' = y / cos(rCyz)
z' = z / cos(rCyz)
x' unchanged

By M. Benesi.

Parameters:
  [0] Sides: double (default: 4.0)
*/
