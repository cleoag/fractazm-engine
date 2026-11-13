// _invtorical2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _invtorical2
// Hand-ported from ASM: 314 bytes, DEoption=-1

constexpr unsigned int FORMULA__INVTORICAL2 = 7045;

// ASM trace:
//   8B7630        mov esi,[esi+0x30] → PVar
//   [esi-0x10] = R, [esi-0x18] = Gamma
//
//   DD02/DC46E8/D9FB  y + Gamma; fsincos → cos(y+G), sin(y+G) → locals
//   DD01/D9FB         z; fsincos → cos(z), sin(z) → locals
//   DD00/DC4DF0/DC46F0  x*cos(y+G) + R = t → local
//   DD00/DC4DE8/DD19    z' = x*sin(y+G)
//   DD45D0/DC4DD8/DD1A  y' = t*sin(z)
//   DD45D0/DC4DE0/DD18  x' = t*cos(z)
//
// Inverse of _totorical2 by Luca GN 2011.
//
// Validation note:
//   The algebra matches the x86 ASM, but the original implementation uses x87
//   `fsincos` plus intermediate stores on every step. That exact x87 behavior
//   is not fully emulated on arm64, so a small accepted late-iteration parity
//   tail remains in validation even though the formula port itself is
//   considered closed.

template<>
cpu_double3 execute_formula<FORMULA__INVTORICAL2>(cpu_double3 p, constant double* params, double &de) {
    const double R     = params[0];  // default: 2.0
    const double Gamma = params[1];  // default: 0.0

    double sy, cy;
    sy = sincos(p.y + Gamma, cy);

    double sz, cz;
    sz = sincos(p.z, cz);

    double t = R + p.x * cy;

    double new_x = t * cz;
    double new_y = t * sz;
    double new_z = p.x * sy;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__invtorical2 = {
    .id = FORMULA__INVTORICAL2,
    .name = "_invtorical2",
    .param_count = 2,
    .complexity = 95,
    .uses_transcendental = true
};

/*
Original M3 Description:
Inverse torical coords by BradC (FractalForums).
Formula by Luca GN 2011.

t = R + x * cos(y + Gamma)
z' = x * sin(y + Gamma)
y' = t * sin(z)
x' = t * cos(z)

This is the inverse of _totorical2.

Parameters:
  [0] R: double (default: 2.0)
  [1] Gamma: double (default: 0.0)
*/
