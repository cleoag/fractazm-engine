// _SinY.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SinY
// Hand-ported from ASM: 400 bytes, DEoption=-1

constexpr unsigned int FORMULA__SINY = 7010;
template<> struct ExternalFormulaWritesWState<FORMULA__SINY> {
    static constexpr bool value = true;
};

// ASM trace (with .Integer and .Single params):
//   8B77F4      mov esi,[edi-0x0C]  → Index (integer, 0-3)
//   83E603      and esi, 3          → mask to 0..3
//   DD04F0      fld [eax+esi*8]     → coord[Index]
//   D867F0      fsub dword [edi-0x10] → - Offset1 (float32)
//   D84FEC      fmul dword [edi-0x14] → * Scale1 (float32)
//   D9FE        fsin                → sin(...)
//   D84FE8      fmul dword [edi-0x18] → * Scale2 (float32)
//   D847E4      fadd dword [edi-0x1C] → + Offset2 (float32)
//   DD1CF0      fstp [eax+esi*8]    → store back
//
// coord[Index] = sin((coord[Index] - Offset1) * Scale1) * Scale2 + Offset2
// Default: Index=1(y), Offset1=0, Scale1=1, Scale2=1, Offset2=0 → y = sin(y)

template<>
cpu_double3 execute_formula<FORMULA__SINY>(cpu_double3 p, constant double* params, double &de) {
    const int Index = int(params[0]);  // ReadFormulaParams already converts Integer.
    const double Offset1 = params[1];
    const double Scale1 = params[2];
    const double Scale2 = params[3];
    const double Offset2 = params[4];

    // Select coordinate by index (0=x, 1=y, 2=z)
    double coord;
    switch (Index & 3) {
        case 0: coord = p.x; break;
        case 1: coord = p.y; break;
        case 2: coord = p.z; break;
        default: coord = params[FORMULA_STATE_W_SLOT]; break;
    }

    double result = sin((coord - Offset1) * Scale1) * Scale2 + Offset2;

    switch (Index & 3) {
        case 0: return cpu_double3(result, p.y, p.z);
        case 1: return cpu_double3(p.x, result, p.z);
        case 2: return cpu_double3(p.x, p.y, result);
        default: params[FORMULA_STATE_W_SLOT] = result; return p;
    }
}

// Alternative direct function implementation (default: y axis)
cpu_double3 _SinY_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__SINY>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__SinY = {
    .id = FORMULA__SINY,
    .name = "_SinY",
    .param_count = 5,
    .complexity = 85,
    .uses_transcendental = true
};

/*
Original M3 Description:
Applies sine function to selected coordinate:
coord[Index] = sin((coord[Index] - Offset1) * Scale1) * Scale2 + Offset2

Index: 0=x, 1=y, 2=z, 3=w

Parameters:
  [0] Index: integer (default: 1 = y)
  [1] Offset 1: single (default: 0)
  [2] Scale 1: single (default: 1)
  [3] Scale 2: single (default: 1)
  [4] Offset 2: single (default: 0)
*/
