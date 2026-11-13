// HybridIntPow7.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.
// Ported from Pascal HybridItIntPow7 (formulas.pas:4601)
// DEoption: 0 (standard log-DE, engine adds C after return)
// Hand-ported from ASM

// Tricomplex power-7 — polynomial form (NOT spherical coordinates).
//
// Pascal source (verbatim):
//   A  := 1 - 7*(S3*(S3*(S3 - 5*R) + 3*R*R)) / (R*R*R + d1em40);
//   y  := A*y*(S1*(S1*(7*S1 - 35*S2) + 21*S2*S2) - S2*S2*S2) + J2;
//   z  := J3 - dZmul*(z*S3*S3*S3 - 7*z*R*(S3*(3*S3 - 5*R) + R*R));
//   x  := A*x*(S1*(S1*(S1 - 21*S2) + 35*S2*S2) - 7*S2*S2*S2) + J1;
// (S1=x*x, S2=y*y, S3=z*z, J1/J2/J3 = C added by engine)
//
// Note: z formula is "J3 - dZmul*(...)". Engine adds J3/cz, so we return
// the negated dZmul term: z' = dZmul * z * (7*R*(S3*(3*S3 - 5*R) + R*R) - S3*S3*S3)
//
// Engine adds Cx, Cy, Cz after return (DEoption=0).

constexpr unsigned int FORMULA_HYBRIDINTPOW7 = 90007;

template<>
cpu_double3 execute_formula<FORMULA_HYBRIDINTPOW7>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    const double dZmul = params[0];

    const double S1 = p.x * p.x;
    const double S2 = p.y * p.y;
    const double S3 = p.z * p.z;
    const double R  = S1 + S2;
    const double RR = R * R;
    const double RRR = RR * R;

    const double A = 1.0 - 7.0 * S3 * (S3 * (S3 - 5.0 * R) + 3.0 * RR) / (RRR + 1e-40);

    const double new_x = A * p.x * (S1 * (S1 * (S1 - 21.0 * S2) + 35.0 * S2 * S2) - 7.0 * S2 * S2 * S2);
    const double new_y = A * p.y * (S1 * (S1 * (7.0 * S1 - 35.0 * S2) + 21.0 * S2 * S2) - S2 * S2 * S2);
    // Pascal: z = J3 - dZmul*(z*S3^3 - 7*z*R*(S3*(3*S3 - 5*R) + R*R))
    // Engine adds J3, so return: -(dZmul*(z*S3^3 - 7*z*R*(...)))
    //   = dZmul * z * (7*R*(S3*(3*S3 - 5*R) + R*R) - S3*S3*S3)
    const double new_z = dZmul * p.z * (7.0 * R * (S3 * (3.0 * S3 - 5.0 * R) + RR) - S3 * S3 * S3);

    return cpu_double3(new_x, new_y, new_z);
}

cpu_double3 HybridIntPow7_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HYBRIDINTPOW7>(p, params, de);
}

constant FormulaInfo formula_info_HybridIntPow7 = {
    .id = FORMULA_HYBRIDINTPOW7,
    .name = "HybridIntPow7",
    .param_count = 1,
    .complexity = 160,
    .uses_transcendental = false
};

/*
Original Pascal source: formulas.pas:4601, fHIntFunctions[7]
Tricomplex power-7 polynomial form.

S1 = x*x, S2 = y*y, S3 = z*z, R = S1+S2
A  = 1 - 7*S3*(S3*(S3 - 5*R) + 3*R*R) / (R*R*R + 1e-40)
x' = A * x * (S1*(S1*(S1 - 21*S2) + 35*S2*S2) - 7*S2*S2*S2)
y' = A * y * (S1*(S1*(7*S1 - 35*S2) + 21*S2*S2) - S2*S2*S2)
z' = dZmul * z * (7*R*(S3*(3*S3 - 5*R) + R*R) - S3*S3*S3)
   (Pascal writes "J3 - dZmul*(...)", engine adds J3; sign absorbed into formula)

params[0] = dZmul (z-multiplier, default 1.0)
Engine adds Cx, Cy, Cz after return (DEoption=0).
*/
