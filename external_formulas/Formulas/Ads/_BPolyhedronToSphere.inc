// _BPolyhedronToSphere.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BPolyhedronToSphere
// Hand-ported from ASM: 158 bytes, DEoption=-1

constexpr unsigned int FORMULA__BPOLYHEDRONTOSPHERE = 7095;
#include "extended_polygon_sector.h"

// ASM trace:
//   halfA = pi / Order1
//   halfB = pi / Order2
//   r_yz = sqrt(y² + z²)
//   angle_xr = |atan2(r_yz, x)|
//   while (angle_xr > halfB): angle_xr -= 2*halfB
//   inv_cos_b = 1.0 / cos(angle_xr)
//   x' = inv_cos_b * x,  y' = inv_cos_b * y,  z' = inv_cos_b * z
//   angle_yz = |atan2(z', y')|      (using scaled y', z')
//   while (angle_yz > halfA): angle_yz -= 2*halfA
//   inv_cos_a = 1.0 / cos(angle_yz)
//   new_y = inv_cos_a * y',  new_z = inv_cos_a * z'
//   new_x = x'  (already set)
//
// Inverse of _BPolyhedronFromSphere: maps polyhedron back to sphere.
// Uses 1/cos instead of cos to undo the polygon folding.
// Order of operations is reversed (xr first, then yz).
//
// C handling: Transform (DEoption=-1), no C involvement.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BPolyhedronToSphereOriginal_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__BPOLYHEDRONTOSPHERE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[9] = {p.x,p.y,p.z};
    BPolyhedronToSphereOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    FormulaWorkBudget budget("_BPolyhedronToSphere");
    const E a=E(params[2])/params[0], b=E(params[2])/params[1];
    const E x=p.x,y=p.y,z=p.z;
    E angle=extendedPolygonSector(abs(extendedAtan2(sqrt(z*z+y*y),x)),
                                 double(b),double(b+b),budget);
    const E factor=E(1)/extendedSinCos(angle).cos;
    const double ox=double(factor*x), sy=double(factor*y), sz=double(factor*z);
    // The Z FST retains its extended value as the next FPATAN input.
    angle=extendedPolygonSector(abs(extendedAtan2(factor*z,E(sy))),
                                double(a),double(a+a),budget);
    const E yz=E(1)/extendedSinCos(angle).cos;
    return cpu_double3(ox,double(yz*sy),double(yz*sz));
#else

    const double Order1 = params[0];  // polygon order for yz-plane (default: 4)
    const double Order2 = params[1];  // polygon order for xr-plane (default: 4)

    const double PI = 3.14159265358979;

    double x = p.x, y = p.y, z = p.z;

    // Half-angles for polygon sectors
    double halfA = PI / Order1;
    double doubleA = 2.0 * halfA;
    double halfB = PI / Order2;
    double doubleB = 2.0 * halfB;

    // First: undo xr-plane folding (inverse order from FromSphere)
    double r_yz = sqrt(y * y + z * z);
    double angle_xr = abs(atan2(r_yz, x));
    if (doubleB > 1e-30) {
        angle_xr = fmod(angle_xr, doubleB);
        if (angle_xr > halfB) {
            angle_xr -= doubleB;
        }
    }
    double cos_b = cos(angle_xr);
    double inv_cos_b = (abs(cos_b) > 1e-30) ? (1.0 / cos_b) : 0.0;

    // Scale all coordinates by inverse cosine
    double xs = inv_cos_b * x;
    double ys = inv_cos_b * y;
    double zs = inv_cos_b * z;

    // Second: undo yz-plane folding (using scaled coords)
    double angle_yz = abs(atan2(zs, ys));
    if (doubleA > 1e-30) {
        angle_yz = fmod(angle_yz, doubleA);
        if (angle_yz > halfA) {
            angle_yz -= doubleA;
        }
    }
    double cos_a = cos(angle_yz);
    double inv_cos_a = (abs(cos_a) > 1e-30) ? (1.0 / cos_a) : 0.0;

    // Apply inverse cosine to y,z only
    double new_x = xs;
    double new_y = inv_cos_a * ys;
    double new_z = inv_cos_a * zs;

    return cpu_double3(new_x, new_y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__BPolyhedronToSphere = {
    .id = FORMULA__BPOLYHEDRONTOSPHERE,
    .name = "_BPolyhedronToSphere",
    .param_count = 2,
    .complexity = 158,
    .uses_transcendental = true
};

/*
Original M3 Description:
Maps polyhedron to sphere by Luca GN.
Inverse of _BPolyhedronFromSphere.

Uses 1/cos instead of cos to expand polygonal geometry
back to spherical. Operations applied in reverse order:
1. Undo xr-plane folding (scale all by 1/cos_b)
2. Undo yz-plane folding (scale y,z by 1/cos_a)

The ASM uses a loop for modular reduction; Metal uses fmod.

Parameters:
  [0] Order1: double (yz-plane polygon order, default: 4)
  [1] Order2: double (xr-plane polygon order, default: 4)

Constants:
  pi at [PVar+0x00]
*/
