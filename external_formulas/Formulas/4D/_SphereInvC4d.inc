// _SphereInvC4d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SphereInvC4d
// Hand-ported from ASM: 122 bytes, DEoption=4

constexpr unsigned int FORMULA__SPHEREINVC4D = 8563;

// ASM trace: Same as _SphereInvC but includes w component in both inversions.
// Part 1: C = C / |C|² (Cx,Cy,Cz,Cw all divided by Cx²+Cy²+Cz²+Cw²)
// Part 2: p = p / |p|² (x,y,z,w all divided by x²+y²+z²+w²)

template<>
cpu_double3 execute_formula<FORMULA__SPHEREINVC4D>(cpu_double3 p, constant double* params, double &de) {
    const double cx = params[0];
    const double cy = params[1];
    const double cz = params[2];
    const double cw = params[3];

    double c_sq = cx * cx + cy * cy + cz * cz + cw * cw;
    if (c_sq > 0.0) {
        double c_inv = 1.0 / c_sq;
        params[WRITE_J1_SLOT] = cx * c_inv;
        params[WRITE_J2_SLOT] = cy * c_inv;
        params[WRITE_J3_SLOT] = cz * c_inv;
        params[WRITE_J4_SLOT] = cw * c_inv;
    }

    double w = de;
    double r_sq = p.x * p.x + p.y * p.y + p.z * p.z + w * w;
    double inv = 1.0 / r_sq;
    de = w * inv;
    return p * inv;
}

// Alternative direct function implementation
cpu_double3 _SphereInvC4d_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__SPHEREINVC4D>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__SphereInvC4d = {
    .id = FORMULA__SPHEREINVC4D,
    .name = "_SphereInvC4d",
    .param_count = 0,
    .complexity = 122,
    .uses_transcendental = false
};

/*
Original M3 Description:
4D sphere inversion (Kelvin transform) of both position and Julia constants.

Mathematical Formula:
rSqrL = 1 / (Cx²+Cy²+Cz²+Cw²)
C' = C * rSqrL
rSqrL = 1 / (x²+y²+z²+w²)
p' = p * rSqrL

Parameters:
  (none)
*/
