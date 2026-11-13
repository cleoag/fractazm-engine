// _tocylindrical.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _tocylindrical
// Hand-ported from ASM: 230 bytes, DEoption=-1

constexpr unsigned int FORMULA__TOCYLINDRICAL = 4521;

// ASM trace (Luca GN 2011):
//   DD00          fld [eax]          → x
//   D8C8          fmul st0,st0       → x²
//   DD02          fld [edx]          → y
//   D8C8          fmul st0,st0       → y²
//   DEC1          faddp              → x²+y²
//   D9FA          fsqrt              → √(x²+y²)
//   DD02          fld [edx]          → y
//   DD00          fld [eax]          → x
//   D9F3          fpatan             → atan2(y, x)
//   DD1A          fstp [edx]         → y' = atan2(y, x)
//   DD18          fstp [eax]         → x' = √(x²+y²)
//
// Cartesian to cylindrical: (x,y,z) → (r, θ, z)

template<>
cpu_double3 execute_formula<FORMULA__TOCYLINDRICAL>(cpu_double3 p, constant double* params, double &de) {
    double r = sqrt(p.x * p.x + p.y * p.y);
    double theta = atan2(p.y, p.x);
    return cpu_double3(r, theta, p.z);
}

// Alternative direct function implementation
cpu_double3 _tocylindrical_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(sqrt(p.x*p.x + p.y*p.y), atan2(p.y, p.x), p.z);
}

// Formula metadata
constant FormulaInfo formula_info__tocylindrical = {
    .id = FORMULA__TOCYLINDRICAL,
    .name = "_tocylindrical",
    .param_count = 0,
    .complexity = 110,
    .uses_transcendental = true
};

/*
Original M3 Description:
Cylindrical coords. Formula by Luca GN 2011.

x' = sqrt(x*x + y*y)
y' = atan2(y, x)
z' = z (unchanged)

Parameters: none
*/
