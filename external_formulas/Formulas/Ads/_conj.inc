// _conj.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _conj
// Hand-ported from ASM: 337 bytes, DEoption=-1

constexpr unsigned int FORMULA__CONJ = 7001;

// ASM trace (Luca GN 2011):
//   DD02     fld [edx]     → y
//   D9E0     fchs          → -y
//   DD1A     fstp [edx]    → y' = -y
//   DD01     fld [ecx]     → z
//   D9E0     fchs          → -z
//   DD19     fstp [ecx]    → z' = -z
//
// 3D conjugation: negates y and z. conj(S) = (x, -y, -z, w)
// Transforms Mandelbrot set into tricorn.

template<>
cpu_double3 execute_formula<FORMULA__CONJ>(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, -p.y, -p.z);
}

// Alternative direct function implementation
cpu_double3 _conj_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, -p.y, -p.z);
}

// Formula metadata
constant FormulaInfo formula_info__conj = {
    .id = FORMULA__CONJ,
    .name = "_conj",
    .param_count = 0,
    .complexity = 60,
    .uses_transcendental = false
};

/*
Original M3 Description:
Complex conjugate function. 3D only conjugation.
Warning: This function in most cases simply screws up the fractal.
Transforms a Mandelbrot set in a tricorn.
Formula by Luca GN 2011.

conj(S) = (x, -y, -z, w)

Parameters: none
*/
