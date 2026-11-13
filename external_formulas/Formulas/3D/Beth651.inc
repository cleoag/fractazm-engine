// Beth651.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth651
// Hand-ported from ASM: 319 bytes, DEoption=0
// Mandalabeth formula by Bethchen (fractalforums.com)

constexpr unsigned int FORMULA_BETH651 = 8913;

// ASM trace:
//   Phase 1: compute powers  X=x², X2=x⁴, Y=y², Y2=y⁴, Z=z², Z2=z⁴
//   Phase 2: cyclic polynomial for each coordinate (8 params: p0..p7)
//     For x-coord (factor=p4*y*z, self=X, others=Y,Z):
//       inner_x = p6*Y*Z - (p7*Y + p0*Z)*X + p3*X2
//       poly_x = p1*Y2 + p5*Z2 - p2*inner_x
//       new_x = p4 * y * z * poly_x + Cx
//     (y,z similarly with cyclic permutation)
//
// C handling: Standard (DEoption=0), engine adds C after return.

template<>
cpu_double3 execute_formula<FORMULA_BETH651>(cpu_double3 p, constant double* params, double &de) {
    // These are [CONSTANTS] (hardcoded in ASM at positive PVar offsets), NOT user parameters
    const double p0 = -2.7639320225002103;
    const double p1 = 1.2360679774997897;
    const double p2 = 2.0;
    const double p3 = 2.2360679774997897;
    const double p4 = 2.4;
    const double p5 = 3.2360679774997897;
    const double p6 = 4.4721359549995794;
    const double p7 = 7.2360679774997897;

    double x = p.x, y = p.y, z = p.z;

    // Compute even powers
    double X = x * x;   // x²
    double Y = y * y;   // y²
    double Z = z * z;   // z²

    double X2 = X * X;  // x⁴
    double Y2 = Y * Y;  // y⁴
    double Z2 = Z * Z;  // z⁴

    // Cyclic polynomial for x (factor=p4*y*z, self=X, others=Y,Z)
    double inner_x = p6 * Y * Z - (p7 * Y + p0 * Z) * X + p3 * X2;
    double poly_x = p1 * Y2 + p5 * Z2 - p2 * inner_x;
    double new_x = p4 * y * z * poly_x;

    // Cyclic polynomial for y (factor=p4*z*x, self=Y, others=Z,X)
    double inner_y = p6 * Z * X - (p7 * Z + p0 * X) * Y + p3 * Y2;
    double poly_y = p1 * Z2 + p5 * X2 - p2 * inner_y;
    double new_y = p4 * z * x * poly_y;

    // Cyclic polynomial for z (factor=p4*x*y, self=Z, others=X,Y)
    double inner_z = p6 * X * Y - (p7 * X + p0 * Y) * Z + p3 * Z2;
    double poly_z = p1 * X2 + p5 * Y2 - p2 * inner_z;
    double new_z = p4 * x * y * poly_z;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Beth651 = {
    .id = FORMULA_BETH651,
    .name = "Beth651",
    .param_count = 0,
    .complexity = 319,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the 'Mandalabeth' formulas made by Bethchen at fractalforums.com.

Higher-order cyclic polynomial formula with golden-ratio-related defaults.
Unlike Beth323/Beth432, the prefactor for each coordinate is p4 times the
product of the OTHER two coordinates (e.g. p4*y*z for x-coord), giving
a different symmetry structure.

Parameters:
  [0] Double: double (default: -2.7639320225002103035908263312687)
  [1] Double: double (default: 1.2360679774997896964091736687313)
  [2] Double: double (default: 2)
  [3] Double: double (default: 2.2360679774997896964091736687313)
  [4] Double: double (default: 2.4)
  [5] Double: double (default: 3.2360679774997896964091736687313)
  [6] Double: double (default: 4.4721359549995793928183473374626)
  [7] Double: double (default: 7.2360679774997896964091736687313)
*/
