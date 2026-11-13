// _skew.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _skew
// Hand-ported from ASM: 107 bytes, DEoption=-1

constexpr unsigned int FORMULA__SKEW = 7073;

// ASM trace:
//   For each axis: sk_i = Limit - coord_i * Skewness
//   m = min(sk_x, sk_y, sk_z)
//   shift = m + |m|    (= 2 * max(m, 0))
//   x' = x - shift
//   y' = y - shift
//   z' = z - shift
//
// Non-conformal skew effect. Best used before IFS formulas
// (Menger3, Amazing Box). Subtracts a uniform shift from all
// coordinates based on the minimum skew value.

template<>
cpu_double3 execute_formula<FORMULA__SKEW>(cpu_double3 p, constant double* params, double &de) {
    const double Limit    = params[0];  // default: 1.0
    const double Skewness = params[1];  // default: -0.125

    double x = p.x, y = p.y, z = p.z;

    double sk_x = Limit - x * Skewness;
    double sk_y = Limit - y * Skewness;
    double sk_z = Limit - z * Skewness;

    double m = min(sk_x, min(sk_y, sk_z));
    double shift = m + abs(m);  // = 2 * max(m, 0)

    return cpu_double3(x - shift, y - shift, z - shift);
}

// Formula metadata
constant FormulaInfo formula_info__skew = {
    .id = FORMULA__SKEW,
    .name = "_skew",
    .param_count = 2,
    .complexity = 107,
    .uses_transcendental = false
};

/*
Original M3 Description:
Non-conformal skew effect. Best used before IFS formulas
(Menger3, Amazing Box).

Computes sk_i = Limit - coord_i * Skewness for each axis.
m = max(sk_x, sk_y, sk_z)
shift = m + |m| (= 2*max(m, 0), only shifts if m > 0)
All coordinates reduced by shift.

Parameters:
  [0] Limit: double (default: 1.0)
  [1] Skewness: double (default: -0.125)
*/
