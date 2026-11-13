// Makin3D-3-4.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin3D-3-4
// Hand-ported from ASM: 190 bytes, DEoption=0

constexpr unsigned int FORMULA_MAKIN3D_3_4 = 7093;

// ASM trace:
//   new_x = x² - y² - z²
//   z²_s = (Mode1>0) ? -z² : (Mode1==0) ? z² : (z>0 ? z² : -z²)
//   new_y = 2*x*y * (1 + z²_s / (x² + y² + |Epsilon|))
//   y²_s = (Mode2>0) ? -y² : (Mode2==0) ? y² : (y>0 ? y² : -y²)
//   new_z = 2*x*z * (1 + y²_s / (x² + z² + |Epsilon|))
//
// Makin 3D formula variant 3-4. Power-2 base with optional sign
// correction on cross-terms controlled by integer mode parameters.
// The modes control how the "third axis squared" term is signed
// in each cross-product component.
//
// C handling: STANDARD — engine adds C after return.

template<>
cpu_double3 execute_formula<FORMULA_MAKIN3D_3_4>(cpu_double3 p, constant double* params, double &de) {
    const int   Mode1   = int(params[0]);   // sign mode for z² in y formula
    const int   Mode2   = int(params[1]);   // sign mode for y² in z formula
    const double Epsilon = params[2];        // denominator offset (default: small)

    double x = p.x, y = p.y, z = p.z;

    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;

    // x: standard power-2
    double new_x = x2 - y2 - z2;

    // Determine signed z² for y formula
    double z2_s;
    if (Mode1 > 0) {
        z2_s = -z2;
    } else if (Mode1 == 0) {
        z2_s = z2;
    } else {
        z2_s = (z > 0.0) ? z2 : -z2;
    }

    // y: 2xy with z²-based correction
    double denom_y = x2 + y2 + abs(Epsilon);
    double new_y = 2.0 * x * y * (1.0 + z2_s / denom_y);

    // Determine signed y² for z formula
    double y2_s;
    if (Mode2 > 0) {
        y2_s = -y2;
    } else if (Mode2 == 0) {
        y2_s = y2;
    } else {
        y2_s = (y > 0.0) ? y2 : -y2;
    }

    // z: 2xz with y²-based correction
    double denom_z = x2 + z2 + abs(Epsilon);
    double new_z = 2.0 * x * z * (1.0 + y2_s / denom_z);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin3D_3_4 = {
    .id = FORMULA_MAKIN3D_3_4,
    .name = "Makin3D-3-4",
    .param_count = 3,
    .complexity = 190,
    .uses_transcendental = false
};

/*
Original M3 Description:
Makin 3D formula variant 3-4 by Rick Makin.
Power-2 base with configurable sign correction on cross-terms.

new_x = x² - y² - z²
new_y = 2*x*y * (1 + z²_signed / (x² + y² + |Epsilon|))
new_z = 2*x*z * (1 + y²_signed / (x² + z² + |Epsilon|))

Mode controls for sign of "third axis squared":
  > 0: always negate (subtract the correction)
  = 0: always positive (add the correction)
  < 0: conditional on sign of the axis value

The Epsilon parameter prevents division by zero and
controls how strongly the correction affects the output.

C handling: STANDARD (engine adds C).

Parameters:
  [0] Mode1: integer (sign mode for z² in y formula)
  [1] Mode2: integer (sign mode for y² in z formula)
  [2] Epsilon: double (denominator offset, default: small value)
*/
