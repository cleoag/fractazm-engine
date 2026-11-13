// Aex-Octo.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Aex-Octo
// Hand-ported from ASM: 411 bytes, DEscale=1, SIpower=2

constexpr unsigned int FORMULA_AEX_OCTO = 7027;

// ASM trace:
//   DD03        fld [ebx]           → x
//   DC09        fmul [ecx]          → x*z
//   DC4FF0      fmul [edi-0x10]     → x*z*XZmul
//   DC4618      fadd [esi+0x18]     → + Cx   → x' = XZmul*x*z + Cx
//   DD03        fld [ebx]           → x
//   D8C8        fmul st0,st0        → x²
//   DD01        fld [ecx]           → z
//   D8C8        fmul st0,st0        → z²
//   DEE1        fsubrp st1,st0      → z² - x² (FSUBRP: st1 = st0 - st1)
//   DC4FE8      fmul [edi-0x18]     → (z²-x²)*Sqmul
//   DC4620      fadd [esi+0x20]     → + Cy   → y' = Sqmul*(z²-x²) + Cy
//   DD02        fld [edx]           → y
//   DC4628      fadd [esi+0x28]     → + Cz   → z' = y + Cz
//   DD19/1A/1B  store z', y', x'
//
// "Octopus formula" by Aexion. Use in Julia mode!

template<>
cpu_double3 execute_formula<FORMULA_AEX_OCTO>(cpu_double3 p, constant double* params, double &de) {
    const double XZmul = params[0];
    const double Sqmul = params[1];

    double new_x = XZmul * p.x * p.z;
    double new_y = Sqmul * (p.z * p.z - p.x * p.x);
    double new_z = p.y;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Aex_Octo = {
    .id = FORMULA_AEX_OCTO,
    .name = "Aex-Octo",
    .param_count = 2,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
The "Octopus formula" kindly suggested by Aexion.
Use in Julia mode!

x' = XZmul * x * z + Cx
y' = Sqmul * (-x² + z²) + Cy
z' = y + Cz

DEscale=1, SIpower=2.
Parameters:
  [0] XZ mul: double (default: 1.25)
  [1] Sq mul: double (default: 1.0)
*/
