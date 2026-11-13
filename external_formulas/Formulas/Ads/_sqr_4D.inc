// _sqr_4D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _sqr_4D
// Hand-ported from ASM: 445 bytes, DEoption=-1

constexpr unsigned int FORMULA__SQR_4D = 7014;
template<> struct ExternalFormulaWritesWState<FORMULA__SQR_4D> { static constexpr bool value = true; };

// ASM trace (identical to _sqr_3D plus w component):
//   8BD8        mov ebx, eax        → save &x
//   8B7630      mov esi,[esi+0x30]  → esi = PVar
//   DD03        fld [ebx]           → x
//   D8C8        fmul st0,st0        → x²
//   DC4EF0      fmul [esi-0x10]     → x² * Scale
//   DC46E8      fadd [esi-0x18]     → + fixX
//   DD1B        fstp [ebx]          → store x'
//   (same pattern for y with [edx], fixY at [esi-0x20])
//   (same pattern for z with [ecx], fixZ at [esi-0x28])
//   DD4108      fld [ecx+8]         → w (contiguous after z)
//   D8C8        fmul st0,st0        → w²
//   DC4EF0      fmul [esi-0x10]     → w² * Scale
//   DC46D0      fadd [esi-0x30]     → + fixW
//   DD5908      fstp [ecx+8]        → store w'
//
// coord' = coord² * Scale + fix (for all 4 dimensions)

template<>
cpu_double3 execute_formula<FORMULA__SQR_4D>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double fixX  = params[1];
    const double fixY  = params[2];
    const double fixZ  = params[3];
    double w = params[FORMULA_STATE_W_SLOT];
    params[FORMULA_STATE_W_SLOT] = w * w * Scale + params[4];

    double new_x = p.x * p.x * Scale + fixX;
    double new_y = p.y * p.y * Scale + fixY;
    double new_z = p.z * p.z * Scale + fixZ;

    return cpu_double3(new_x, new_y, new_z);
}

// Alternative direct function implementation
cpu_double3 _sqr_4D_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__SQR_4D>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__sqr_4D = {
    .id = FORMULA__SQR_4D,
    .name = "_sqr_4D",
    .param_count = 5,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Squares each coordinate with scaling and offset (4D version):
x' = x² * Scale + fixX
y' = y² * Scale + fixY
z' = z² * Scale + fixZ
w' = w² * Scale + fixW

Disable Analytical DE to see the effect correctly.

Parameters:
  [0] Scale: double (default: 1.0)
  [1] fixX: double (default: 0.0)
  [2] fixY: double (default: 0.0)
  [3] fixZ: double (default: 0.0)
  [4] fixW: double (default: 0.0)
*/
