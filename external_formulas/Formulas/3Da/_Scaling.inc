// _Scaling.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Scaling
// Hand-ported from ASM: 44 bytes, DEoption=2

constexpr unsigned int FORMULA__SCALING = 9452;

// ASM trace:
//   fld [ecx+8]      ; w
//   fld [ecx]         ; z
//   fld [edx]         ; y
//   fld [eax]         ; x
//   fld [PVar-0x10]   ; Scale
//   fmul st(4),st(0)  ; w *= Scale
//   fmul st(3),st(0)  ; z *= Scale
//   fmul st(2),st(0)  ; y *= Scale
//   fmulp st(1)       ; x *= Scale
//   All 4 components multiplied by same Scale factor.
//   DEoption=2: DE must also be scaled.

template<>
cpu_double3 execute_formula<FORMULA__SCALING>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];

    de *= Scale;

    return p * Scale;
}

// Alternative direct function implementation
cpu_double3 _Scaling_formula(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];

    de *= Scale;

    return p * Scale;
}

// Formula metadata
constant FormulaInfo formula_info__Scaling = {
    .id = FORMULA__SCALING,
    .name = "_Scaling",
    .param_count = 1,
    .complexity = 40,
    .uses_transcendental = false
};

/*
Original M3 Description:
Uniform scaling of all coordinates by Scale factor.
DEoption=2: Distance estimate is also scaled.

Mathematical Formula:
x' = x * Scale
y' = y * Scale
z' = z * Scale
DE' = DE * |Scale|

Parameters:
  [0] Scale: double (default: 1)
*/
