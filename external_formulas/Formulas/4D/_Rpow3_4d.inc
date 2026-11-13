// _Rpow3_4d.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Rpow3_4d
// Hand-ported from ASM: 41 bytes, DEoption=4

constexpr unsigned int FORMULA__RPOW3_4D = 9101;
template<> struct ExternalFormulaReadsRout<FORMULA__RPOW3_4D> { static constexpr bool value = true; };

// ASM trace:
//   fld [ecx+8]      ; w
//   fld [ecx]        ; z
//   fld [edx]        ; y
//   fld [eax]        ; x
//   fld [esi+0x38]   ; r² (pre-computed, includes w² in 4D mode)
//   fmul st(4)..     ; w,z,y,x all *= r²
// 4D version of _Rpow3: scales all 4 components by r².

template<>
cpu_double3 execute_formula<FORMULA__RPOW3_4D>(cpu_double3 p, constant double* params, double &de) {
    const double r_sq = params[FORMULA_ROUT_SLOT];
    de *= r_sq;
    return p * r_sq;
}

// Alternative direct function implementation
cpu_double3 _Rpow3_4d_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__RPOW3_4D>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__Rpow3_4d = {
    .id = FORMULA__RPOW3_4D,
    .name = "_Rpow3_4d",
    .param_count = 0,
    .complexity = 41,
    .uses_transcendental = false
};

/*
Original M3 Description:
Does a power of 3 on the current length of the 4d vector.

Mathematical Formula:
r² = x²+y²+z²+w² (in 4D mode)
x' = x * r²
y' = y * r²
z' = z * r²
w' = w * r²

Parameters:
  (none)
*/
