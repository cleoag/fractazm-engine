// Beth322.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Beth322
// Hand-ported from ASM: 126 bytes, DEoption=0

constexpr unsigned int FORMULA_BETH322 = 7079;

// ASM trace:
//   x2=x², y2=y², z2=z², x4=x⁴, y4=y⁴, z4=z⁴
//   Qx = 2*x2*(y2+z2) - (y4+z4)
//   new_x = x * (2*x4 - 5*Qx) + Cx
//   Qy = 2*y2*(x2+z2) - (x4+z4)
//   new_y = y * (2*y4 - 5*Qy) + Cy
//   Qz = 2*z2*(x2+y2) - (x4+y4)
//   new_z = z * (2*z4 - 5*Qz) + Cz
//
// Power-5 polynomial fractal (Beth 3-2-2 pattern).
// Each axis: coord * (2*coord⁴ - 5*(2*coord²*other_sum - other_pow4_sum))
// Cyclic structure across all three axes.
//
// C handling: Cx, Cy, Cz all added inline by formula.

template<>
cpu_double3 execute_formula<FORMULA_BETH322>(cpu_double3 p, constant double* params, double &de) {
    // Constant from m3 [CONSTANTS] section
    const double K = 5.0;  // at [PVar+0x00]

    double x = p.x, y = p.y, z = p.z;

    double x2 = x * x, y2 = y * y, z2 = z * z;
    double x4 = x2 * x2, y4 = y2 * y2, z4 = z2 * z2;

    // Cyclic quadratic terms
    double Qx = 2.0 * x2 * (y2 + z2) - (y4 + z4);
    double Qy = 2.0 * y2 * (x2 + z2) - (x4 + z4);
    double Qz = 2.0 * z2 * (x2 + y2) - (x4 + y4);

    // Power-5 output: coord * (2*coord⁴ - K*Q)
    double new_x = x * (2.0 * x4 - K * Qx);
    double new_y = y * (2.0 * y4 - K * Qy);
    double new_z = z * (2.0 * z4 - K * Qz);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Beth322 = {
    .id = FORMULA_BETH322,
    .name = "Beth322",
    .param_count = 0,
    .complexity = 126,
    .uses_transcendental = false
};

/*
Original M3 Description:
Power-5 polynomial fractal formula (Beth 3-2-2 pattern).

Each output is:
  Qx = 2*x²*(y²+z²) - (y⁴+z⁴)
  new_x = x * (2*x⁴ - 5*Qx)

Expanded: new_x = x * (2*x⁴ - 10*x²*(y²+z²) + 5*(y⁴+z⁴))

Cyclic structure: same formula rotated for y and z axes.

C handling: Cx, Cy, Cz all added inline.
SI_Power: 3.0 (used for DE scaling).

Constants (from m3 [CONSTANTS]):
  5.0 (at [PVar+0x00])

Parameters: None
*/
