// _dynamicfast.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _dynamicfast
// Hand-ported from ASM: 190 bytes, DEoption=-1
// 3D dynamic system transform using polynomial sin approximation
// Cyclic perturbation: each axis is disturbed by cos/sin of the other two

constexpr unsigned int FORMULA__DYNAMICFAST = 4097;

// Polynomial sin approximation (max error ~1e-6, 7th-order)
// Matches x87 fist/fild + polynomial evaluation + odd-negate pattern
// Coefficients from GteConstants.h
//
// Subroutine at 0x8 (sin_approx) and 0x8c (cos_approx = sin(x + pi/2)):
//   t = x * (1/pi)
//   n = round_to_int(t)
//   frac = t - n
//   poly = frac * (pi + C*frac^2 + B*frac^4 + A*frac^6)
//   result = (n & 1) ? -poly : poly
static inline double sin_approx(double x) {
    const double inv_pi = 0.31830988618379067;  // [PVar+0x00]
    const double D = 3.1415926535897932;         // [PVar+0x08] = pi
    const double C = -5.167375251323450;         // [PVar+0x10]
    const double B = 2.543310591226590;          // [PVar+0x18]
    const double A = -0.557168173477934;         // [PVar+0x20]

    double t = x * inv_pi;
    int n = int(rint(t));            // fist/fild pattern (round to nearest)
    double frac = t - double(n);

    double f2 = frac * frac;
    double poly = frac * (((A * f2 + B) * f2 + C) * f2 + D);

    return (n & 1) ? -poly : poly;  // negate for odd integer part
}

static inline double cos_approx(double x) {
    // Entry at 0x8c: adds pi/2 before calling sin
    return sin_approx(x + 1.5707963267948966);  // [PVar+0x28] = pi/2
}

// ASM trace:
//   For each axis, the pattern is:
//     cos_approx(other1 * Gamma) * Alpha + sin_approx(other2 * Delta) * Beta
//     negate, add self, multiply by Strength
//
//   x' = Strength * (x - (Alpha*cos(y*Gamma) + Beta*sin(z*Delta)))
//   y' = Strength * (y - (Alpha*cos(z*Gamma) + Beta*sin(x*Delta)))
//   z' = Strength * (z - (Alpha*cos(x*Gamma) + Beta*sin(y*Delta)))
//
//   Cyclic: x uses (y,z), y uses (z,x), z uses (x,y)
//
// C handling: DEoption=-1 (transform, no C addition)

template<>
cpu_double3 execute_formula<FORMULA__DYNAMICFAST>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [PVar-0x10] (default: 1.0)
    const double Alpha    = params[1];  // [PVar-0x18] (default: 0.1)
    const double Beta     = params[2];  // [PVar-0x20] (default: 0.1)
    const double Gamma    = params[3];  // [PVar-0x28] (default: 1.5)
    const double Delta    = params[4];  // [PVar-0x30] (default: 1.5)

    double x = p.x, y = p.y, z = p.z;

    // x output: x - (Alpha*cos(y*Gamma) + Beta*sin(z*Delta))
    double new_x = Strength * (x - (Alpha * cos_approx(y * Gamma)
                                  + Beta  * sin_approx(z * Delta)));

    // y output: y - (Alpha*cos(z*Gamma) + Beta*sin(x*Delta))
    double new_y = Strength * (y - (Alpha * cos_approx(z * Gamma)
                                  + Beta  * sin_approx(x * Delta)));

    // z output: z - (Alpha*cos(x*Gamma) + Beta*sin(y*Delta))
    double new_z = Strength * (z - (Alpha * cos_approx(x * Gamma)
                                  + Beta  * sin_approx(y * Delta)));

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__dynamicfast = {
    .id = FORMULA__DYNAMICFAST,
    .name = "_dynamicfast",
    .param_count = 5,
    .complexity = 190,
    .uses_transcendental = false
};

/*
Original M3 Description:
See _gnarl2D_2, this is a polynomial version,
faster and accurate (1e-6 MAX error, stopping at x^7)
- no visible difference in practice!
No sin() functions are used.
The numerical coefficients are taken from
the tables found here:
http://www.geometrictools.com/GTEngine/Include/GteConstants.h

This is a 3D generalization of the gnarl attractor. Each coordinate
is perturbed by cos/sin functions of the other two coordinates,
creating a cyclic dynamic system:
  x' = S * (x - (A*cos(y*G) + B*sin(z*D)))
  y' = S * (y - (A*cos(z*G) + B*sin(x*D)))
  z' = S * (z - (A*cos(x*G) + B*sin(y*D)))

The sin/cos functions are computed using a 7th-order polynomial
approximation with coefficients from GteConstants.h, achieving
~1e-6 max error compared to true sin/cos.

Parameters:
  [0] Strength: double (default: 1.0) - overall scale factor
  [1] Alpha: double (default: 0.1) - cosine amplitude
  [2] Beta: double (default: 0.1) - sine amplitude
  [3] Gamma: double (default: 1.5) - cosine frequency
  [4] Delta: double (default: 1.5) - sine frequency

Constants (hardcoded polynomial coefficients):
  [PVar+0x00] = 1/pi = 0.31830988618379067
  [PVar+0x08] = pi = 3.1415926535897932
  [PVar+0x10] = -5.167375251323450
  [PVar+0x18] = 2.543310591226590
  [PVar+0x20] = -0.557168173477934
  [PVar+0x28] = pi/2 = 1.5707963267948966
*/
