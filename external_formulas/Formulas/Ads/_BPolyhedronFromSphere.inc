// _BPolyhedronFromSphere.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _BPolyhedronFromSphere
// Hand-ported from ASM: 157 bytes, DEoption=-1

constexpr unsigned int FORMULA__BPOLYHEDRONFROMSPHERE = 7094;

// ASM trace:
//   halfA = pi / Order1
//   halfB = pi / Order2
//   angle_yz = |atan2(z, y)|
//   while (angle_yz > halfA): angle_yz -= 2*halfA   (reduce to [-halfA, halfA])
//   cos_a = cos(angle_yz)
//   proj_y = cos_a * y,  proj_z = cos_a * z
//   r_yz = sqrt(proj_y² + proj_z²)
//   angle_xr = |atan2(r_yz, x)|
//   while (angle_xr > halfB): angle_xr -= 2*halfB   (reduce to [-halfB, halfB])
//   cos_b = cos(angle_xr)
//   new_x = cos_b * x,  new_y = cos_b * proj_y,  new_z = cos_b * proj_z
//
// Maps sphere to polyhedron by folding angles into polygon sectors
// and applying cosine correction. Creates polygonal cross-sections
// from spherical geometry.
//
// C handling: Transform (DEoption=-1), no C involvement.

template<>
cpu_double3 execute_formula<FORMULA__BPOLYHEDRONFROMSPHERE>(cpu_double3 p, constant double* params, double &de) {
    const double Order1 = params[0];  // polygon order for yz-plane (default: 4)
    const double Order2 = params[1];  // polygon order for xr-plane (default: 4)

    // Constants from m3: pi stored at [PVar+0x00]
    const double PI = 3.14159265358979;

    double x = p.x, y = p.y, z = p.z;

    // Half-angles for polygon sectors
    double halfA = PI / Order1;
    double doubleA = 2.0 * halfA;
    double halfB = PI / Order2;
    double doubleB = 2.0 * halfB;

    // First fold: yz-plane angle reduction
    double angle_yz = abs(atan2(z, y));
    // Reduce angle to [0, halfA] via modular arithmetic
    if (doubleA > 1e-30) {
        angle_yz = fmod(angle_yz, doubleA);
        if (angle_yz > halfA) {
            angle_yz -= doubleA;
        }
    }
    double cos_a = cos(angle_yz);

    // Project y,z through cosine correction
    double proj_y = cos_a * y;
    double proj_z = cos_a * z;
    double r_yz = sqrt(proj_y * proj_y + proj_z * proj_z);

    // Second fold: x vs yz-radius angle reduction
    double angle_xr = abs(atan2(r_yz, x));
    if (doubleB > 1e-30) {
        angle_xr = fmod(angle_xr, doubleB);
        if (angle_xr > halfB) {
            angle_xr -= doubleB;
        }
    }
    double cos_b = cos(angle_xr);

    // Apply second cosine correction to all coordinates
    double new_x = cos_b * x;
    double new_y = cos_b * proj_y;
    double new_z = cos_b * proj_z;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__BPolyhedronFromSphere = {
    .id = FORMULA__BPOLYHEDRONFROMSPHERE,
    .name = "_BPolyhedronFromSphere",
    .param_count = 2,
    .complexity = 157,
    .uses_transcendental = true
};

/*
Original M3 Description:
Maps sphere to polyhedron by Luca GN.
Applies angular folding with cosine correction to create
polygonal cross-sections from spherical geometry.

Step 1: Fold yz-angle into polygon sector
  angle = |atan2(z, y)| mod (2*pi/Order1)
  Apply cos correction to y,z

Step 2: Fold x-vs-radius angle into polygon sector
  angle = |atan2(r_yz, x)| mod (2*pi/Order2)
  Apply cos correction to all

The ASM uses a loop for modular reduction; Metal uses fmod.

Parameters:
  [0] Order1: double (yz-plane polygon order, default: 4)
  [1] Order2: double (xr-plane polygon order, default: 4)

Constants:
  pi at [PVar+0x00]
*/
