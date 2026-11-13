// _SphereInvC.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SphereInvC
// Hand-ported from ASM: 94 bytes, DEoption=-1

constexpr unsigned int FORMULA__SPHEREINVC = 7922;

// ASM trace:
//   Part 1: Invert Julia constants C
//     Load Cx, Cy, Cz from [esi+0x18/0x20/0x28]
//     |C|² = Cx² + Cy² + Cz²
//     inv = 1.0 / |C|²
//     Cx *= inv, Cy *= inv, Cz *= inv → C/|C|²
//     Store back
//
//   Part 2: Invert position p
//     Load x, y, z from [eax/edx/ecx]
//     |p|² = x² + y² + z²
//     inv = 1.0 / |p|²
//     x *= inv, y *= inv, z *= inv → p/|p|²
//     Store back
//
// Kelvin inversion (spherical inversion) of both C and p.

template<>
cpu_double3 execute_formula<FORMULA__SPHEREINVC>(cpu_double3 p, constant double* params, double &de) {
    double c_sq = params[0] * params[0] + params[1] * params[1] + params[2] * params[2];
    if (c_sq > 0.0) {
        double c_inv = 1.0 / c_sq;
        params[WRITE_J1_SLOT] = params[0] * c_inv;
        params[WRITE_J2_SLOT] = params[1] * c_inv;
        params[WRITE_J3_SLOT] = params[2] * c_inv;
    }

    // Part 2: Invert position
    double r_sq = p.x * p.x + p.y * p.y + p.z * p.z;
    double inv = 1.0 / r_sq;

    return p * inv;
}

// Alternative direct function implementation
cpu_double3 _SphereInvC_formula(cpu_double3 p, constant double* params, double &de) {
    double c_sq = params[0] * params[0] + params[1] * params[1] + params[2] * params[2];
    if (c_sq > 0.0) {
        double c_inv = 1.0 / c_sq;
        params[WRITE_J1_SLOT] = params[0] * c_inv;
        params[WRITE_J2_SLOT] = params[1] * c_inv;
        params[WRITE_J3_SLOT] = params[2] * c_inv;
    }
    double r_sq = p.x * p.x + p.y * p.y + p.z * p.z;
    double inv = 1.0 / r_sq;
    return p * inv;
}

// Formula metadata
constant FormulaInfo formula_info__SphereInvC = {
    .id = FORMULA__SPHEREINVC,
    .name = "_SphereInvC",
    .param_count = 0,
    .complexity = 94,
    .uses_transcendental = false
};

/*
Original M3 Description:
Complete sphere inversion (Kelvin transform) of both position and Julia constants.

Mathematical Formula:
rSqrL = 1 / (Cx²+Cy²+Cz²)
Cx' = Cx * rSqrL, Cy' = Cy * rSqrL, Cz' = Cz * rSqrL

rSqrL = 1 / (x²+y²+z²)
x' = x * rSqrL, y' = y * rSqrL, z' = z * rSqrL

Parameters:
  (none)
*/
