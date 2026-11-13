// _sphereXinv.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _sphereXinv
// Hand-ported from ASM: 419 bytes, DEoption=-1

constexpr unsigned int FORMULA__SPHEREXINV = 7052;

// ASM trace:
//   Translate: x'=x+Xctr, y'=y+Yctr, z'=z+Zctr (stored back)
//   rho = x'² + y'² + z'²
//   x'': DD00/D8F9 = FDIVR st0,st1 → rho/x' (reversed!)
//        * Radius - Xctr → store x''
//   y'': DD02/D8F1 = FDIV st0,st1 → y'/rho (normal)
//        * Radius - Yctr → store y''
//   z'': DD01/D9C9/DEF9 = FDIVP normal → z'/rho
//        * Radius - Zctr → store z''
//
// "Coding error" sphere inversion: x gets rho/x instead of x/rho.

template<>
cpu_double3 execute_formula<FORMULA__SPHEREXINV>(cpu_double3 p, constant double* params, double &de) {
    const double Xctr   = params[0];  // default: 0
    const double Yctr   = params[1];  // default: 0
    const double Zctr   = params[2];  // default: 0
    const double Radius = params[3];  // default: 1

    double x = p.x + Xctr;
    double y = p.y + Yctr;
    double z = p.z + Zctr;

    double rho = x*x + y*y + z*z;

    double new_x = (rho / x) * Radius - Xctr;  // reversed: rho/x
    double new_y = (y / rho) * Radius - Yctr;  // normal: y/rho
    double new_z = (z / rho) * Radius - Zctr;  // normal: z/rho

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info__sphereXinv = {
    .id = FORMULA__SPHEREXINV,
    .name = "_sphereXinv",
    .param_count = 4,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
Born from a coding error while implementing sphere inversion.
Gives fuzzy but interesting effects.
Formula by Luca GN 2011.

Translate: x'=x+Xctr, y'=y+Yctr, z'=z+Zctr
rho = x'² + y'² + z'²
x'' = (rho/x') * R - Xctr    (reversed division for x!)
y'' = (y'/rho) * R - Yctr    (normal)
z'' = (z'/rho) * R - Zctr    (normal)

Parameters:
  [0] X center: double (default: 0)
  [1] Y center: double (default: 0)
  [2] Z center: double (default: 0)
  [3] Radius: double (default: 1)
*/
