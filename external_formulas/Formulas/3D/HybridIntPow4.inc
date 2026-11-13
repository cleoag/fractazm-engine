// HybridIntPow4.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Ported from Pascal HybridItIntPow4 (formulas.pas:4348)
// DEoption: 0 (standard log-DE, engine adds C after return)
// Hand-ported from ASM

// Tricomplex power-4 — polynomial form (NOT spherical coordinates).
//
// Pascal source (verbatim):
//   R  := x*x + y*y;
//   A  := 1 + (sz*(sz - 6*R)) / (R*R + d1em40);
//   y  := 4*x*y*A*(sx - sy) + J2;
//   z  := dZmul*4*Sqrt(R)*z*(R - sz) + J3;
//   x  := A*(sx*(sx - 6*sy) + sy*sy) + J1;
// (J1/J2/J3 = C added by engine)
//
// Engine adds Cx, Cy, Cz after return (DEoption=0).

constexpr unsigned int FORMULA_HYBRIDINTPOW4 = 90004;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDINTPOW4>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    const double dZmul = params[0];

    const double sx = p.x * p.x;
    const double sy = p.y * p.y;
    const double sz = p.z * p.z;
    const double R  = sx + sy;
    const double RR = R * R;

    const double A = 1.0 + sz * (sz - 6.0 * R) / (RR + 1e-40);

    const double new_x = A * (sx * (sx - 6.0 * sy) + sy * sy);
    const double new_y = 4.0 * p.x * p.y * A * (sx - sy);
    const double new_z = 4.0 * dZmul * sqrt(R) * p.z * (R - sz);

    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridIntPow4_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDINTPOW4>(p, params, de);
}

constant FormulaInfo formula_info_HybridIntPow4 = {
    .id = FORMULA_HYBRIDINTPOW4,
    .name = "HybridIntPow4",
    .param_count = 1,
    .complexity = 100,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas:4348, fHIntFunctions[4]
Tricomplex power-4 polynomial form.

sx = x*x, sy = y*y, sz = z*z, R = sx+sy
A  = 1 + sz*(sz - 6*R) / (R*R + 1e-40)
x' = A * (sx*(sx - 6*sy) + sy*sy)
y' = 4 * x * y * A * (sx - sy)
z' = 4 * dZmul * sqrt(R) * z * (R - sz)

params[0] = dZmul (z-multiplier, default 1.0)
Engine adds Cx, Cy, Cz after return (DEoption=0).
*/
