// _conj2D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _conj2D
// Hand-ported from ASM: 349 bytes, DEoption=-1

constexpr unsigned int FORMULA__CONJ2D = 7002;

// ASM trace (Luca GN 2011):
//   DD02     fld [edx]     → y
//   D9E0     fchs          → -y
//   DD1A     fstp [edx]    → y' = -y
//
// 2D conjugation: only negates y. conj(S) = (x, -y, z, w)

template<>
cpu_double3 execute_formula<FORMULA__CONJ2D>(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, -p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _conj2D_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, -p.y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__conj2D = {
    .id = FORMULA__CONJ2D,
    .name = "_conj2D",
    .param_count = 0,
    .complexity = 30,
    .uses_transcendental = false
};

/*
Original M3 Description:
Complex conjugate function. Normal complex conjugation - affects only y!
Transforms a Mandelbrot set in a tricorn.
Formula by Luca GN 2011.

conj(S) = (x, -y, z, w)

Parameters: none
*/
