// _invtorical.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _invtorical
// Hand-ported from ASM: 383 bytes, DEoption=-1

constexpr unsigned int FORMULA__INVTORICAL = 7043;

// ASM trace:
//   8B7630        mov esi,[esi+0x30] → PVar
//   DD00/D9E0     x; -x
//   DC46F0        fadd R → R-x
//   D9E1          fabs → |R-x|
//   DD02/D9FF     y; cos(y)
//   D9E1          fabs → |cos(y)|
//   DEC9          |R-x| * |cos(y)| = x'
//   DD10          fst [eax] → store x' (keep on stack)
//   DD02/D9F2     y; fptan → 1, tan(y)
//   DEC9/DEC9     multiply: tan(y) * x' → y'
//   DD1A          store y'
//
// Inverse of _totorical: converts from torical coords back to Cartesian.

template<>
cpu_double3 execute_formula<FORMULA__INVTORICAL>(cpu_double3 p, constant double* params, double &de) {
    const double R = params[0];  // default: 2.0

    double new_x = abs(R - p.x) * abs(cos(p.y));
    double new_y = new_x * tan(p.y);

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__invtorical = {
    .id = FORMULA__INVTORICAL,
    .name = "_invtorical",
    .param_count = 1,
    .complexity = 90,
    .uses_transcendental = true
};

/*
Original M3 Description:
Reversed torical (generalized) coords.
Formula by Luca GN 2011

x' = |R - x| * |cos(y)|
y' = x' * tan(y)
z' = z (unchanged)

This is the inverse of _totorical.
Note: tan(y) can diverge when y = pi/2 + n*pi.

Parameters:
  [0] R: double (default: 2.0)
*/
