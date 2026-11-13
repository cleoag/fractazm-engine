// _AbsX.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _AbsX
// Hand-ported from ASM: 278 bytes, DEoption=-1

constexpr unsigned int FORMULA__ABSX = 5001;

// ASM trace (Luca GN 2011):
//   8B7630        mov esi,[esi+0x30]  → esi = PVar
//   DD00          fld [eax]           → x
//   DD46F0        fld [esi-0x10]      → fixX
//   DEC1          faddp               → x + fixX
//   D9E1          fabs                → |x + fixX|
//   DD46F0        fld [esi-0x10]      → fixX
//   D9E0          fchs                → -fixX
//   DEC1          faddp               → |x + fixX| - fixX
//   DD18          fstp [eax]          → x' = |x + fixX| - fixX
//
// Mirror fold on X axis. Default fixX=0 gives x'=|x|.

template<>
cpu_double3 execute_formula<FORMULA__ABSX>(cpu_double3 p, double* params, double &de) {
    const double fixX = params[0];
    return cpu_double3(abs(p.x + fixX) - fixX, p.y, p.z);
}

// Alternative direct function implementation
cpu_double3 _AbsX_formula(cpu_double3 p, double* params, double &de) {
    return cpu_double3(abs(p.x + params[0]) - params[0], p.y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__AbsX = {
    .id = FORMULA__ABSX,
    .name = "_AbsX",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Absolute value for x (with fixing constant).
Can work as a "mirror". Formula by Luca GN 2011.

x' = abs(x + fixX) - fixX

Parameters:
  [0] fixX: double (default: 0.0)
*/
