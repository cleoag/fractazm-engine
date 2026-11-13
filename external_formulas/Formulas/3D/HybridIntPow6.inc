// HybridIntPow6.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Ported from Pascal HybridItIntPow6 (formulas.pas:4500)
// DEoption: 0 (standard log-DE, engine adds C after return)
// Hand-ported from ASM

// Tricomplex power-6 — polynomial form (NOT spherical coordinates).
//
// Pascal source (verbatim):
//   A  := 1 - (sz*(sz*(sz - 15*R) + 15*R*R)) / (R*R*R + d1em40);
//   y  := 2*A*x*y*(S1*(3*S1 - 10*S2) + 3*S2*S2) + J2;
//   z  := dZmul*2*z*Sqrt(R)*(sz*(3*sz - 10*R) + 3*R*R) + J3;
//   x  := A*(S1*S1*(S1 - 15*S2) + S2*S2*(15*S1 - S2)) + J1;
// (S1=x*x, S2=y*y, J1/J2/J3 = C added by engine)
//
// Engine adds Cx, Cy, Cz after return (DEoption=0).

constexpr unsigned int FORMULA_HYBRIDINTPOW6 = 90006;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDINTPOW6>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    const double dZmul = params[0];

    const double S1 = p.x * p.x;
    const double S2 = p.y * p.y;
    const double sz = p.z * p.z;
    const double R  = S1 + S2;
    const double RR = R * R;
    const double RRR = RR * R;

    const double A = 1.0 - sz * (sz * (sz - 15.0 * R) + 15.0 * RR) / (RRR + 1e-40);

    const double new_x = A * (S1 * S1 * (S1 - 15.0 * S2) + S2 * S2 * (15.0 * S1 - S2));
    const double new_y = 2.0 * A * p.x * p.y * (S1 * (3.0 * S1 - 10.0 * S2) + 3.0 * S2 * S2);
    const double new_z = 2.0 * dZmul * p.z * sqrt(R) * (sz * (3.0 * sz - 10.0 * R) + 3.0 * RR);

    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridIntPow6_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDINTPOW6>(p, params, de);
}

constant FormulaInfo formula_info_HybridIntPow6 = {
    .id = FORMULA_HYBRIDINTPOW6,
    .name = "HybridIntPow6",
    .param_count = 1,
    .complexity = 140,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas:4500, fHIntFunctions[6]
Tricomplex power-6 polynomial form.

S1 = x*x, S2 = y*y, sz = z*z, R = S1+S2
A  = 1 - sz*(sz*(sz - 15*R) + 15*R*R) / (R*R*R + 1e-40)
x' = A * (S1*S1*(S1 - 15*S2) + S2*S2*(15*S1 - S2))
y' = 2 * A * x * y * (S1*(3*S1 - 10*S2) + 3*S2*S2)
z' = 2 * dZmul * z * sqrt(R) * (sz*(3*sz - 10*R) + 3*R*R)

params[0] = dZmul (z-multiplier, default 1.0)
Engine adds Cx, Cy, Cz after return (DEoption=0).
*/
