// _invcylindrical.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _invcylindrical
// Hand-ported from ASM: 237 bytes, DEoption=-1

constexpr unsigned int FORMULA__INVCYLINDRICAL = 4522;

// ASM trace (Luca GN 2011):
//   DD02          fld [edx]          → y
//   D9FB          fsincos            → st0=cos(y), st1=sin(y)
//   DD00          fld [eax]          → st0=x, st1=cos(y), st2=sin(y)
//   DCCA          fmul st2,st0       → st2 = x*sin(y)
//   DEC9          fmulp st1,st0      → st0 = x*cos(y), st1 = x*sin(y)
//   DD18          fstp [eax]         → x' = x*cos(y)
//   DD1A          fstp [edx]         → y' = x*sin(y)
//
// Cylindrical to Cartesian: (r, θ, z) → (r·cos(θ), r·sin(θ), z)

template<>
cpu_double3 execute_formula<FORMULA__INVCYLINDRICAL>(cpu_double3 p, constant double* params, double &de) {
    double c = cos(p.y);
    double s = sin(p.y);
    return cpu_double3(p.x * c, p.x * s, p.z);
}

// Alternative direct function implementation
cpu_double3 _invcylindrical_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x * cos(p.y), p.x * sin(p.y), p.z);
}

// Formula metadata
constant FormulaInfo formula_info__invcylindrical = {
    .id = FORMULA__INVCYLINDRICAL,
    .name = "_invcylindrical",
    .param_count = 0,
    .complexity = 70,
    .uses_transcendental = true
};

/*
Original M3 Description:
Inverse cylindrical coords, very easy transform.
Formula by Luca GN 2011.

x' = x * cos(y)
y' = x * sin(y)
z' = z (unchanged)

Parameters: none
*/
