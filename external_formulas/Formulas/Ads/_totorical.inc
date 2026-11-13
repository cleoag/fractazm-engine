// _totorical.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _totorical
// Hand-ported from ASM: 280 bytes, DEoption=-1

constexpr unsigned int FORMULA__TOTORICAL = 7040;

// ASM trace:
//   8B7630      mov esi,[esi+0x30]  → PVar (overwrites esi)
//   DD00/D8C8   x; x²
//   DD02/D8C8   y; y²
//   DEC1        x² + y²
//   D9FA        sqrt(x²+y²)
//   D9E0        fchs → -sqrt(x²+y²)
//   DC46F0      fadd [esi-0x10] → R - sqrt(x²+y²)
//   DD02/DD00   push y, push x
//   D9F3        fpatan → atan2(y, x)
//   DD1A        store y' = atan2(y, x)
//   DD18        store x' = R - sqrt(x²+y²)
//
// Torical coordinates by Luca GN 2011.

template<>
cpu_double3 execute_formula<FORMULA__TOTORICAL>(cpu_double3 p, constant double* params, double &de) {
    const double R = params[0];  // default: 2.0

    double new_x = R - sqrt(p.x * p.x + p.y * p.y);
    double new_y = atan2(p.y, p.x);

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__totorical = {
    .id = FORMULA__TOTORICAL,
    .name = "_totorical",
    .param_count = 1,
    .complexity = 90,
    .uses_transcendental = true
};

/*
Original M3 Description:
Torical (generalized) coords.
Formula by Luca GN 2011

Define R = (inner radius + outer radius) / 2.

x' = R - sqrt(x² + y²)
y' = atan2(y, x)
z' = z (unchanged)

Almost identical to standard cylindrical coords.
Reference: http://mathworld.wolfram.com/Torus.html

Parameters:
  [0] R: double (default: 2.0)
*/
