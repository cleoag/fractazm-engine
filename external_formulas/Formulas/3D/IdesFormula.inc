// IdesFormula.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: IdesFormula
// Hand-ported from ASM: 315 bytes, DEscale=1

constexpr unsigned int FORMULA_IDESFORMULA = 7046;

// ASM trace (note: edi=context, esi=PVar):
//   DD01/DD02/DD00   push z, y, x
//   D9C0/D8CA/D8CB   x; xy; xyz
//   DC4EE8           Y_mul * xyz
//   DC4720           + Cy
//   DD1A             store y'
//
//   D8C8/D9C9/D8C8/D9CA/D8C8   x²; y²; z²
//
//   D9C0/D8C3        z²+y²
//   DC4ED8           * Xsub_mul
//   D9C2/DC4EF0      x² * X_mul
//   DEE1             X_mul*x² - (z²+y²)*Xsub_mul
//   DC4718           + Cx
//   DD18             store x'
//
//   DC4EE0           Z_mul*z²
//   D9CA/DEC1        x²+y²
//   DC4ED0           * Zsub_mul
//   DEE9             Z_mul*z² - (x²+y²)*Zsub_mul
//   DC4728           + Cz
//   DD19             store z'
//
// Trafassel's Ide's Formula (FractalForums).

template<>
cpu_double3 execute_formula<FORMULA_IDESFORMULA>(cpu_double3 p, constant double* params, double &de) {
    double x = p.x, y = p.y, z = p.z;

    const double X_mul    = params[0];  // default: 1.0
    const double Y_mul    = params[1];  // default: 2.0
    const double Z_mul    = params[2];  // default: 1.0
    const double Xsub_mul = params[3];  // default: 0.5
    const double Zsub_mul = params[4];  // default: 0.5

    double x2 = x*x, y2 = y*y, z2 = z*z;

    double new_x = X_mul * x2 - (y2 + z2) * Xsub_mul;
    double new_y = Y_mul * x * y * z;
    double new_z = Z_mul * z2 - (x2 + y2) * Zsub_mul;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_IdesFormula = {
    .id = FORMULA_IDESFORMULA,
    .name = "IdesFormula",
    .param_count = 5,
    .complexity = 85,
    .uses_transcendental = false
};

/*
Original M3 Description:
A formula made by Trafassel, the original Ide's Formula:
http://www.fractalforums.com/videos/formula-21-(julia-set-interpretation)/

x' = X_mul * x² - (y² + z²) * Xsub_mul + Cx
y' = Y_mul * x*y*z + Cy
z' = Z_mul * z² - (x² + y²) * Zsub_mul + Cz

Parameters:
  [0] X mul: double (default: 1.0)
  [1] Y mul: double (default: 2.0)
  [2] Z mul: double (default: 1.0)
  [3] Xsub mul: double (default: 0.5)
  [4] Zsub mul: double (default: 0.5)
*/
