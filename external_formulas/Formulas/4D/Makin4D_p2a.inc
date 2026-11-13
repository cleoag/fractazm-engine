// Makin4D p2a.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin4D p2a
// Hand-ported from ASM: 187 bytes, DEoption=4
// David Makin's 4D "p2a" formula — stereographic squaring with pair dominance.

constexpr unsigned int FORMULA_MAKIN4D_P2A = 9401;

// ASM trace (187 bytes, 1 branch + 2 jmp):
//
// P = 2*(x*z - y*w)         ; cross product 1
// Q = 2*(x*w + y*z)         ; cross product 2
// A = x² + y²               ; xy-pair norm²
// B = z² + w²               ; zw-pair norm²
//
// if B < A:
//   scale = (A-B)/A = 1 - B/A
//   x' = (x²-y²) * scale + Cx
//   y' = 2*x*y * scale + Cy
// else:
//   scale = (A-B)/B = A/B - 1
//   x' = (z²-w²) * scale + Cx
//   y' = 2*z*w * scale + Cy
//
// z' = P + Cz = 2*(x*z - y*w) + Cz
// w' = Q + Cw + W_add = 2*(x*w + y*z) + Cw + W_add
//
// C handling: Formula handles C internally (DEoption=4).
// C values are passed in params[16..19] (Hack for 4D support until engine update)

template<>
cpu_double3 execute_formula<FORMULA_MAKIN4D_P2A>(cpu_double3 p, constant double* params, double &de) {
    const double W_add = params[0]; // [edi-0x10] -> params[0]
    
    // C values passed in last 4 slots of params (engine/harness responsibility)

    double x = p.x, y = p.y, z = p.z;
    double w = de; // w passed via de parameter

    // Cross products
    double P = 2.0 * (x * z - y * w);
    double Q = 2.0 * (x * w + y * z);

    // Pair norms
    double A = x * x + y * y;         // xy-pair
    double B = z * z + w * w;         // zw-pair

    double x_out, y_out;

    if (B < A) {
        // xy-pair dominates: scale xy-squaring by (A-B)/A
        double scale = (A - B) / A;   // safe: A > B >= 0
        x_out = (x * x - y * y) * scale;
        y_out = 2.0 * x * y * scale;
    } else {
        // zw-pair dominates: scale zw-squaring by (A-B)/B.
        // Match x87 semantics directly; do not sanitize zero denominators.
        double scale = (A - B) / B;
        x_out = (z * z - w * w) * scale;
        y_out = 2.0 * z * w * scale;
    }

    double z_out = P;
    double w_out = Q + W_add;

    de = w_out; // return w via de
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Makin4D_p2a = {
    .id = FORMULA_MAKIN4D_P2A,
    .name = "Makin4D p2a",
    .param_count = 1,
    .complexity = 187,
    .uses_transcendental = false
};
