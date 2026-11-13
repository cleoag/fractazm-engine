// _MaxClipping.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _MaxClipping
// Hand-ported from ASM: 351 bytes, DEoption=-1

constexpr unsigned int FORMULA__MAXCLIPPING = 7003;

// ASM trace:
//   For each axis: load coord, fcomp with clip limit, sahf, jbe (skip if <= limit)
//   If coord > limit: copy the 8-byte double limit value directly into coord memory.
//
//   The formula clips each coordinate to its maximum allowed value.
//   Uses integer memory copy (mov dword) to avoid FPU state issues.
//
// x' = min(x, X_Clip)
// y' = min(y, Y_Clip)
// z' = min(z, Z_Clip)

template<>
cpu_double3 execute_formula<FORMULA__MAXCLIPPING>(cpu_double3 p, constant double* params, double &de) {
    const double xClip = params[0];
    const double yClip = params[1];
    const double zClip = params[2];

    return cpu_double3(min(p.x, xClip), min(p.y, yClip), min(p.z, zClip));
}

// Alternative direct function implementation
cpu_double3 _MaxClipping_formula(cpu_double3 p, constant double* params, double &de) {
    return cpu_double3(min(p.x, params[0]), min(p.y, params[1]), min(p.z, params[2]));
}

// Formula metadata
constant FormulaInfo formula_info__MaxClipping = {
    .id = FORMULA__MAXCLIPPING,
    .name = "_MaxClipping",
    .param_count = 3,
    .complexity = 90,
    .uses_transcendental = false
};

/*
Original M3 Description:
Clips each coordinate to its maximum value.

x' = min(x, X_Clip)
y' = min(y, Y_Clip)
z' = min(z, Z_Clip)

Parameters:
  [0] X Clip: double (default: 1024)
  [1] Y Clip: double (default: 1024)
  [2] Z Clip: double (default: 1024)
*/
