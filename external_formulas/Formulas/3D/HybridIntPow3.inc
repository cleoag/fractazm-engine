// HybridIntPow3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Ported from Pascal HybridItIntPow3 (formulas.pas:4300)
// DEoption: 0 (standard log-DE, engine adds C after return)
// Hand-ported from ASM

// Tricomplex power-3 — polynomial form (NOT spherical coordinates).
// Separates the 2D complex plane and z independently.
//
// ASM trace summary:
//   sx = x*x, sy = y*y, R = sx + sy
//   A  = 1 - 3*sz / (R + 1e-40)   where sz = z*z
//   x' = x * A * (sx - 3*sy)
//   y' = y * A * (3*sx - sy)
//   z' = (3*R - sz) * z * dZmul
//
// Engine adds Cx, Cy, Cz after return (DEoption=0).

constexpr unsigned int FORMULA_HYBRIDINTPOW3 = 90003;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDINTPOW3>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    const double dZmul = params[0];

    const double sx = p.x * p.x;
    const double sy = p.y * p.y;
    const double sz = p.z * p.z;
    const double R  = sx + sy;

    const double A = 1.0 - 3.0 * sz / (R + 1e-40);

    const double new_x = p.x * A * (sx - 3.0 * sy);
    const double new_y = p.y * A * (3.0 * sx - sy);
    const double new_z = (3.0 * R - sz) * p.z * dZmul;

    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridIntPow3_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDINTPOW3>(p, params, de);
}

constant FormulaInfo formula_info_HybridIntPow3 = {
    .id = FORMULA_HYBRIDINTPOW3,
    .name = "HybridIntPow3",
    .param_count = 1,
    .complexity = 90,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas:4300, fHIntFunctions[3]
Tricomplex power-3 polynomial form.

sx = x*x, sy = y*y, sz = z*z, R = sx + sy
A  = 1 - 3*sz / (R + 1e-40)
x' = x * A * (sx - 3*sy)
y' = y * A * (3*sx - sy)
z' = (3*R - sz) * z * dZmul

params[0] = dZmul (z-multiplier, default 1.0)
Engine adds Cx, Cy, Cz after return (DEoption=0).
*/
