// _SquareR.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SquareR
// Hand-ported from ASM: 39 bytes, DEoption=-1

constexpr unsigned int FORMULA__SQUARER = 9574;
template<> struct ExternalFormulaReadsRout<FORMULA__SQUARER> { static constexpr bool value = true; };

// ASM trace:
//   fld [ecx]        ; z
//   fld [edx]        ; y
//   fld [eax]        ; x
//   fld [esi+0x38]   ; r² (pre-computed)
//   fld st(0)        ; dup: r², r²
//   fsqrt            ; sqrt(r²), r²
//   fdivp st(1)      ; r² / sqrt(r²) = sqrt(r²) = r
//   fmul st(3)..     ; xyz *= r
// m = r²/sqrt(r²) = sqrt(r²) = r
// Scales position by r, making magnitude r² ("squares the radius")

template<>
cpu_double3 execute_formula<FORMULA__SQUARER>(cpu_double3 p, constant double* params, double &de) {
    const double r_sq = params[FORMULA_ROUT_SLOT];
    // Preserve the executable x87 behavior exactly:
    //   r = r_sq / sqrt(r_sq)
    // rather than simplifying to sqrt(r_sq). The two differ for infinities,
    // where x87 yields inf/inf -> NaN and the rewritten form would stay inf.
    double r = r_sq / sqrt(r_sq);
    return p * r;
}

// Alternative direct function implementation
cpu_double3 _SquareR_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__SQUARER>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__SquareR = {
    .id = FORMULA__SQUARER,
    .name = "_SquareR",
    .param_count = 0,
    .complexity = 39,
    .uses_transcendental = false
};

/*
Original M3 Description:
Squares the current length of the 3d vector.

Mathematical Formula:
r = sqrt(x²+y²+z²)
x' = x * r
y' = y * r
z' = z * r
(resulting magnitude = r²)

Parameters:
  (none)
*/
