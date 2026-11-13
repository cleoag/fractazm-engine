// Beth1522.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth1522
// Hand-ported from ASM: 298 bytes, DEoption=0
// Mandalabeth formula by Bethchen (fractalforums.com)

constexpr unsigned int FORMULA_BETH1522 = 5304;

// ASM trace:
//   Phase 1: compute powers  X=x², X2=x⁴, Y=y², Y2=y⁴, Z=z², Z2=z⁴
//   Phase 2: cyclic polynomial for each coordinate (5 params: a,b,c,d,e)
//     inner_x = e*Y*Z - (c*Y + b*Z)*X + X2
//     poly_x = b*Y2 + c*Z2 - d*inner_x
//     val_x = a * x * poly_x
//     new_x = Cx - val_x   (fsubr)
//     (y,z similarly with cyclic permutation of squared vars)
//
// C handling: Uses fsubr (Cx - val). Since DEoption=0, engine adds C,
// so we return -val so engine gives (-val)+Cx = Cx-val.

template<>
cpu_double3 execute_formula<FORMULA_BETH1522>(cpu_double3 p, constant double* params, double &de) {
    // These are [CONSTANTS] (hardcoded in ASM at positive PVar offsets), NOT user parameters
    const double a = 0.15625;
    const double b = 20.652475842498528;
    const double c = -10.652475842498528;
    const double d = 2.0;
    const double e = 30.0;

    double x = p.x, y = p.y, z = p.z;

    // Compute even powers
    double X = x * x;   // x²
    double Y = y * y;   // y²
    double Z = z * z;   // z²

    double X2 = X * X;  // x⁴
    double Y2 = Y * Y;  // y⁴
    double Z2 = Z * Z;  // z⁴

    // Cyclic polynomial for x (self=X, others=Y,Z)
    double inner_x = e * Y * Z - (c * Y + b * Z) * X + X2;
    double poly_x = b * Y2 + c * Z2 - d * inner_x;

    // Cyclic polynomial for y (self=Y, others=Z,X)
    double inner_y = e * Z * X - (c * Z + b * X) * Y + Y2;
    double poly_y = b * Z2 + c * X2 - d * inner_y;

    // Cyclic polynomial for z (self=Z, others=X,Y)
    double inner_z = e * X * Y - (c * X + b * Y) * Z + Z2;
    double poly_z = b * X2 + c * Y2 - d * inner_z;

    // Output: negated because ASM uses fsubr (Cx - val)
    double new_x = -(a * x * poly_x);
    double new_y = -(a * y * poly_y);
    double new_z = -(a * z * poly_z);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Beth1522 = {
    .id = FORMULA_BETH1522,
    .name = "Beth1522",
    .param_count = 0,
    .complexity = 298,
    .uses_transcendental = false
};

/*
Original M3 Description:
This is one of the 'Mandalabeth' formulas made by Bethchen at fractalforums.com.

Cyclic polynomial formula with reverse-subtract C handling. Each output
coordinate is -(a * coord * polynomial), where the polynomial uses
differences of 4th powers and a sub-expression involving cross products
of squared coordinates.

Parameters:
  [0] Double: double (default: 0.15625)
  [1] Double: double (default: 20.652475842498527874864215681119)
  [2] Double: double (default: -10.652475842498527874864215681119)
  [3] Double: double (default: 2)
  [4] Double: double (default: 30)
*/
