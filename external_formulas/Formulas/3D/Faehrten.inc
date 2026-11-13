// Faehrten.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Faehrten
// Hand-ported from ASM: 106 bytes, DEscale=1

constexpr unsigned int FORMULA_FAEHRTEN = 7057;

// ASM trace:
//   new_x = x * (y*Y_mul + z*Z_mul) + Cx
//   new_y = y² + (x²+z²)*Sq_mul + x*z*Twist + Cy
//   new_z = z² + (x²+y²)*Sq_mul + x*y*Twist + Cz
//
// Fahrten-class formula with cross-product twist.
// "Squares multiplier" blends squared terms; "Twist" adds cross-product rotation.

template<>
cpu_double3 execute_formula<FORMULA_FAEHRTEN>(cpu_double3 p, constant double* params, double &de) {
    const double Y_mul  = params[0];  // default: 1
    const double Z_mul  = params[1];  // default: 0.5
    const double Sq_mul = params[2];  // default: -1/3
    const double Twist  = params[3];  // default: 0

    double x = p.x, y = p.y, z = p.z;
    double x2 = x*x, y2 = y*y, z2 = z*z;

    double new_x = x * (y * Y_mul + z * Z_mul);
    double new_y = y2 + (x2 + z2) * Sq_mul + x * z * Twist;
    double new_z = z2 + (x2 + y2) * Sq_mul + x * y * Twist;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Faehrten = {
    .id = FORMULA_FAEHRTEN,
    .name = "Faehrten",
    .param_count = 4,
    .complexity = 106,
    .uses_transcendental = false
};

/*
Original M3 Description:
Fahrten-class formula with cross-product twist term.

x' = x * (y * Y_mul + z * Z_mul)    (+ Cx from engine)
y' = y² + (x²+z²) * Sq_mul + x*z * Twist   (+ Cy)
z' = z² + (x²+y²) * Sq_mul + x*y * Twist   (+ Cz)

Parameters:
  [0] Y multiplier: double (default: 1)
  [1] Z multiplier: double (default: 0.5)
  [2] Squares multiplier: double (default: -0.33333...)
  [3] Twist: double (default: 0)
*/
