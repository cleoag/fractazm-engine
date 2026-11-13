// _FlipXY.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FlipXY
// Hand-ported from ASM: 144 bytes, DEoption=-1

constexpr unsigned int FORMULA__FLIPXY = 3437;

// ASM trace:
//   558BEC          push ebp; mov ebp,esp
//   DD02            fld [edx]       → st0 = y
//   DD00            fld [eax]       → st0 = x, st1 = y
//   DD1A            fstp [edx]      → y' = x
//   DD18            fstp [eax]      → x' = y
//   5DC20800        pop ebp; ret 8

template<>
cpu_double3 execute_formula<FORMULA__FLIPXY>(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.y, p.x, p.z);
}

// Alternative direct function implementation
cpu_double3 _FlipXY_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.y, p.x, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__FlipXY = {
    .id = FORMULA__FLIPXY,
    .name = "_FlipXY",
    .param_count = 0,
    .complexity = 40,
    .uses_transcendental = false
};

/*
Original M3 Description:
Exchanging the x and y values:
x' = y
y' = x

Parameters: none
*/
