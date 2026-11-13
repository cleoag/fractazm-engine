// Beth432.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth432
// Hand-ported from ASM: 267 bytes, DEoption=0
// Mandalabeth formula by Bethchen (fractalforums.com)

constexpr unsigned int FORMULA_BETH432 = 6646;

// ASM trace:
//   Phase 1: compute powers  x², x⁴, y², y⁴, z², z⁴
//   Phase 2: cyclic polynomial for each coordinate (6 params: a,b,c,d,e,f)
//     poly_x = f*(y⁴z² + y²z⁴) + e*x²*(b*y²z² + y⁴ + z⁴)
//            + d*x⁴*(y² + z²) + 2*x⁶ + c*(y⁶ + z⁶)
//     new_x = a * x * poly_x + Cx
//     (y,z similarly with cyclic permutation x→y→z→x)
//
// C handling: Standard (DEoption=0), engine adds C after return.

template<>
cpu_double3 execute_formula<FORMULA_BETH432>(cpu_double3 p, constant double* params, double &de) {
    // These are [CONSTANTS] (hardcoded in ASM at positive PVar offsets), NOT user parameters
    const double a = -0.395061728395061728395;
    const double b = -12.0;
    const double c = 14.0;
    const double d = -21.0;
    const double e = -35.0;
    const double f = -105.0;

    double x = p.x, y = p.y, z = p.z;

    // Compute even powers
    double x2 = x * x, y2 = y * y, z2 = z * z;  // squares
    double x4 = x2 * x2, y4 = y2 * y2, z4 = z2 * z2;  // 4th powers
    double x6 = x4 * x2, y6 = y4 * y2, z6 = z4 * z2;  // 6th powers

    // Cyclic polynomial for x
    double poly_x = f * (y4 * z2 + y2 * z4)
                 + e * x2 * (b * y2 * z2 + y4 + z4)
                 + d * x4 * (y2 + z2)
                 + 2.0 * x6
                 + c * (y6 + z6);

    // Cyclic polynomial for y (x→y→z→x permutation)
    double poly_y = f * (z4 * x2 + z2 * x4)
                 + e * y2 * (b * z2 * x2 + z4 + x4)
                 + d * y4 * (z2 + x2)
                 + 2.0 * y6
                 + c * (z6 + x6);

    // Cyclic polynomial for z (y→z→x→y permutation)
    double poly_z = f * (x4 * y2 + x2 * y4)
                 + e * z2 * (b * x2 * y2 + x4 + y4)
                 + d * z4 * (x2 + y2)
                 + 2.0 * z6
                 + c * (x6 + y6);

    // Output: a * coord * polynomial
    double new_x = a * x * poly_x;
    double new_y = a * y * poly_y;
    double new_z = a * z * poly_z;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Beth432 = {
    .id = FORMULA_BETH432,
    .name = "Beth432",
    .param_count = 0,
    .complexity = 267,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the 'Mandalabeth' formulas made by Bethchen at fractalforums.com.

Higher-order cyclic polynomial formula. Each output coordinate is the
original coordinate times a degree-6 polynomial (in squared variables)
with cyclic permutation symmetry.

Parameters:
  [0] Double: double (default: -0.395061728395061728395)
  [1] Double: double (default: -12)
  [2] Double: double (default: 14)
  [3] Double: double (default: -21)
  [4] Double: double (default: -35)
  [5] Double: double (default: -105)
*/
