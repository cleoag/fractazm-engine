// _FlipXZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FlipXZ
// Hand-ported from ASM: 144 bytes, DEoption=-1

constexpr unsigned int FORMULA__FLIPXZ = 5876;

// ASM trace:
//   558BEC          push ebp; mov ebp,esp
//   DD01            fld [ecx]       → st0 = z
//   DD00            fld [eax]       → st0 = x, st1 = z
//   DD19            fstp [ecx]      → z' = x
//   DD18            fstp [eax]      → x' = z
//   5DC20800        pop ebp; ret 8

template<>
cpu_double3 execute_formula<FORMULA__FLIPXZ>(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.z, p.y, p.x);
}

// Alternative direct function implementation
cpu_double3 _FlipXZ_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.z, p.y, p.x);
}

// Formula metadata
constant FormulaInfo formula_info__FlipXZ = {
    .id = FORMULA__FLIPXZ,
    .name = "_FlipXZ",
    .param_count = 0,
    .complexity = 40,
    .uses_transcendental = false
};

/*
Original M3 Description:
Exchanging the x and z values:
x' = z
z' = x

Parameters: none
*/
