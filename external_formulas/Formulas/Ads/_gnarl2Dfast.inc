// _gnarl2Dfast.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _gnarl2Dfast
// Hand-ported from ASM: 182 bytes, DEoption=-1
// 2D gnarl attractor transform using polynomial sin approximation
// Triple-nested sin of the OTHER coordinate perturbs each axis

constexpr unsigned int FORMULA__GNARL2DFAST = 6364;

// Polynomial sin approximation (max error ~1e-6, 7th-order)
// Matches x87 fist/fild + polynomial evaluation + odd-negate pattern
// Entry at 0x87: sin_approx(x)
//   t = x * (1/pi), n = round(t), frac = t - n
//   poly = frac * (pi + C*frac^2 + B*frac^4 + A*frac^6)
//   result = (n odd) ? -poly : poly
static inline double gnarl2d_sin_approx(double x) {
    const double inv_pi = 0.31830988618379067;  // [PVar+0x00]
    const double D = 3.1415926535897932;         // [PVar+0x08] = pi
    const double C = -5.167375251323450;         // [PVar+0x10]
    const double B = 2.543310591226590;          // [PVar+0x18]
    const double A = -0.557168173477934;         // [PVar+0x20]

    double t = x * inv_pi;
    int n = int(rint(t));
    double frac = t - double(n);

    double f2 = frac * frac;
    double poly = frac * (((A * f2 + B) * f2 + C) * f2 + D);

    return (n & 1) ? -poly : poly;
}

// ASM trace:
//   The formula uses only sin_approx (entry 0x87), no cos variant.
//   Triple-nested sin pattern for each axis:
//
//   For x output (using y):
//     S1 = sin(y * Beta)
//     S2 = sin(Alpha * (y + S1))
//     S3 = sin(y + S2)
//     new_x = Strength * (x - X_step * S3)
//
//   For y output (using x):
//     S1 = sin(x * Beta)
//     S2 = sin(Alpha * (x + S1))
//     S3 = sin(x + S2)
//     new_y = Strength * (y - Y_step * S3)
//
//   z is unchanged.
//
// C handling: DEoption=-1 (transform, no C addition)

template<>
cpu_double3 execute_formula<FORMULA__GNARL2DFAST>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [PVar-0x10] (default: 1.03)
    const double X_step   = params[1];  // [PVar-0x18] (default: 0.1)
    const double Y_step   = params[2];  // [PVar-0x20] (default: 0.1)
    const double Alpha    = params[3];  // [PVar-0x28] (default: 3.0)
    const double Beta     = params[4];  // [PVar-0x30] (default: 2.0)

    double x = p.x, y = p.y, z = p.z;

    // x output: triple-nested sin of y
    double s1y = gnarl2d_sin_approx(y * Beta);
    double s2y = gnarl2d_sin_approx(Alpha * (y + s1y));
    double s3y = gnarl2d_sin_approx(y + s2y);
    double new_x = Strength * (x - X_step * s3y);

    // y output: triple-nested sin of x
    double s1x = gnarl2d_sin_approx(x * Beta);
    double s2x = gnarl2d_sin_approx(Alpha * (x + s1x));
    double s3x = gnarl2d_sin_approx(x + s2x);
    double new_y = Strength * (y - Y_step * s3x);

    // z unchanged
    return cpu_double3(new_x, new_y, z);
}

// Formula metadata
constant FormulaInfo formula_info__gnarl2Dfast = {
    .id = FORMULA__GNARL2DFAST,
    .name = "_gnarl2Dfast",
    .param_count = 5,
    .complexity = 182,
    .uses_transcendental = false
};

/*
Original M3 Description:
See Gnarl2D, this is a polynomial version,
faster and accurate (1e-6 MAX error, stopping at x^7)
- no visible difference in practice!
No sin() functions are used.
The numerical coefficients are taken from
the tables found here:
http://www.geometrictools.com/GTEngine/Include/GteConstants.h

The gnarl attractor perturbs each axis by a triple-nested sin
of the OTHER axis:
  x' = Strength * (x - X_step * sin(y + sin(Alpha*(y + sin(y*Beta)))))
  y' = Strength * (y - Y_step * sin(x + sin(Alpha*(x + sin(x*Beta)))))
  z' = z  (unchanged)

This creates organic, gnarl-like patterns in 2D (the x-y plane).

Parameters:
  [0] Strength: double (default: 1.03) - overall scale
  [1] X_step: double (default: 0.1) - x perturbation amplitude
  [2] Y_step: double (default: 0.1) - y perturbation amplitude
  [3] Alpha: double (default: 3.0) - middle nesting frequency
  [4] Beta: double (default: 2.0) - inner nesting frequency

Constants (hardcoded polynomial coefficients):
  [PVar+0x00] = 1/pi = 0.31830988618379067
  [PVar+0x08] = pi = 3.1415926535897932
  [PVar+0x10] = -5.167375251323450
  [PVar+0x18] = 2.543310591226590
  [PVar+0x20] = -0.557168173477934
*/
