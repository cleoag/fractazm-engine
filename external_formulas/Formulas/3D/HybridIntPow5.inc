// HybridIntPow5.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Ported from Pascal HybridItIntPow5 (formulas.pas:4415)
// DEoption: 0 (standard log-DE, engine adds C after return)
// Hand-ported from ASM

// Tricomplex power-5 — polynomial form (NOT spherical coordinates).
//
// Pascal source (verbatim):
//   A  := 1 + 5*(sz*sz - 2*R*sz) / (R*R + d1em40);
//   y  := A*y*(5*sx*sx - sy*(10*sx - sy)) + J2;
//   z  := dZmul*z*(sz*(sz - 10*R) + 5*R*R) + J3;
//   x  := A*x*(sx*(sx - 10*sy) + 5*sy*sy) + J1;
// (J1/J2/J3 = C added by engine)
//
// Engine adds Cx, Cy, Cz after return (DEoption=0).

constexpr unsigned int FORMULA_HYBRIDINTPOW5 = 90005;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDINTPOW5>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    const double dZmul = params[0];

    const double sx = p.x * p.x;
    const double sy = p.y * p.y;
    const double sz = p.z * p.z;
    const double R  = sx + sy;
    const double RR = R * R;

    const double A = 1.0 + 5.0 * (sz * sz - 2.0 * R * sz) / (RR + 1e-40);

    const double new_x = A * p.x * (sx * (sx - 10.0 * sy) + 5.0 * sy * sy);
    const double new_y = A * p.y * (5.0 * sx * sx - sy * (10.0 * sx - sy));
    const double new_z = dZmul * p.z * (sz * (sz - 10.0 * R) + 5.0 * RR);

    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridIntPow5_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDINTPOW5>(p, params, de);
}

constant FormulaInfo formula_info_HybridIntPow5 = {
    .id = FORMULA_HYBRIDINTPOW5,
    .name = "HybridIntPow5",
    .param_count = 1,
    .complexity = 120,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas:4415, fHIntFunctions[5]
Tricomplex power-5 polynomial form.

sx = x*x, sy = y*y, sz = z*z, R = sx+sy
A  = 1 + 5*(sz*sz - 2*R*sz) / (R*R + 1e-40)
x' = A * x * (sx*(sx - 10*sy) + 5*sy*sy)
y' = A * y * (5*sx*sx - sy*(10*sx - sy))
z' = dZmul * z * (sz*(sz - 10*R) + 5*R*R)

params[0] = dZmul (z-multiplier, default 1.0)
Engine adds Cx, Cy, Cz after return (DEoption=0).
*/
