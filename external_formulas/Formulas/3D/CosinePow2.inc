// CosinePow2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: CosinePow2
// Hand-ported from ASM: 244 bytes, DEscale=1, SIpower=2

constexpr unsigned int FORMULA_COSINEPOW2 = 7026;

// ASM trace: identical to PseudoXDB except NO fabs on z.
// PseudoXDB inserts D9E1 (fabs) after loading z; CosinePow2 does not.
//
// R = sqrt(x²+y²), K = 2z/R
// x' = K*(x²-y²) + Cx = 2z(x²-y²)/R + Cx
// y' = 2K*x*y + Cy = 4zxy/R + Cy
// z' = (z²-x²-y²)*Zmul + Cz

template<>
cpu_double3 execute_formula<FORMULA_COSINEPOW2>(cpu_double3 p, constant double* params, double &de) {
    const double Zmul = params[0];

    double x = p.x, y = p.y, z = p.z;
    double x2 = x * x;
    double y2 = y * y;
    double R = sqrt(x2 + y2);
    double K = 2.0 * z / R;

    double new_x = K * (x2 - y2);
    double new_y = 2.0 * K * x * y;
    double new_z = (z * z - x2 - y2) * Zmul;

    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_CosinePow2 = {
    .id = FORMULA_COSINEPOW2,
    .name = "CosinePow2",
    .param_count = 1,
    .complexity = 90,
    .uses_transcendental = false
};

/*
Original M3 Description:
Cosine power-2 fractal formula.
R = sqrt(x²+y²), K = 2z/R
x' = K(x²-y²) + Cx
y' = 2Kxy + Cy
z' = (z²-x²-y²)*Zmul + Cz

DEscale=1, SIpower=2.
Parameters:
  [0] Z multiplier: double (default: 1.0)
*/
