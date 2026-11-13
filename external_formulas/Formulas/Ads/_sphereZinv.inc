// _sphereZinv.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _sphereZinv
// Hand-ported from ASM: 419 bytes, DEoption=-1

constexpr unsigned int FORMULA__SPHEREZINV = 7054;

// ASM trace: Same as _sphereXinv but z gets the reversed division.
//   x'': D8F1 = FDIV → x'/rho (normal)
//   y'': D8F1 = FDIV → y'/rho (normal)
//   z'': DEF1 = FDIVRP → rho/z' (reversed!)

template<>
cpu_double3 execute_formula<FORMULA__SPHEREZINV>(cpu_double3 p, constant double* params, double &de) {
    const double Xctr   = params[0];  // default: 0
    const double Yctr   = params[1];  // default: 0
    const double Zctr   = params[2];  // default: 0
    const double Radius = params[3];  // default: 1

    double x = p.x + Xctr;
    double y = p.y + Yctr;
    double z = p.z + Zctr;

    double rho = x*x + y*y + z*z;

    double new_x = (x / rho) * Radius - Xctr;  // normal: x/rho
    double new_y = (y / rho) * Radius - Yctr;  // normal: y/rho
    double new_z = (rho / z) * Radius - Zctr;  // reversed: rho/z

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__sphereZinv = {
    .id = FORMULA__SPHEREZINV,
    .name = "_sphereZinv",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
Born from a coding error while implementing sphere inversion.
Formula by Luca GN 2011.

rho = x'² + y'² + z'²
x'' = (x'/rho) * R - Xctr    (normal)
y'' = (y'/rho) * R - Yctr    (normal)
z'' = (rho/z') * R - Zctr    (reversed division for z!)

Parameters:
  [0] X center: double (default: 0)
  [1] Y center: double (default: 0)
  [2] Z center: double (default: 0)
  [3] Radius: double (default: 1)
*/
