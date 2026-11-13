// _Rpow3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Rpow3
// Hand-ported from ASM: 33 bytes, DEoption=-1

constexpr unsigned int FORMULA__RPOW3 = 7477;
template<> struct ExternalFormulaReadsRout<FORMULA__RPOW3> { static constexpr bool value = true; };

// ASM trace:
//   fld [ecx]        ; z
//   fld [edx]        ; y
//   fld [eax]        ; x
//   fld [esi+0x38]   ; r² (pre-computed = x²+y²+z²)
//   fmul st(3),st(0) ; z *= r²
//   fmul st(2),st(0) ; y *= r²
//   fmulp st(1)      ; x *= r²
// Scales each coordinate by r², making the position vector r³ in magnitude.

template<>
cpu_double3 execute_formula<FORMULA__RPOW3>(cpu_double3 p, constant double* params, double &de) {
    const double r_sq = params[FORMULA_ROUT_SLOT];
    return p * r_sq;
}

// Alternative direct function implementation
cpu_double3 _Rpow3_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__RPOW3>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__Rpow3 = {
    .id = FORMULA__RPOW3,
    .name = "_Rpow3",
    .param_count = 0,
    .complexity = 33,
    .uses_transcendental = false
};

/*
Original M3 Description:
Does a power of 3 on the current length of the 3d vector.

Mathematical Formula:
r² = x²+y²+z²
x' = x * r²
y' = y * r²
z' = z * r²
(resulting magnitude = r³)

Parameters:
  (none - uses pre-computed r² from iteration context)
*/
