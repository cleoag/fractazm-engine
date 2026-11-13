// Makin4D p4b.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Makin4D p4b
// Hand-ported from ASM: 304 bytes, DEoption=4
// David Makin's 4D "p4b" formula — stereographic squaring applied twice (4th power).

constexpr unsigned int FORMULA_MAKIN4D_P4B = 8679;

// ASM trace (304 bytes, 0 branches):
//
// Two identical squaring passes, each computing:
//   r² = x²+y²+z²+w²
//   P = 2*(xz - yw)
//   Q = 2*(xw + yz)
//   A = 2*(x² - y²)
//   B = 4*x*y
//   r_xy = sqrt(A² + B²) = 2*(x² + y²)
//   ratio = 1 - r²/r_xy = [(x²+y²) - (z²+w²)] / [2*(x²+y²)]
//   x_new = A * ratio
//   y_new = B * ratio
//   z_new = P
//   w_new = Q
//
// After both squarings, C is added:
//   x_out = x + Cx
//   y_out = y + Cy
//   z_out = z + Cz
//   w_out = w + Cw + W_add
//
// For 3D (w=0):
//   First pass: P=2xz, Q=2yz, A=2(x²-y²), B=4xy
//   Second pass uses updated (x,y,z,w) including nonzero w from Q.
//
// C handling: Formula adds C internally after both squarings.

template<>
cpu_double3 execute_formula<FORMULA_MAKIN4D_P4B>(cpu_double3 p, constant double* params, double &de) {
    const double W_add = params[0];  // [edi-0x10] (default: 0.0)
    const double Cx    = params[1];
    const double Cy    = params[2];
    const double Cz    = params[3];
    const double Cw    = params[4];

    double x = p.x, y = p.y, z = p.z;
    double w = de;

    // === First squaring pass ===
    {
        double r2 = x * x + y * y + z * z + w * w;
        double P = 2.0 * (x * z - y * w);
        double Q = 2.0 * (x * w + y * z);
        double A = 2.0 * (x * x - y * y);
        double B = 4.0 * x * y;

        // r_xy = 2*(x²+y²), ratio = 1 - r²/r_xy
        // Match x87 semantics directly; do not sanitize zero denominators.
        double r_xy = sqrt(A * A + B * B);  // = 2*(x²+y²)
        double ratio = 1.0 - r2 / r_xy;

        x = A * ratio;
        y = B * ratio;
        z = P;
        w = Q;
    }

    // === Second squaring pass ===
    {
        double r2 = x * x + y * y + z * z + w * w;
        double P = 2.0 * (x * z - y * w);
        double Q = 2.0 * (x * w + y * z);
        double A = 2.0 * (x * x - y * y);
        double B = 4.0 * x * y;

        double r_xy = sqrt(A * A + B * B);
        double ratio = 1.0 - r2 / r_xy;

        x = A * ratio;
        y = B * ratio;
        z = P;
        w = Q;
    }

    // Add C
    x += Cx;
    y += Cy;
    z += Cz;
    w += Cw + W_add;

    de = w;
    return cpu_double3(x, y, z);
}

// Formula metadata
constant FormulaInfo formula_info_Makin4D_p4b = {
    .id = FORMULA_MAKIN4D_P4B,
    .name = "Makin4D p4b",
    .param_count = 5,
    .complexity = 304,
    .uses_transcendental = false
};

/*
Original M3 Description:
David Makin's 4D formula "p4b" from fractalforums.com.
Stereographic squaring applied twice to get 4th power.

Each squaring computes:
  r² = |q|²
  P = 2(xz-yw), Q = 2(xw+yz)
  A = 2(x²-y²), B = 4xy
  ratio = 1 - r²/[2(x²+y²)]
  x_new = A*ratio, y_new = B*ratio
  z_new = P, w_new = Q

Unlike p2a, this version always uses the xy-pair squaring
(no branch on pair dominance).

Full 4D:
  Two squaring passes, then add C.
  w_out += Cw + W_add

3D projection (w=0):
  First pass: w becomes 2yz (nonzero).
  Second pass uses full 4D computation.

Parameters:
  [0] W_add: double (default: 0.0) - w offset [edi-0x10]
  [1..4] Cx, Cy, Cz, Cw: from engine context
*/
