// _FlipYZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _FlipYZ
// Hand-ported from ASM: 144 bytes, DEoption=-1

constexpr unsigned int FORMULA__FLIPYZ = 8377;

// ASM trace:
//   558BEC          push ebp; mov ebp,esp
//   DD01            fld [ecx]       → st0 = z
//   DD02            fld [edx]       → st0 = y, st1 = z
//   DD19            fstp [ecx]      → z' = y
//   DD1A            fstp [edx]      → y' = z
//   5DC20800        pop ebp; ret 8

template<>
cpu_double3 execute_formula<FORMULA__FLIPYZ>(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, p.z, p.y);
}

// Alternative direct function implementation
cpu_double3 _FlipYZ_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, p.z, p.y);
}

// Formula metadata
constant FormulaInfo formula_info__FlipYZ = {
    .id = FORMULA__FLIPYZ,
    .name = "_FlipYZ",
    .param_count = 0,
    .complexity = 40,
    .uses_transcendental = false
};

/*
Original M3 Description:
Exchanging the y and z values:
y' = z
z' = y

Parameters: none
*/
