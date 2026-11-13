// _toruspinv3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _toruspinv3
// Hand-ported from ASM: 111 bytes, DEoption=-1

constexpr unsigned int FORMULA__TORUSPINV3 = 7069;

// ASM trace:
//   Differs from _toruspinv1 in TWO ways:
//   1. No sqrt: uses d_sq = (R-(x²+y²))² + z²  directly (D9FA→D9D0 = fnop)
//   2. z uses multiply: z' = Radius * d_sq * |z|  (same as v2)
//
//   d_sq = (R - (x²+y²))² + z²    (NO sqrt!)
//   x' = |x| * Radius / d_sq - Xctr
//   y' = |y| * Radius / d_sq - Yctr
//   z' = Radius * d_sq * |z| - Zctr

template<>
cpu_double3 execute_formula<FORMULA__TORUSPINV3>(cpu_double3 p, constant double* params, double &de) {
    const double Xctr   = params[0];
    const double Yctr   = params[1];
    const double Zctr   = params[2];
    const double Radius = params[3];
    const double R      = params[4];

    double x = p.x + Xctr;
    double y = p.y + Yctr;
    double z = p.z + Zctr;

    double t = R - (x*x + y*y);
    double d_sq = t*t + z*z;  // NO sqrt (unlike v1 and v2)

    double new_x = abs(x) * Radius / d_sq - Xctr;
    double new_y = abs(y) * Radius / d_sq - Yctr;
    double new_z = Radius * d_sq * abs(z) - Zctr;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__toruspinv3 = {
    .id = FORMULA__TORUSPINV3,
    .name = "_toruspinv3",
    .param_count = 5,
    .complexity = 111,
    .uses_transcendental = false
};

/*
Original M3 Description:
Torus pseudo-inversion variant 3. Differs from v1 in two ways:
1. No sqrt: uses squared distance directly
2. z uses multiply (like v2)
Formula by Luca GN 2011.

Parameters:
  [0] X center: double (default: 0)
  [1] Y center: double (default: 0)
  [2] Z center: double (default: 0)
  [3] Radius: double (default: 1)
  [4] R: double (default: 1)
*/
