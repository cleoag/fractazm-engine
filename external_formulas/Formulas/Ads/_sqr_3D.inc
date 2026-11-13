// _sqr_3D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _sqr_3D
// Hand-ported from ASM: 382 bytes, DEoption=-1

constexpr unsigned int FORMULA__SQR_3D = 7012;

// ASM trace:
//   8BD8        mov ebx, eax        → save &x
//   8B7630      mov esi,[esi+0x30]  → esi = PVar
//   DD03        fld [ebx]           → x
//   D8C8        fmul st0,st0        → x²
//   DC4EF0      fmul [esi-0x10]     → x² * Scale
//   DC46E8      fadd [esi-0x18]     → + fixX
//   DD1B        fstp [ebx]          → store x'
//   DD02        fld [edx]           → y
//   D8C8        fmul st0,st0        → y²
//   DC4EF0      fmul [esi-0x10]     → y² * Scale
//   DC46E0      fadd [esi-0x20]     → + fixY
//   DD1A        fstp [edx]          → store y'
//   DD01        fld [ecx]           → z
//   D8C8        fmul st0,st0        → z²
//   DC4EF0      fmul [esi-0x10]     → z² * Scale
//   DC46D8      fadd [esi-0x28]     → + fixZ
//   DD19        fstp [ecx]          → store z'
//
// x' = x² * Scale + fixX
// y' = y² * Scale + fixY
// z' = z² * Scale + fixZ

template<>
cpu_double3 execute_formula<FORMULA__SQR_3D>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double fixX  = params[1];
    const double fixY  = params[2];
    const double fixZ  = params[3];

    double new_x = p.x * p.x * Scale + fixX;
    double new_y = p.y * p.y * Scale + fixY;
    double new_z = p.z * p.z * Scale + fixZ;

    return cpu_double3(new_x, new_y, new_z);
}

// Alternative direct function implementation
cpu_double3 _sqr_3D_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__SQR_3D>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__sqr_3D = {
    .id = FORMULA__SQR_3D,
    .name = "_sqr_3D",
    .param_count = 4,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Squares each coordinate with scaling and offset:
x' = x² * Scale + fixX
y' = y² * Scale + fixY
z' = z² * Scale + fixZ

Disable Analytical DE to see the effect correctly.

Parameters:
  [0] Scale: double (default: 1.0)
  [1] fixX: double (default: 0.0)
  [2] fixY: double (default: 0.0)
  [3] fixZ: double (default: 0.0)
*/
