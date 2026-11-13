// _gnarl3Dfast.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _gnarl3Dfast
// Hand-ported from ASM: 235 bytes, DEoption=-1
// 3D gnarl attractor transform using polynomial sin approximation
// Cyclic triple-nested sin: x perturbed by z, y by x, z by y

constexpr unsigned int FORMULA__GNARL3DFAST = 4282;

// Polynomial sin approximation (max error ~1e-6, 7th-order)
// Subroutine at 0xBC: sin_approx only (no cos variant in this formula)
static inline double gnarl3d_sin_approx(double x) {
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

// ASM trace (235 bytes):
//   Three identical blocks, each using triple-nested sin of a DIFFERENT axis:
//
//   Block 1 (x output, perturbed by z):
//     S1 = sin(z * Beta)
//     S2 = sin(Alpha * (z + S1))
//     S3 = sin(z + S2)
//     new_x = Strength * (x - X_step * S3)
//
//   Block 2 (y output, perturbed by x):
//     S1 = sin(x * Beta)
//     S2 = sin(Alpha * (x + S1))
//     S3 = sin(x + S2)
//     new_y = Strength * (y - Y_step * S3)
//
//   Block 3 (z output, perturbed by y):
//     S1 = sin(y * Beta)
//     S2 = sin(Alpha * (y + S1))
//     S3 = sin(y + S2)
//     new_z = Strength * (z - Z_step * S3)
//
//   Cyclic: x←z, y←x, z←y
//
// C handling: DEoption=-1 (transform, no C addition)

template<>
cpu_double3 execute_formula<FORMULA__GNARL3DFAST>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // [PVar-0x10] (default: 1.03)
    const double X_step   = params[1];  // [PVar-0x18] (default: 0.1)
    const double Y_step   = params[2];  // [PVar-0x20] (default: 0.1)
    const double Z_step   = params[3];  // [PVar-0x28] (default: 0.1)
    const double Alpha    = params[4];  // [PVar-0x30] (default: 3.0)
    const double Beta     = params[5];  // [PVar-0x38] (default: 2.0)

    double x = p.x, y = p.y, z = p.z;

    // x output: triple-nested sin of z
    double s1z = gnarl3d_sin_approx(z * Beta);
    double s2z = gnarl3d_sin_approx(Alpha * (z + s1z));
    double s3z = gnarl3d_sin_approx(z + s2z);
    double new_x = Strength * (x - X_step * s3z);

    // y output: triple-nested sin of x
    double s1x = gnarl3d_sin_approx(x * Beta);
    double s2x = gnarl3d_sin_approx(Alpha * (x + s1x));
    double s3x = gnarl3d_sin_approx(x + s2x);
    double new_y = Strength * (y - Y_step * s3x);

    // z output: triple-nested sin of y
    double s1y = gnarl3d_sin_approx(y * Beta);
    double s2y = gnarl3d_sin_approx(Alpha * (y + s1y));
    double s3y = gnarl3d_sin_approx(y + s2y);
    double new_z = Strength * (z - Z_step * s3y);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__gnarl3Dfast = {
    .id = FORMULA__GNARL3DFAST,
    .name = "_gnarl3Dfast",
    .param_count = 6,
    .complexity = 235,
    .uses_transcendental = false
};

/*
Original M3 Description:
See Gnarl3D, this is a polynomial version,
faster and accurate (1e-6 MAX error, stopping at x^7)
- no visible difference in practice!
No sin() functions are used.
The numerical coefficients are taken from
the tables found here:
http://www.geometrictools.com/GTEngine/Include/GteConstants.h

3D generalization of the gnarl attractor. Each coordinate
is perturbed by a triple-nested sin of a DIFFERENT coordinate,
creating a cyclic dynamic system:
  x' = S * (x - X_step * sin(z + sin(A*(z + sin(z*B)))))
  y' = S * (y - Y_step * sin(x + sin(A*(x + sin(x*B)))))
  z' = S * (z - Z_step * sin(y + sin(A*(y + sin(y*B)))))

Cyclic mapping: x←z, y←x, z←y

Parameters:
  [0] Strength: double (default: 1.03) - overall scale
  [1] X_step: double (default: 0.1) - x perturbation amplitude
  [2] Y_step: double (default: 0.1) - y perturbation amplitude
  [3] Z_step: double (default: 0.1) - z perturbation amplitude
  [4] Alpha: double (default: 3.0) - middle nesting frequency
  [5] Beta: double (default: 2.0) - inner nesting frequency

Constants (hardcoded polynomial coefficients):
  [PVar+0x00] = 1/pi = 0.31830988618379067
  [PVar+0x08] = pi = 3.1415926535897932
  [PVar+0x10] = -5.167375251323450
  [PVar+0x18] = 2.543310591226590
  [PVar+0x20] = -0.557168173477934
*/
