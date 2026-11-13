// _toruspinv1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _toruspinv1
// Hand-ported from ASM: 111 bytes, DEoption=-1

constexpr unsigned int FORMULA__TORUSPINV1 = 7067;

// ASM trace:
//   Translate: x+=Xctr, y+=Yctr, z+=Zctr
//   d = sqrt((R - (x²+y²))² + z²)    (torus distance-like term)
//
//   x' = |x| * Radius / d - Xctr      (|x|/d, normal inversion)
//   y' = |y| * Radius / d - Yctr      (|y|/d, normal inversion)
//   z' = |z| * d / Radius - Zctr      (|z|*d/Radius, reversed relationship)
//
//   ASM for z:
//     fabs |z|; fxch st(1) → st0=d, st1=|z|; fmulp → st0=|z|*d
//     fld Radius; fdivp st(1) → st0 = (|z|*d)/Radius
//
// Torus pseudo-inversion. z gets reversed relationship: d in numerator.
// Formula by Luca GN 2011.

template<>
cpu_double3 execute_formula<FORMULA__TORUSPINV1>(cpu_double3 p, constant double* params, double &de) {
    const double Xctr   = params[0];  // default: 0
    const double Yctr   = params[1];  // default: 0
    const double Zctr   = params[2];  // default: 0
    const double Radius = params[3];  // default: 1
    const double R      = params[4];  // default: 1 (torus major radius parameter)

    double x = p.x + Xctr;
    double y = p.y + Yctr;
    double z = p.z + Zctr;

    // Torus-like distance computation
    double t = R - (x*x + y*y);  // R minus xy-plane radius²
    double d = sqrt(t*t + z*z);  // distance in torus cross-section

    double new_x = abs(x) * Radius / d - Xctr;
    double new_y = abs(y) * Radius / d - Yctr;
    double new_z = abs(z) * d / Radius - Zctr;  // reversed: d in numerator, Radius in denominator

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__toruspinv1 = {
    .id = FORMULA__TORUSPINV1,
    .name = "_toruspinv1",
    .param_count = 5,
    .complexity = 111,
    .uses_transcendental = false
};

/*
Original M3 Description:
Torus pseudo-inversion in 3D. Combines torus-like distance with
sphere inversion. x and y get normal inversion (|coord|*Radius/d),
z gets reversed relationship (|z|*d/Radius).
Formula by Luca GN 2011.

Translate: x'=x+Xctr, y'=y+Yctr, z'=z+Zctr
d = sqrt((R - (x'²+y'²))² + z'²)
x'' = |x'| * Radius / d - Xctr
y'' = |y'| * Radius / d - Yctr
z'' = |z'| * d / Radius - Zctr

Parameters:
  [0] X center: double (default: 0)
  [1] Y center: double (default: 0)
  [2] Z center: double (default: 0)
  [3] Radius: double (default: 1)
  [4] R: double (default: 1)
*/
