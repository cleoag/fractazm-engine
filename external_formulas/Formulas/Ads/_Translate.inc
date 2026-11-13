// _Translate.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _Translate
// Hand-ported from ASM: 179 bytes, DEoption=-1

constexpr unsigned int FORMULA__TRANSLATE = 8065;

// ASM trace:
//   558BEC56        push ebp; mov ebp,esp; push esi
//   8B7508          mov esi,[ebp+8]    → esi = iteration context
//   8B7630          mov esi,[esi+0x30] → esi = PVar
//   DD01            fld [ecx]          → st0 = z
//   DD02            fld [edx]          → st0 = y, st1 = z
//   DD00            fld [eax]          → st0 = x, st1 = y, st2 = z
//   DC46F0          fadd [esi-0x10]    → st0 = x + X_add
//   DD18            fstp [eax]         → x' = x + X_add
//   DC46E8          fadd [esi-0x18]    → st0 = y + Y_add  (now st0 = y after pop)
//   DD1A            fstp [edx]         → y' = y + Y_add
//   DC46E0          fadd [esi-0x20]    → st0 = z + Z_add  (now st0 = z after pop)
//   DD19            fstp [ecx]         → z' = z + Z_add
//   5E5DC20800      pop esi; pop ebp; ret 8

template<>
cpu_double3 execute_formula<FORMULA__TRANSLATE>(cpu_double3 p, constant double* params, double &de) {
    const double X_add = params[0];
    const double Y_add = params[1];
    const double Z_add = params[2];

    return cpu_double3(p.x + X_add, p.y + Y_add, p.z + Z_add);
}

// Alternative direct function implementation
cpu_double3 _Translate_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x + params[0], p.y + params[1], p.z + params[2]);
}

// Formula metadata
constant FormulaInfo formula_info__Translate = {
    .id = FORMULA__TRANSLATE,
    .name = "_Translate",
    .param_count = 3,
    .complexity = 60,
    .uses_transcendental = false
};

/*
Original M3 Description:
Simple translation of coordinates.

x' = x + X_add
y' = y + Y_add
z' = z + Z_add

Parameters:
  [0] X_add: double (default: 0)
  [1] Y_add: double (default: 0)
  [2] Z_add: double (default: 0)
*/
