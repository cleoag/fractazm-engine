// Benesi2Pow2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Benesi2Pow2
// Hand-ported from ASM: 89 bytes, DEoption=0

constexpr unsigned int FORMULA_BENESI2POW2 = 7084;

// ASM trace:
//   ρ_xy = sqrt(x² + y²)
//   new_x = x² - y² - z² + Cx
//   new_y = |Cy| - |2 * z * ρ_xy|
//   new_z = |Cz| - |2*x*y*(x²+y²-z²)/(x²+y²)|
//
// Benesi Type 2 Power-2 formula.
// XY-plane orientation (ρ = sqrt(x²+y²)), z as "radial" axis.
// C handling: NON-STANDARD — same |C| - |formula| pattern as Type 1.

template<>
cpu_double3 execute_formula<FORMULA_BENESI2POW2>(cpu_double3 p, constant double* params, double &de) {
    // C passed as params (non-standard C handling)
    const double Cx = params[0];
    const double Cy = params[1];
    const double Cz = params[2];

    double x = p.x, y = p.y, z = p.z;

    double x2 = x * x;
    double y2 = y * y;
    double z2 = z * z;
    double rho_xy2 = x2 + y2;
    double rho_xy = sqrt(rho_xy2);

    // x: standard power-2
    double new_x = x2 - y2 - z2 + Cx;

    // y: |Cy| - |2*z*sqrt(x²+y²)|
    double new_y = fabs(Cy) - fabs(2.0 * z * rho_xy);

    // z: |Cz| - |2*x*y*(x²+y²-z²)/(x²+y²)|
    double z_numer = 2.0 * x * y * (rho_xy2 - z2);
    double z_term;
    if (rho_xy2 == 0.0 && (x != 0.0 || y != 0.0)) {
        // x87 keeps tiny x/y tails alive longer than ARM double. When only the
        // denominator underflows away, the reference stays finite here.
        z_term = 0.0;
    } else {
        z_term = z_numer / rho_xy2;
    }
    double new_z = fabs(Cz) - fabs(z_term);

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Benesi2Pow2 = {
    .id = FORMULA_BENESI2POW2,
    .name = "Benesi2Pow2",
    .param_count = 3,
    .complexity = 89,
    .uses_transcendental = false
};

/*
Original M3 Description:
Benesi Type 2 Power-2 formula by Ben Benesi.
Uses xy-plane for the radial component (ρ = sqrt(x²+y²)).

ρ = sqrt(x²+y²)
new_x = x² - y² - z² + Cx
new_y = |Cy| - |2*z*ρ|
new_z = |Cz| - |2*x*y*(x²+y²-z²)/(x²+y²)|

C handling: NON-STANDARD (same as Benesi1Pow2).
Engine should NOT add C to this formula; template handles it.

Parameters (C passed as params):
  [0] Cx: double
  [1] Cy: double
  [2] Cz: double
*/
