// trifoliumIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: trifoliumIFS
// Hand-ported from ASM: 367 bytes, DEoption=20
// dIFS shape — SSE2 IFS transform + 3SingleAngles rotation + trifolium surface DE

constexpr unsigned int FORMULA_TRIFOLIUMIFS = 10304;

// ASM trace summary:
// SSE2 IFS preamble (0x00-0x40): x=p.x*Scale+X_add, etc.
// 0x40-0x51: Store to stack, init accumulators neg=0, pos=0
// 0x55-0x9C: Apply 3x3 rotation matrix (.3SingleAngles TFRot)
//   Same rotation pattern as other formulas: 9 matrix entries
//   edi adjusted by -0x2C/+0x2C to access rotation matrix offsets
// 0xAA-0xCA: Compute (x²+y²)² = (x²+y²)*(x²+y²)
//   and |b * z³| = |PetalFix1 * z² * z|
//   Accumulate into pos_accum
// 0xCF-0xE2: Compute a*x*(x² - 3*y²) where a = PetalFix2 (description constant)
//   ASM: x² from fmul st(0), then fld y², fmul [edi]=const_neg3 (-3),
//   faddp (x²-3y²), fmulp x → x*(x²-3y²), fmul PetalFix2
//   If positive → add to pos_accum, else subtract from neg_accum
// 0x100-0x114: shape_de = sqrt(sqrt(pos)) - sqrt(sqrt(neg))
//   i.e. fourth root difference: pos^(1/4) - neg^(1/4)
//   Multiply by const[1] = 0.5
// 0x117-0x12: Compare with |z| - const[2] (5.0), take max for z clipping
// 0x12-0x138: Scale correction
// 0x13B-0x16E: Orbit trap: r * OTrapOnDist + iter * OTrapOnIts + OTrapOffset

// The trifolium curve equation:
//   (x² + y²)² + |b*z³| = a*x*(x² - 3y²)
// where a = Petal_fix_2, b = Petal_fix_1
// The DE is computed as the difference of fourth roots of the two sides.

template<>
cpu_double3 execute_formula<FORMULA_TRIFOLIUMIFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters from .m3
    const double Petal_fix_1 = params[0];          // [PVar-0x10] Double Petal fix 1 = 16
    const double Scale = params[1];                // [PVar-0x20] .2Doubles Scale = 2
    const double Z_add = params[2];                // [PVar-0x30] .2Doubles Z add = 0
    const double Y_add = params[3];                // [PVar-0x38] Double Y add = 0
    const double X_add = params[4];                // [PVar-0x40] Double X add = 3
    const double Petal_fix_2 = params[5];          // [PVar-0x48] Double Petal fix 2 = 1
    const int Apply_scale_add = int(params[6]);   // [PVar-0x4c] Integer Apply scale+add = 1
    // .3SingleAngles TFRot at PVar-0x50..-0x74 (9 single-precision rotation matrix entries)
    const RotMatrix3 Rot = rotMatrix3FromFloats(params, 7);
    const double OTrapOnDist = params[16];         // [PVar-0x78] Double OTrapOnDist = .1
    const double OTrapOnIts = params[17];          // [PVar-0x80] Double OTrapOnIts = .1
    const double OTrapOffset = params[18];         // [PVar-0x88] Double OTrapOffset = -1
    // CONSTANTS:
    // [edi+0x00] = -3.0
    // [edi+0x08] = 0.5
    // [edi+0x10] = 5.0

    const double const_neg3 = -3.0;
    const double const_half = 0.5;
    const double const_clip = 5.0;

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    // Step 2: Apply 3x3 rotation (.3SingleAngles TFRot)
    // With default angles = 0, this is identity
    // The ASM adjusts edi by -0x2C, applies 9-element rotation matrix,
    // then restores edi by +0x2C
    double rx = x * Rot.col0.x + y * Rot.col1.x + z * Rot.col2.x;
    double ry = x * Rot.col0.y + y * Rot.col1.y + z * Rot.col2.y;
    double rz = x * Rot.col0.z + y * Rot.col1.z + z * Rot.col2.z;

    // Step 3: Compute trifolium DE
    // pos_accum and neg_accum start at 0
    double pos_accum = 0.0;
    double neg_accum = 0.0;

    // Part A: (x² + y²)² + |Petal_fix_1 * z³|
    // ASM 0xAA-0xCA:
    //   x² + y² → square that → (x²+y²)²
    //   z² * z → z³, * Petal_fix_1, fabs → |b*z³|
    //   Add together → pos_accum += result
    double r2_xy = rx * rx + ry * ry;
    double lhs = r2_xy * r2_xy;     // (x² + y²)²
    double z3_term = fabs(Petal_fix_1 * rz * rz * rz);  // |b * z³|
    pos_accum = lhs + z3_term;      // [esp+0x20]

    // Part B: a * x * (x² - 3y²) where a = Petal_fix_2
    // ASM 0xCF-0xE2:
    //   fld x, dup, fmul st(0) → x²
    //   fld y², fmul (-3) → -3y²
    //   faddp → x² + (-3y²) = x² - 3y²
    //   fmulp x → x * (x² - 3y²)
    //   fmul Petal_fix_2
    //   if > 0: add to pos_accum; else: subtract from neg_accum (which stays 0 for typical values)
    double rhs = Petal_fix_2 * rx * (rx * rx + const_neg3 * ry * ry);

    // ASM: ftst, if positive → add to [esp+0x20] (pos), else fsubr [esp+0x18] (neg)
    if (rhs > 0.0) {
        pos_accum += rhs;       // rhs goes to positive accumulator
    } else {
        neg_accum -= rhs;       // |rhs| goes to negative accumulator (fsubr pattern)
    }

    // Step 4: Fourth root difference DE
    // ASM 0x100-0x114:
    //   sqrt(sqrt(pos_accum)) - sqrt(sqrt(neg_accum))
    //   * const_half (0.5)
    double de_raw = sqrt(sqrt(pos_accum)) - sqrt(sqrt(neg_accum));
    double shape_de = de_raw * const_half;

    // Step 5: Z clipping — max with (|z| - const_clip)
    // ASM 0x117-0x12:
    //   fld [esp+0x10]=rz, fabs, fsub [edi+0x10]=5.0
    //   fcom shape_de, if z_clip < shape_de → keep shape_de, else shape_de = z_clip
    double z_clip = fabs(rz) - const_clip;
    if (z_clip > shape_de) {
        shape_de = z_clip;
    }

    // Step 6: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Step 7: Orbit trap (stored to [esi+0x80])
    // r * OTrapOnDist + iter * OTrapOnIts + OTrapOffset
    params[63] = fabs(sqrt(rx*rx + ry*ry + rz*rz) * OTrapOnDist + params[ITER_COUNT_SLOT] * OTrapOnIts + OTrapOffset);

    return Apply_scale_add ? cpu_double3(x, y, z) : p;
}

cpu_double3 trifoliumIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TRIFOLIUMIFS>(p, params, de);
}

constant FormulaInfo formula_info_trifoliumIFS = {
    .id = FORMULA_TRIFOLIUMIFS,
    .name = "trifoliumIFS",
    .param_count = 19,
    .complexity = 367,
    .uses_transcendental = true
};

/*
Original M3 Description:
3D solid inspired from the trifolium curve.
Negative petal fix will show a "negative flower".

Equation:
  (x² + y²)² + |b * z³| = a * x * (x² - 3 * y²)

where a = Petal_fix_2, b = Petal_fix_1

The DE is computed as: 0.5 * (pos^(1/4) - neg^(1/4))
where pos and neg accumulate the two sides of the implicit equation.
This is then clipped against |z| - 5.0.

Luca GN 2012

Parameters:
  [0] Petal fix 1: double (default: 16) — coefficient for z³ term
  [1] Scale: double (default: 2) — IFS scale (.2Doubles)
  [2] Z add: double (default: 0)
  [3] Y add: double (default: 0)
  [4] X add: double (default: 3)
  [5] Petal fix 2: double (default: 1) — coefficient for x(x²-3y²) term
  [6] Apply scale+add: int (default: 1)
  [7..15] .3SingleAngles TFRot rotation matrix (default: identity)
  [16] OTrapOnDist: double (default: 0.1)
  [17] OTrapOnIts: double (default: 0.1)
  [18] OTrapOffset: double (default: -1)

CONSTANTS:
  [0] -3.0
  [1] 0.5
  [2] 5.0
*/
