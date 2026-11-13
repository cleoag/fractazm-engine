// _SinePow2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _SinePow2
// Hand-ported from ASM: 76 bytes, DEoption=-1

constexpr unsigned int FORMULA__SINEPOW2 = 9447;

// ASM trace (careful FPU stack analysis):
//   Load z, y, x onto FPU stack
//   y² = y*y, x² = x*x
//   r_xy² = x²+y², r_xy = sqrt(r_xy²)
//   t = (r_xy² - z²) / r_xy²   (= 1 - z²/(x²+y²))
//
//   new_z = 2 * Z_mul * r_xy * z
//   new_x = (x² - y²) * t
//   new_y = 2 * x * y * t * Y_mul
//
// This is a trigonometry-free power-2 Mandelbulb using double-angle identities.

template<>
cpu_double3 execute_formula<FORMULA__SINEPOW2>(cpu_double3 p, constant double* params, double &de) {
    const double Y_mul = params[0];
    const double Z_mul = params[1];

    double x = p.x, y = p.y, z = p.z;
    double x_sq = x * x;
    double y_sq = y * y;
    double r_xy_sq = x_sq + y_sq;
    double r_xy = sqrt(r_xy_sq);

    // The x87 path has no zero guard here; preserve the natural IEEE non-finite
    // path instead of clamping singular cases to zero.
    double t = (r_xy_sq - z * z) / r_xy_sq;

    cpu_double3 result;
    result.x = (x_sq - y_sq) * t;
    result.y = 2.0 * x * y * t * Y_mul;
    result.z = 2.0 * r_xy * z * Z_mul;

    return result;
}

// Alternative direct function implementation
cpu_double3 _SinePow2_formula(cpu_double3 p, constant double* params, double &de) {
    const double Y_mul = params[0];
    const double Z_mul = params[1];

    double x = p.x, y = p.y, z = p.z;
    double x_sq = x * x;
    double y_sq = y * y;
    double r_xy_sq = x_sq + y_sq;
    double r_xy = sqrt(r_xy_sq);

    double t = (r_xy_sq - z * z) / r_xy_sq;

    cpu_double3 result;
    result.x = (x_sq - y_sq) * t;
    result.y = 2.0 * x * y * t * Y_mul;
    result.z = 2.0 * r_xy * z * Z_mul;

    return result;
}

// Formula metadata
constant FormulaInfo formula_info__SinePow2 = {
    .id = FORMULA__SINEPOW2,
    .name = "_SinePow2",
    .param_count = 2,
    .complexity = 76,
    .uses_transcendental = false
};

/*
Original M3 Description:
Trigonometry-free power-2 Mandelbulb using double-angle identities.

Mathematical Formula:
r_xy = sqrt(x²+y²)
t = 1 - z²/(x²+y²)
x' = (x²-y²) * t
y' = 2*x*y * t * Y_mul
z' = 2 * r_xy * z * Z_mul

Parameters:
  [0] Y_multiplier: double (default: 1)
  [1] Z_multiplier: double (default: -1)
*/
