// ATetraVS2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ATetraVS2
// Hand-ported from ASM: 440 bytes, Capstone-verified
// DEoption: 11 (ABox-style analytic DE via w tracking)
// Extended API required: VaryScale (iteration state), J1/J2/J3 (C components)

constexpr unsigned int FORMULA_ATETRAVS2 = 10061;

// Helper: ATetra fold for a pair of coordinates
// Implements the inclined plane folding from the description.
// Fold45 = Fold * (-1/sqrt(2))
// For a pair (a, b):
//   sum = a + b + 2*Fold45
//   if sum > 0: a' = -b - 2*Fold45, b' = -a - 2*Fold45  (swap, negate, shift)
//   elif sum <= 0:
//     sum2 = a + b - 2*Fold45
//     if sum2 > 0: no change  (coordinates already in correct quadrant)
//     else: a' = 2*Fold45 - b, b' = 2*Fold45 - a  (swap, negate, shift back)
static void atetra_fold_pair(double &a, double &b, double Fold45) {
    double twoF45 = 2.0 * Fold45;
    double sum = a + b + twoF45;
    if (sum > 0.0) {
        // Positive quadrant fold
        double tmp = -b - twoF45;
        b = -a - twoF45;
        a = tmp;
    } else {
        // Negative quadrant: check nested condition
        double sum2 = a + b - twoF45;
        if (sum2 <= 0.0) {
            // Negative quadrant fold
            double tmp = twoF45 - b;
            b = twoF45 - a;
            a = tmp;
        }
        // else: no fold needed
    }
}

template<>
cpu_double3 execute_formula<FORMULA_ATETRAVS2>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double ScaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];
    const double Scale_vary = params[4];
    const double Radius_recip = params[5];  // DRecipro: pre-computed 1/Radius

    // VaryScale tracking:
    // Full formula: VaryScale = VaryScale + Scale_vary * (|VaryScale| - 1)
    // When Scale_vary=0, VaryScale = Scale on every iteration.
        double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

    // Fold45 = Fold * (-1/sqrt(2))
    // From CONSTANTS section: -0.70710678118654752440... = -1/sqrt(2)
    const double inv_sqrt2 = -0.70710678118655;
    double Fold45 = Fold * inv_sqrt2;

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Apply tetrahedron folding on three pairs:
    // 1. Fold (y, x) pair — from ASM block 0x0036-0x0093
    //    Loads y then x, compares sums, stores to [edx]=y and [ebx]=x
    atetra_fold_pair(y, x, Fold45);

    // 2. Fold (z, x) pair — from ASM block 0x0095-0x00F2
    //    Loads z then x, compares sums, stores to [ecx]=z and [ebx]=x
    atetra_fold_pair(z, x, Fold45);

    // 3. Fold (z, y) pair — from ASM block 0x00F4-0x0151
    //    Loads z then y, compares sums, stores to [ecx]=z and [edx]=y
    atetra_fold_pair(z, y, Fold45);

    // Sphere fold with Radius scaling
    // ASM at 0x0153-0x018:
    //   rr = (x^2 + y^2 + z^2) * (1/Radius)
    //   if rr < MinR^2:       m = Scale / MinR^2  (from BOXSCALE precompute)
    //   elif rr <= 1.0:       m = VaryScale / rr
    //   else:                 m = VaryScale
    double rr = (x * x + y * y + z * z) * Radius_recip;
    double m;
    if (rr < minR2) {
        m = ScaleOverMinR2;
    } else if (rr <= 1.0) {
        m = VaryScale / rr;
    } else {
        m = VaryScale;
    }

    // DE tracking (w component) — ASM at 0x0192-0x0197
    de = de * m;

    // Apply scaling + C component addition
    // ASM stores in order: st0(top)=x*m -> [ecx]=z, st1=y*m -> [edx]=y, st2=z*m -> [ebx]=x
    // Cross-mapping: z_out = x*m + J3(Cz), y_out = y*m + J2(Cy), x_out = z*m + J1(Cx)

    cpu_double3 result;
    result.x = z * m;   // x_out = z*m + J1 (Cx) — ASM: [edi-0x68]
    result.y = y * m;   // y_out = y*m + J2 (Cy) — ASM: [edi-0x60]
    result.z = x * m;   // z_out = x*m + J3 (Cz) — ASM: [edi-0x58]
    return result;
}

// Alternative direct function implementation
cpu_double3 ATetraVS2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ATETRAVS2>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ATetraVS2 = {
    .id = FORMULA_ATETRAVS2,
    .name = "ATetraVS2",
    .param_count = 5,
    .complexity = 520,
    .uses_transcendental = false
};

/*
Original M3 Description:
Revised and improved edition of Amazing Tetrahedron. More simple and visually
correct results. Based on this folding formula:

  Fold45 = Fold * (-1/sqrt(2))
  For each pair (a, b) from {(y,x), (z,x), (z,y)}:
    sum = a + b + 2*Fold45
    if sum > 0:
       n = a, a = -b - 2*Fold45, b = -n - 2*Fold45  (swap+negate+shift)
    else:
       a -= 2*Fold45, b -= 2*Fold45
       sum2 = a + b
       if sum2 < 0:
          n = a, a = -b, b = -n  (swap+negate)
       a += Fold45, b += Fold45
  (... repeated for x+y, x+z, y+z — like Sierpinski3 but folding)

  Sphere fold with Radius scaling:
    rr = (x^2 + y^2 + z^2) / Radius
    if rr < MinR^2:       m = Scale / MinR^2
    elif rr <= 1:         m = VaryScale / rr
    else:                 m = VaryScale

  Apply + C addition (cross-mapped):
    z_out = x * m (J3)
    y_out = y * m (J2)
    x_out = z * m (J1)
    w = w * m  (DE tracking)

Parameters:
  [0] Scale: double (default: 1.414)
  [1] Min_R: double (default: 0.5) via BOXSCALE
  [2] Fold: double (default: 1)
  [3] Scale_vary: double (default: 0)
  [4] Radius_recip: DRecipro reciprocal of Radius (default: 1/8 = 0.125)
  [5] Cx (J1): from TIteration3D (default: 0 in evaluator)
  [6] Cy (J2): from TIteration3D (default: 0 in evaluator)
  [7] Cz (J3): from TIteration3D (default: 0 in evaluator)

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = Scale (double) = 1.414
  [PVar - 0x18] = Scale/sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x20] = sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x28] = Fold (double) = 1
  [PVar - 0x30] = Scale_vary (double) = 0
  [PVar - 0x38] = 1/Radius (DRecipro reciprocal) = 0.125
  [PVar - 0x40] = Radius (DRecipro value) = 8
  [PVar + 0x00] = -0.70710678... = -1/sqrt(2) (constant)

TIteration3D field access (via edi = PIteration3D + 0x80):
  [edi + 0x48] = VaryScale (offset 200)
  [edi + 0x50] = bFirstIt (offset 208)
  [edi - 0x68] = J1 (offset 24) -> added to x_out (from z*m)
  [edi - 0x60] = J2 (offset 32) -> added to y_out (from y*m)
  [edi - 0x58] = J3 (offset 40) -> added to z_out (from x*m)

ASM Trace Summary:
  0x0000-0x0033: VaryScale initialization and update
  0x0036-0x0093: Tetra fold pair (y, x) using Fold45
  0x0095-0x00F2: Tetra fold pair (z, x) using Fold45
  0x00F4-0x0151: Tetra fold pair (z, y) using Fold45
  0x0153-0x0167: rr = (x^2+y^2+z^2) * (1/Radius)
  0x0169-0x018: Sphere fold (rr < MinR^2 / rr <= 1 / else)
  0x0192-0x019E: DE tracking + coordinate scaling
  0x01A0-0x01AD: C addition with cross-mapping (x<->z swap)

NOTES:
  - VaryScale requires persistent iteration state supplied by the wrapper/runtime
  - When Scale_vary=0 (default), VaryScale remains equal to Scale on every iteration
  - Cross-mapping: x_out gets z component, z_out gets x component (confirmed in ASM)
  - Fold45 = Fold * (-1/sqrt(2)) where the constant -1/sqrt(2) is from [CONSTANTS]
  - DRecipro Radius=8 stores 1/Radius=0.125 at first slot, Radius=8 at second
  - The rr is scaled by 1/Radius before sphere fold comparison
  - Luca GN 2011, revised May 2012 with Radius parameter
*/
