// HybridIntPow2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Hand-ported from Pascal source (formulas.pas)
// DEoption: 0 (standard log-DE, engine adds C after return)

// HybridItIntPow2 — Power-2 sine bulb formula (formulas.pas line 4141)
// Original Pascal/ASM:
//   r_xy² = x² + y²
//   r_xy  = sqrt(r_xy²)
//   new_z = 2 * z * r_xy * Zmul           // w component = new z
//   a     = r_xy² - z²                     // (xx+yy) - zz via fsubrp
//   a_norm = a / r_xy²                     // a divided by r_xy²
//   new_x = (x² - y²) * a_norm
//   new_y = 2 * x * y * a_norm

constexpr unsigned int FORMULA_HYBRIDINTPOW2 = 90002;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDINTPOW2>(cpu_double3 p, constant double* params, double &de) {
    (void)de;  // DEoption=0 — unused

    const double Zmul = params[0];

    double x = p.x;
    double y = p.y;
    double z = p.z;

    double xx = x * x;
    double yy = y * y;
    double r_xy2 = xx + yy;

    double new_z = 2.0 * z * sqrt(r_xy2) * Zmul;

    if (r_xy2 < 1e-40) {
        return cpu_double3(0.0, 0.0, new_z);
    }

    // a = r_xy² - z²
    double a = r_xy2 - z * z;
    double a_norm = a / r_xy2;

    double new_x = (xx - yy) * a_norm;
    double new_y = 2.0 * x * y * a_norm;

    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridIntPow2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDINTPOW2>(p, params, de);
}

constant FormulaInfo formula_info_HybridIntPow2 = {
    .id = FORMULA_HYBRIDINTPOW2,
    .name = "HybridIntPow2",
    .param_count = 1,
    .complexity = 200,
    .uses_transcendental = true
};

/*
Original Pascal source: formulas.pas, procedure HybridItIntPow2 (line 4141)
Power-2 sine-bulb formula. The name "IntPow2" refers to the integer power 2
using the sine-bulb coordinate construction rather than full spherical coordinates.

Parameters:
  [0] Zmul: double (default: 1.0) — scale factor for Z output

Math:
  r_xy² = x² + y²
  r_xy  = sqrt(r_xy²)
  new_z = 2 * z * r_xy * Zmul
  if r_xy² < 1e-40: new_x = new_y = 0
  else:
    a     = r_xy² - z²
    a_norm = a / r_xy²
    new_x = (x² - y²) * a_norm
    new_y = 2 * x * y * a_norm

Engine adds Cx, Cy, Cz after return (DEoption=0).
*/
