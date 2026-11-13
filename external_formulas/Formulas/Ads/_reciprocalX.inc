// _reciprocalX.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _reciprocalX
// Hand-ported from ASM: 177 bytes, DEoption=-1

constexpr unsigned int FORMULA__RECIPROCALX = 9189;

// ASM trace:
//   558BEC          push ebp; mov ebp,esp
//   DD00            fld [eax]       → st0 = x
//   D9E8            fld1            → st0 = 1.0, st1 = x
//   DEF1            fdivrp st1,st0  → st0 = 1.0 / x
//   DD18            fstp [eax]      → x' = 1/x
//   5DC20800        pop ebp; ret 8
//
// Note: Disturbs the DE, so handle with care.

template<>
cpu_double3 execute_formula<FORMULA__RECIPROCALX>(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(1.0 / p.x, p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _reciprocalX_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(1.0 / p.x, p.y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__reciprocalX = {
    .id = FORMULA__RECIPROCALX,
    .name = "_reciprocalX",
    .param_count = 0,
    .complexity = 40,
    .uses_transcendental = false
};

/*
Original M3 Description:
Computes the reciprocal of x:
x = 1/x

Disturbs the DE, so handle with care.

Parameters: none
*/
