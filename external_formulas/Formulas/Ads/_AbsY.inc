// _AbsY.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _AbsY
// Hand-ported from ASM: 278 bytes, DEoption=-1

constexpr unsigned int FORMULA__ABSY = 5002;

// ASM trace (Luca GN 2011):
//   Same as _AbsX but operates on edx (y) instead of eax (x).
//   y' = abs(y + fixY) - fixY

template<>
cpu_double3 execute_formula<FORMULA__ABSY>(cpu_double3 p, constant double* params, double &de) {
    const double fixY = params[0];
    return cpu_double3(p.x, abs(p.y + fixY) - fixY, p.z);
}

// Alternative direct function implementation
cpu_double3 _AbsY_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, abs(p.y + params[0]) - params[0], p.z);
}

// Formula metadata
constant FormulaInfo formula_info__AbsY = {
    .id = FORMULA__ABSY,
    .name = "_AbsY",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Absolute value for y (with fixing constant).
Can work as a "mirror". Formula by Luca GN 2011.

y' = abs(y + fixY) - fixY

Parameters:
  [0] fixY: double (default: 0.0)
*/
