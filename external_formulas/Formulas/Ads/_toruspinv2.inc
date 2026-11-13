// _toruspinv2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _toruspinv2
// Hand-ported from ASM: 111 bytes, DEoption=-1

constexpr unsigned int FORMULA__TORUSPINV2 = 7068;

// ASM trace:
//   Identical to _toruspinv1 except z computation:
//   _toruspinv1: z' = Radius / (d * |z|) - Zctr   (DE F9 = fdivp)
//   _toruspinv2: z' = Radius * d * |z| - Zctr      (DE C9 = fmulp)
//
//   d = sqrt((R - (x²+y²))² + z²)
//   x' = |x| * Radius / d - Xctr
//   y' = |y| * Radius / d - Yctr
//   z' = Radius * d * |z| - Zctr    (multiply instead of divide)

template<>
cpu_double3 execute_formula<FORMULA__TORUSPINV2>(cpu_double3 p, constant double* params, double &de) {
    const double Xctr   = params[0];
    const double Yctr   = params[1];
    const double Zctr   = params[2];
    const double Radius = params[3];
    const double R      = params[4];

    double x = p.x + Xctr;
    double y = p.y + Yctr;
    double z = p.z + Zctr;

    double t = R - (x*x + y*y);
    double d = sqrt(t*t + z*z);

    double new_x = abs(x) * Radius / d - Xctr;
    double new_y = abs(y) * Radius / d - Yctr;
    double new_z = Radius * d * abs(z) - Zctr;  // multiply (not divide)

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__toruspinv2 = {
    .id = FORMULA__TORUSPINV2,
    .name = "_toruspinv2",
    .param_count = 5,
    .complexity = 111,
    .uses_transcendental = false
};

/*
Original M3 Description:
Torus pseudo-inversion variant 2. Same as _toruspinv1 except:
z uses multiply instead of divide (Radius * d * |z| instead of Radius / (d * |z|)).
Formula by Luca GN 2011.

Parameters:
  [0] X center: double (default: 0)
  [1] Y center: double (default: 0)
  [2] Z center: double (default: 0)
  [3] Radius: double (default: 1)
  [4] R: double (default: 1)
*/
