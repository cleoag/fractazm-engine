// _gnarl2D_2fast.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _gnarl2D_2fast
// Hand-ported from ASM: 174 bytes, DEoption=-1
// Asymmetric 2D gnarl attractor variant using polynomial sin/cos approximation
// Cross-axis perturbation with different nesting structures per axis

constexpr unsigned int FORMULA__GNARL2D_2FAST = 7921;

// Polynomial sin approximation (max error ~1e-6, 7th-order)
// Same subroutine as _gnarl2Dfast and _dynamicfast
// Two entry points: sin_approx (0x7) and cos_approx (0x7C = sin(x + pi/2))
static inline double gnarl2d2_sin_approx(double x) {
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

static inline double gnarl2d2_cos_approx(double x) {
    // Entry at 0x7C: adds pi/2 before calling sin
    return gnarl2d2_sin_approx(x + 1.5707963267948966);  // [PVar+0x28] = pi/2
}

// ASM trace (174 bytes):
//   The two halves are ASYMMETRIC — different nesting order and sign patterns:
//
//   First half (computing new x, stored to [ebx]):
//     inner = Alpha * sin(x + Beta * cos(x))
//     outer = sin((y+x)*Gamma - inner)
//     val1 = y + X_step * outer
//
//   Second half (computing new y, stored to [edx]):
//     inner = sin(Alpha * (y + Beta * cos(y)))
//     outer = sin((x-y)*Gamma + inner)
//     val2 = x + X_step * outer
//
//   Final:
//     new_x = Strength * val1
//     new_y = Strength * val2
//     z unchanged
//
//   Note: Y_step parameter exists in M3 but is NOT used by this formula.
//   Both axes use X_step.
//
// C handling: DEoption=-1 (transform, no C addition)

template<>
cpu_double3 execute_formula<FORMULA__GNARL2D_2FAST>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [PVar-0x10] (default: 1.0)
    const double X_step   = params[1];  // [PVar-0x18] (default: -0.1)
    // params[2] = Y_step (default: -0.1) — UNUSED by this formula
    const double Alpha    = params[3];  // [PVar-0x28] (default: 2.0)
    const double Beta     = params[4];  // [PVar-0x30] (default: -4.0)
    const double Gamma    = params[5];  // [PVar-0x38] (default: -0.1)

    double x = p.x, y = p.y, z = p.z;

    // First half: new_x (asymmetric pattern A)
    //   Alpha * sin(x + Beta*cos(x))  then  sin((y+x)*Gamma - that)
    double inner1 = Alpha * gnarl2d2_sin_approx(x + Beta * gnarl2d2_cos_approx(x));
    double outer1 = gnarl2d2_sin_approx((y + x) * Gamma - inner1);
    double new_x = Strength * (y + X_step * outer1);

    // Second half: new_y (asymmetric pattern B)
    //   sin(Alpha*(y + Beta*cos(y)))  then  sin((x-y)*Gamma + that)
    double inner2 = gnarl2d2_sin_approx(Alpha * (y + Beta * gnarl2d2_cos_approx(y)));
    double outer2 = gnarl2d2_sin_approx((x - y) * Gamma + inner2);
    double new_y = Strength * (x + X_step * outer2);

    // z unchanged
    return cpu_double3(new_x, new_y, z);
}

// Formula metadata
constant FormulaInfo formula_info__gnarl2D_2fast = {
    .id = FORMULA__GNARL2D_2FAST,
    .name = "_gnarl2D_2fast",
    .param_count = 6,
    .complexity = 174,
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

This is an asymmetric variant of the 2D gnarl attractor.
Unlike _gnarl2Dfast which uses the same triple-nested sin
for both axes, this variant uses different nesting structures:

  x' = S * (y + X_step * sin((y+x)*G - A*sin(x + B*cos(x))))
  y' = S * (x + X_step * sin((x-y)*G + sin(A*(y + B*cos(y)))))
  z' = z  (unchanged)

The key asymmetries:
  1. Nesting order differs: first half has Alpha outside sin,
     second half has Alpha inside sin
  2. Sum vs difference: (y+x)*Gamma vs (x-y)*Gamma
  3. Subtract vs add: inner term subtracted vs added
  4. Cross-axis: x output uses y as base, y output uses x as base

Parameters:
  [0] Strength: double (default: 1.0) - overall scale
  [1] X_step: double (default: -0.1) - perturbation amplitude (used for both axes)
  [2] Y_step: double (default: -0.1) - UNUSED (exists for M3 compatibility)
  [3] Alpha: double (default: 2.0) - nesting frequency/amplitude
  [4] Beta: double (default: -4.0) - inner cosine frequency
  [5] Gamma: double (default: -0.1) - linear combination frequency

Constants (hardcoded polynomial coefficients):
  [PVar+0x00] = 1/pi = 0.31830988618379067
  [PVar+0x08] = pi = 3.1415926535897932
  [PVar+0x10] = -5.167375251323450
  [PVar+0x18] = 2.543310591226590
  [PVar+0x20] = -0.557168173477934
  [PVar+0x28] = pi/2 = 1.5707963267948966
*/
