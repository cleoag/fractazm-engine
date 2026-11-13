// _AbsZ.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _AbsZ
// Hand-ported from ASM: 278 bytes, DEoption=-1

constexpr unsigned int FORMULA__ABSZ = 5003;

// ASM trace (Luca GN 2011):
//   Same as _AbsX but operates on ecx (z) instead of eax (x).
//   z' = abs(z + fixZ) - fixZ

template<>
cpu_double3 execute_formula<FORMULA__ABSZ>(cpu_double3 p, constant double* params, double &de) {
    const double fixZ = params[0];
    return cpu_double3(p.x, p.y, abs(p.z + fixZ) - fixZ);
}

// Alternative direct function implementation
cpu_double3 _AbsZ_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(p.x, p.y, abs(p.z + params[0]) - params[0]);
}

// Formula metadata
constant FormulaInfo formula_info__AbsZ = {
    .id = FORMULA__ABSZ,
    .name = "_AbsZ",
    .param_count = 1,
    .complexity = 80,
    .uses_transcendental = false
};

/*
Original M3 Description:
Absolute value for z (with fixing constant).
Can work as a "mirror". Formula by Luca GN 2011.

z' = abs(z + fixZ) - fixZ

Parameters:
  [0] fixZ: double (default: 0.0)
*/
