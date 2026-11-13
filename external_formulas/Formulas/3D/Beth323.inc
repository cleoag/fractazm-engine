// Beth323.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth323
// Hand-ported from ASM: 262 bytes, DEoption=0
// Mandalabeth formula by Bethchen (fractalforums.com)

constexpr unsigned int FORMULA_BETH323 = 742;

// ASM trace:
//   Phase 1: compute squares (overwrite xyz, save originals)
//     X = x², Y = y², Z = z²  (also save x_orig, y_orig, z_orig)
//   Phase 2: cyclic polynomial for each coordinate
//     poly_x = a*(Y-Z)*X² + b*(Y²-Z²)*X + c*(Y³-Z³)
//     new_x = x_orig * poly_x + Cx
//     poly_y = a*(Z-X)*Y² + b*(Z²-X²)*Y + c*(Z³-X³)
//     new_y = y_orig * poly_y + Cy
//     poly_z = a*(X-Y)*Z² + b*(X²-Y²)*Z + c*(X³-Y³)
//     new_z = z_orig * poly_z + Cz
//
// C handling: Standard (DEoption=0), engine adds C after return.
// But ASM explicitly adds Cx/Cy/Cz, so this formula includes C.

template<>
cpu_double3 execute_formula<FORMULA_BETH323>(cpu_double3 p, constant double* params, double &de) {
    // These are [CONSTANTS] (hardcoded in ASM at positive PVar offsets), NOT user parameters
    const double a = 21.0;
    const double b = -35.0;
    const double c = 7.0;

    double x = p.x, y = p.y, z = p.z;

    // Compute even powers
    double X = x * x;   // x²
    double Y = y * y;   // y²
    double Z = z * z;   // z²

    double X2 = X * X;  // x⁴
    double Y2 = Y * Y;  // y⁴
    double Z2 = Z * Z;  // z⁴

    double X3 = X2 * X; // x⁶
    double Y3 = Y2 * Y; // y⁶
    double Z3 = Z2 * Z; // z⁶

    // Cyclic polynomial: each coordinate uses differences of the other two
    double poly_x = a * (Y - Z) * X2 + b * (Y2 - Z2) * X + c * (Y3 - Z3);
    double poly_y = a * (Z - X) * Y2 + b * (Z2 - X2) * Y + c * (Z3 - X3);
    double poly_z = a * (X - Y) * Z2 + b * (X2 - Y2) * Z + c * (X3 - Y3);

    // Multiply by original coordinate (NOT squared)
    double new_x = x * poly_x;
    double new_y = y * poly_y;
    double new_z = z * poly_z;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Beth323 = {
    .id = FORMULA_BETH323,
    .name = "Beth323",
    .param_count = 0,
    .complexity = 262,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the 'Mandalabeth' formulas made by Bethchen at fractalforums.com.

Cyclic polynomial formula. Each output coordinate is the original coordinate
times a polynomial in (x², y², z²) that uses differences of powers of the
other two squared coordinates.

Parameters:
  [0] Double: double (default: 21)
  [1] Double: double (default: -35)
  [2] Double: double (default: 7)
*/
