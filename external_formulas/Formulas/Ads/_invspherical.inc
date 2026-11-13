// _invspherical.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _invspherical
// Hand-ported from ASM: 308 bytes, DEoption=-1

constexpr unsigned int FORMULA__INVSPHERICAL = 4523;

// ASM trace (Luca GN 2011):
//   DD02          fld [edx]          → y
//   D9FB          fsincos            → st0=cos(y), st1=sin(y)
//   DD01          fld [ecx]          → st0=z
//   D9FB          fsincos            → st0=cos(z), st1=sin(z), st2=cos(y), st3=sin(y)
//   D9CA          fxch st2           → st0=cos(y), st2=cos(z)
//   DC08          fmul [eax]         → st0 = x*cos(y)
//   DD19          fstp [ecx]         → z' = x*cos(y)
//   D9CA+DCC9     fxch; fmul st1     → compute sin(y)*cos(z), sin(y)*sin(z)
//   ... multiply by x, store to x', y'
//
// Spherical to Cartesian: (r, θ, φ) → (r·sin(θ)·cos(φ), r·sin(θ)·sin(φ), r·cos(θ))

template<>
cpu_double3 execute_formula<FORMULA__INVSPHERICAL>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x;  // r
    double sy = sin(p.y);  // sin(theta)
    double cy = cos(p.y);  // cos(theta)
    double sz = sin(p.z);  // sin(phi)
    double cz = cos(p.z);  // cos(phi)

    return cpu_double3(x * sy * cz, x * sy * sz, x * cy);
}

// Alternative direct function implementation
cpu_double3 _invspherical_formula(cpu_double3 p, constant double* params, double &de) {
    double sy = sin(p.y), cy = cos(p.y);
    double sz = sin(p.z), cz = cos(p.z);
    return cpu_double3(p.x * sy * cz, p.x * sy * sz, p.x * cy);
}

// Formula metadata
constant FormulaInfo formula_info__invspherical = {
    .id = FORMULA__INVSPHERICAL,
    .name = "_invspherical",
    .param_count = 0,
    .complexity = 170,
    .uses_transcendental = true
};

/*
Original M3 Description:
Inverse spherical coords. Fast conversion without parameters.
Formula by Luca GN 2011.

From Wikipedia:
x' = x * sin(y) * cos(z)
y' = x * sin(y) * sin(z)
z' = x * cos(y)

Parameters: none
*/
