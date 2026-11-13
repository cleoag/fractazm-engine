// _YplusSinZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _YplusSinZ
// Hand-ported from ASM: 117 bytes, DEoption=-1

constexpr unsigned int FORMULA__YPLUSSINZ = 1427;

// ASM trace:
//   558BEC          push ebp; mov ebp,esp
//   DD02            fld [edx]       → st0 = y
//   DD01            fld [ecx]       → st0 = z, st1 = y
//   D9FE            fsin            → st0 = sin(z)
//   DEC1            faddp st1,st0   → st0 = y + sin(z)
//   DD1A            fstp [edx]      → y' = y + sin(z)
//   5DC20800        pop ebp; ret 8

template<>
cpu_double3 execute_formula<FORMULA__YPLUSSINZ>(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, p.y + sin(p.z), p.z);
}

// Alternative direct function implementation
cpu_double3 _YplusSinZ_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, p.y + sin(p.z), p.z);
}

// Formula metadata
constant FormulaInfo formula_info__YplusSinZ = {
    .id = FORMULA__YPLUSSINZ,
    .name = "_YplusSinZ",
    .param_count = 0,
    .complexity = 50,
    .uses_transcendental = true
};

/*
Original M3 Description:
y' = y + sin(z)

Parameters: none
*/
