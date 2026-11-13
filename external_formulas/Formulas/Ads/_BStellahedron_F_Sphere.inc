// _BStellahedron_F_Sphere.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BStellahedron_F_Sphere
// Hand-ported from ASM: 221 bytes, DEoption=-1
// Stellahedron folding + spherical projection — maps points into polyhedral sectors.

constexpr unsigned int FORMULA__BSTELLAHEDRON_F_SPHERE = 173;
#include "extended_polygon_sector.h"

// ASM trace (221 bytes, 2 modular reduction loops + final scaling):
//
// Precompute (from parameters):
//   half1 = A / Scale1;  full1 = 2 * half1
//   ratio1 = cos(half1 * B) / cos(half1 * B - Offset1)
//   half2 = A / Scale2;  full2 = 2 * half2
//   ratio2 = cos(half2 * B) / cos(half2 * B - Offset2)
//
// Step 1: yz-plane folding
//   angle1 = |atan2(z, y)|
//   while angle1 > half1:  angle1 -= full1   (modular reduction)
//   scale_yz = cos(|angle1| - Offset1) * ratio1
//
// Step 2: Compute projected radius
//   sy = scale_yz * y,  sz = scale_yz * z
//   r_yz = sqrt(sy² + sz²)
//
// Step 3: x-vs-yz plane folding
//   angle2 = |atan2(r_yz, x)|
//   while angle2 > half2:  angle2 -= full2   (modular reduction)
//   scale_xr = cos(|angle2| - Offset2) * ratio2
//
// Step 4: Final output
//   x' = scale_xr * x
//   y' = scale_xr * scale_yz * y
//   z' = scale_xr * scale_yz * z
//
// C handling: Transform (DEoption=-1), no C.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BStellahedron_F_SphereOriginal_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__BSTELLAHEDRON_F_SPHERE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[9] = {p.x,p.y,p.z};
    BStellahedron_F_SphereOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    FormulaWorkBudget budget("_BStellahedron_F_Sphere");
    const E a=E(params[4])/params[0], b=E(params[4])/params[1];
    const double half1=double(a),step1=double(a+a),half2=double(b),step2=double(b+b);
    auto ratio = [&](double half,double offset) {
        const E base=E(half)*params[5];
        const double denominator=double(extendedSinCos(base-offset).cos);
        return double(extendedSinCos(base).cos/denominator);
    };
    const double ratio1=ratio(half1,params[2]),ratio2=ratio(half2,params[3]);
    E angle=extendedPolygonSector(abs(extendedAtan2(E(p.z),E(p.y))),half1,step1,budget);
    const E factor=extendedSinCos(abs(angle)-params[2]).cos*ratio1;
    // The Y multiplication retains factor; Z reloads its stored Double.
    const E sy=factor*p.y, sz=E(double(factor))*p.z;
    const double savedY=double(sy),savedZ=double(sz);
    angle=extendedPolygonSector(abs(extendedAtan2(sqrt(sy*sy+sz*sz),E(p.x))),half2,step2,budget);
    const E factor2=extendedSinCos(abs(angle)-params[3]).cos*ratio2;
    return cpu_double3(double(factor2*p.x),double(factor2*savedY),double(factor2*savedZ));
#else

    // [CONSTANTS] embedded as compile-time values (positive PVar offsets):
    const double A = 3.14159265358979;  // [edi+0x00] = π
    const double B = 0.5;              // [edi+0x08] = 0.5
    // [OPTIONS] parameters (negative PVar offsets):
    const double Scale1  = params[0];   // [edi-0x10] = Sides (default: 4.0)
    const double Scale2  = params[1];   // [edi-0x18] = Sides2 (default: 4.0)
    const double Offset1 = params[2];   // [edi-0x20] = StellAngle (default: 0.0)
    const double Offset2 = params[3];   // [edi-0x28] = StellAngle2 (default: 0.0)

    double x = p.x, y = p.y, z = p.z;

    // Precompute sector sizes
    double half1 = A / Scale1;
    double full1 = 2.0 * half1;
    double angle1_base = half1 * B;
    double cos1 = cos(angle1_base - Offset1);
    double ratio1 = (fabs(cos1) > 1e-30) ? (cos(angle1_base) / cos1) : 1.0;

    double half2 = A / Scale2;
    double full2 = 2.0 * half2;
    double angle2_base = half2 * B;
    double cos2 = cos(angle2_base - Offset2);
    double ratio2 = (fabs(cos2) > 1e-30) ? (cos(angle2_base) / cos2) : 1.0;

    // Step 1: yz-plane angular folding
    double angle1 = fabs(atan2(z, y));

    // Modular reduction: bring angle into sector
    for (int i = 0; i < 64 && angle1 > half1; i++) {
        angle1 -= full1;
    }

    double scale_yz = cos(fabs(angle1) - Offset1) * ratio1;

    // Step 2: Projected radius
    double sy = scale_yz * y;
    double sz = scale_yz * z;
    double r_yz = sqrt(sy * sy + sz * sz);

    // Step 3: x-vs-yz plane angular folding
    double angle2 = fabs(atan2(r_yz, x));

    for (int i = 0; i < 64 && angle2 > half2; i++) {
        angle2 -= full2;
    }

    double scale_xr = cos(fabs(angle2) - Offset2) * ratio2;

    // Step 4: Final output
    double x_out = scale_xr * x;
    double y_out = scale_xr * sy;
    double z_out = scale_xr * sz;

    return cpu_double3(x_out, y_out, z_out);
#endif
}

// Formula metadata
constant FormulaInfo formula_info__BStellahedron_F_Sphere = {
    .id = FORMULA__BSTELLAHEDRON_F_SPHERE,
    .name = "_BStellahedron_F_Sphere",
    .param_count = 4,
    .complexity = 221,
    .uses_transcendental = true
};

/*
Original M3 Description:
Stellahedron folding transform with spherical projection.
Maps 3D space into polyhedral sectors by angular folding in two planes:
1. Fold in yz-plane (azimuthal symmetry)
2. Fold in x-vs-yz plane (polar symmetry)

Each fold reduces the angle to a sector, then applies cosine-based
distance scaling to create the polyhedron shape.

The ratio = cos(base)/cos(base-offset) compensates for the angular
offset, ensuring continuous distance at sector boundaries.

Parameters:
  [0] A: double — sector angle scale [edi]
  [1] B: double — angular multiplier for ratio [edi+0x08]
  [2] Scale1: double — number of sectors in yz-plane [edi-0x10]
  [3] Scale2: double — number of sectors in xr-plane [edi-0x18]
  [4] Offset1: double — angular offset 1 [edi-0x20]
  [5] Offset2: double — angular offset 2 [edi-0x28]
*/
