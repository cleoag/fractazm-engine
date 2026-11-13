// heartKluchIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: heartKluchIFS
// Hand-ported from ASM: 315 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS transform + x87 heart isosurface DE)
// Heart isosurface by Alex Kluchikov (2003), with 3SingleAngles rotation

constexpr unsigned int FORMULA_HEARTKLUCHIFS = 10210;

// ASM trace summary:
//   0x00-0x40: Standard SSE2 IFS preamble (Scale, X/Y/Z_add, Apply_scale_add)
//   0x4B-0x9D: 3x3 rotation matrix multiply (3SingleAngles Rotate)
//   0xA0-0x0103: Heart surface DE computation:
//     p = |y - 1.4|
//     x_sq = x^2, z_sq = z^2, y_sq = y^2
//     r_sq = x_sq + y_sq + z_sq
//     inner = 0.25 * z_sq / (p^4 + 0.01)  -> (p is the |y-shift| value)
//     shape = |y*(-1.1) + (-0.8)*sqrt(sqrt(inner + x_sq))|
//     DE = sqrt(6.25 * x_sq + z_sq + shape^2) - 1
//   0x103-0x118: min(heart_DE, r_dist - [edi+0x30])
//   0x118-0x13A: Scale correction + orbit trap + return

template<>
cpu_double3 execute_formula<FORMULA_HEARTKLUCHIFS>(cpu_double3 p, constant double* params, double &de) {
    const double OTrapOffset = params[0];       // [PVar - 0x10]
    const double Scale = params[1];             // [PVar - 0x20] .2Doubles
    const double Z_add = params[2];             // [PVar - 0x30] .2Doubles
    const double Y_add = params[3];             // [PVar - 0x38]
    const double X_add = params[4];             // [PVar - 0x40]
    const int Apply_scale_add = int(params[5]); // [PVar - 0x44] (was -0x44 before edi adjust)
    // Actually: checking ASM: 0x21: cmp [edi - 0x44], 0 -> Apply_scale_add at edi-0x44
    // But then edi is adjusted: 0x4B: add edi, -0x2c
    // After that the rotation matrix is at relative offsets from adjusted edi
    // Let me re-verify: the Apply_scale_add check is at original edi-0x44 which is
    // the first integer after X_add. But wait, the OPTIONS show:
    // .Integer Apply scale+add = 0 at position after X_add
    // So from original edi: -0x10=OTrapOffset, -0x20=Scale, -0x30=Z_add,
    // -0x38=Y_add, -0x40=X_add, -0x44=Apply_scale_add (int, 4 bytes)
    // Then .Double OTrapOnIts at -0x4c, .3SingleAngles Rotate...
    // Actually .3SingleAngles takes 9 floats (36 bytes) = 9*4 = 0x24 bytes
    // The edi adjustment of -0x2c is to make the rotation matrix accessible at nice offsets
    const double OTrapOnIts = params[6];        // [PVar - 0x4c]
    // params[7..15] = 3SingleAngles Rotate (9 float32 values, 3x3 rotation matrix)
    const double r00 = params[7];
    const double r10 = params[8];
    const double r20 = params[9];
    const double r01 = params[10];
    const double r11 = params[11];
    const double r21 = params[12];
    const double r02 = params[13];
    const double r12 = params[14];
    const double r22 = params[15];

    // Constants from [CONSTANTS] section at positive offsets from PVar:
    // After edi is restored at 0x9D (add edi, 0x2c):
    // [edi+0x00] = 1.4, [edi+0x08] = 0.01, [edi+0x10] = 0.25,
    // [edi+0x18] = -1.1, [edi+0x20] = -0.8, [edi+0x28] = 6.25, [edi+0x30] = 1.445
    const double K_Y_SHIFT = 1.4;    // [PVar + 0x00]
    const double K_EPSILON = 0.01;    // [PVar + 0x08]
    const double K_QUARTER = 0.25;    // [PVar + 0x10]
    const double K_Y_MUL = -1.1;     // [PVar + 0x18]
    const double K_SQRT_MUL = -0.8;  // [PVar + 0x20]
    const double K_X_MUL = 6.25;     // [PVar + 0x28]
    const double K_RADIUS = 1.445;   // [PVar + 0x30]

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Apply 3x3 rotation (3SingleAngles)
    // ASM at 0x4B-0x9C: edi adjusted by -0x2c, then 3x3 matrix multiply
    double rx = x * r00 + y * r10 + z * r20;
    double ry = x * r01 + y * r11 + z * r21;
    double rz = x * r02 + y * r12 + z * r22;

    x = rx; y = ry; z = rz;

    // Step 3: Heart surface DE computation
    // ASM at 0xA0 onward (after edi restored):
    // 0xA0: fld y (from [esp+8])
    // 0xA4: fsub [edi] = y - K_Y_SHIFT (1.4)
    // 0xA6: fabs -> pp = |y - 1.4|
    double pp = fabs(y - K_Y_SHIFT);

    // 0xA8: fld z (from [esp+0x10])
    // 0xAC: fld x (from [esp])
    // 0xAF: fmul st(0) -> x^2
    // 0xB1: fst [esp+0x10] -> store x_sq
    double x_sq = x * x;

    // 0xB5: fxch st(1) -> z on top
    // 0xB7: fmul st(0) -> z^2
    // 0xB9: fst [esp] -> store z_sq
    double z_sq = z * z;

    // 0xBC: fld y ([esp+8])
    // 0xC0: fmul st(0) -> y^2
    // 0xC2: faddp -> y^2 + z^2
    // 0xC4: faddp -> y^2 + z^2 + x_sq = r_sq (but in different order on stack)
    // Wait, let me re-trace more carefully:
    // After 0xB9: st0=z_sq, st1=x_sq, st2=pp
    // 0xBC: fld [esp+8] = y -> st0=y, st1=z_sq, st2=x_sq, st3=pp
    // 0xC0: fmul st(0) -> st0=y^2
    // 0xC2: faddp st(1) -> st0=y^2+z_sq, st1=x_sq, st2=pp
    // 0xC4: faddp st(1) -> st0=y^2+z_sq+x_sq = r_sq, st1=pp
    double y_sq = y * y;
    double r_sq = x_sq + y_sq + z_sq;

    // 0xC6: fsqrt -> r = sqrt(r_sq)
    double r = sqrt(r_sq);

    // 0xC8: fxch st(1) -> st0=pp, st1=r
    // 0xCA: fmul st(0) -> pp^2
    // 0xCC: fmul st(0) -> pp^4
    double pp4 = pp * pp * pp * pp;

    // 0xCE: fadd [edi+0x08] -> pp^4 + 0.01
    double denom = pp4 + K_EPSILON;

    // 0xD1: fld [esp] = z_sq
    // 0xD4: fmul [edi+0x10] = 0.25 -> 0.25 * z_sq
    // 0xD7: fdivrp -> st0 = (0.25 * z_sq) / (pp^4 + 0.01)
    // Note: DE F1 = fdivrp st(1), st(0) -> st(1) = st(0) / st(1)
    // st0 = 0.25*z_sq, st1 = denom
    // fdivrp: st1 = st0/st1 = (0.25*z_sq)/denom, pop -> st0 = result
    double inner_frac = (K_QUARTER * z_sq) / denom;

    // 0xD9: fadd [esp+0x10] = x_sq -> inner_frac + x_sq
    double inner = inner_frac + x_sq;

    // 0xDD: fsqrt -> sqrt(inner)
    // 0xDF: fsqrt -> sqrt(sqrt(inner)) = fourth_root(inner)
    double fourth_root_inner = sqrt(sqrt(inner));

    // 0xE1: fmul [edi+0x20] = -0.8 -> -0.8 * fourth_root(inner)
    double shape_val = K_SQRT_MUL * fourth_root_inner;

    // 0xE4: fld [esp+8] = y
    // 0xE8: fmul [edi+0x18] = -1.1 -> y * (-1.1)
    // 0xEB: faddp -> y*(-1.1) + (-0.8)*fourth_root(inner)
    double combined = y * K_Y_MUL + shape_val;

    // 0xED: fabs -> |combined|
    double abs_combined = fabs(combined);

    // 0xEF: fmul st(0) -> abs_combined^2
    double abs_sq = abs_combined * abs_combined;

    // 0xF1: fadd [esp+0x10] = x_sq -> abs_sq + x_sq
    // Wait: [esp+0x10] was originally z, but we stored x_sq there at 0xB1
    // 0xF5: fld [esp] = z_sq (stored at 0xB9)
    // 0xF8: fmul [edi+0x28] = 6.25 -> 6.25 * z_sq
    // Hmm wait, the description says: DE = sqrt(6.25*x*x + z*z + p*p) - 1
    // But ASM does: [esp+0x10] had x_sq stored, [esp] had z_sq stored
    // Let me re-check the store addresses:
    // 0xB1: fst [esp+0x10] stored x_sq
    // 0xB9: fst [esp] stored z_sq
    // 0xF1: fadd [esp+0x10] = x_sq -> abs_sq + x_sq
    // 0xF5: fld [esp] = z_sq
    // 0xF8: fmul [edi+0x28] = 6.25 -> 6.25 * z_sq
    // Hmm, description says 6.25*x*x but ASM multiplies z_sq by 6.25?
    // Wait: the description has axes potentially rotated. The "x" in description
    // may map to z in our rotated frame, or vice versa.
    // Actually the description says the formula works on the rotated coords.
    // The description uses "x" and "z" generically. Let me trust the ASM.
    // 0xF1: abs_sq + x_sq (stored x_sq)
    // 0xF5: z_sq (stored z_sq)
    // 0xF8: z_sq * 6.25
    // 0xFB: faddp -> z_sq*6.25 + (abs_sq + x_sq)

    // Wait, I need to re-read the stores. After rotation:
    // [esp] = x (rotated), [esp+8] = y (rotated), [esp+0x10] = z (rotated)
    // These were stored at 0x92-0x99 (fstp to esp, esp+8, esp+0x10)
    // Then at 0xAF: fld x from [esp], fmul st(0) -> x^2
    // 0xB1: fst [esp+0x10] -> OVERWRITES z slot with x_sq!
    // 0xB7: z was loaded before at 0xA8, z^2 computed
    // 0xB9: fst [esp] -> OVERWRITES x slot with z_sq!
    // 0xBC: y still at [esp+8]
    //
    // So after stores: [esp]=z_sq, [esp+8]=y, [esp+0x10]=x_sq
    //
    // At 0xF1: fadd [esp+0x10] = x_sq -> abs_sq + x_sq ✓
    // At 0xF5: fld [esp] = z_sq ✓
    // At 0xF8: fmul [edi+0x28] = 6.25 -> 6.25 * z_sq
    // Hmm, but description says 6.25*x*x. Trust ASM:
    // Actually wait. Let me re-read the description more carefully:
    // "DE = (sqrt(6.25 *x*x + z*z + p*p ) - 1)"
    // This means 6.25*x^2 + z^2 + p^2
    // In ASM: 6.25 * z_sq + abs_sq + x_sq
    // That would be 6.25*z^2 + p^2 + x^2
    // These don't match! The description says 6.25 multiplies x^2, ASM says z^2.
    // Trust the ASM. But actually, note the rotation matrix may swap axes.
    // In any case, trust the ASM.

    double final_inner = abs_sq + x_sq + K_X_MUL * z_sq;

    // 0xFB: faddp -> sum
    // 0xFD: fsqrt -> sqrt of sum
    double heart_de = sqrt(final_inner);

    // 0xFF: fld1
    // 0x101: fsubp -> heart_de - 1
    heart_de = heart_de - 1.0;

    // Now: st0 = heart_de, st1 = r (from 0xC6)
    // 0x103: fxch st(1) -> st0=r, st1=heart_de
    // 0x105: fsub [edi+0x30] = K_RADIUS -> r - 1.445
    double r_dist = r - K_RADIUS;

    // 0x108: fcom st(1) -> compare r_dist vs heart_de
    // 0x10A: wait, fnstsw, and ah,0x41
    // 0x110: jne 0x116: if r_dist <= heart_de (equal or less): jump
    // If r_dist > heart_de (ah & 0x41 == 0, je not taken):
    //   0x112: fstp st(1) -> discard heart_de, keep r_dist
    // If r_dist <= heart_de:
    //   0x116: fstp st(0) -> discard r_dist, keep heart_de
    // So result = min(r_dist, heart_de)
    // Wait: and ah, 0x41: tests C3 and C0
    // If st0 > st1 (r_dist > heart_de): C0=0, C3=0 -> ah&0x41=0 -> jne NOT taken
    //   -> fstp st(1) -> keep st0=r_dist -> result = r_dist
    // If st0 < st1: C0=1 -> ah&0x41=1 -> jne taken -> fstp st(0) -> keep heart_de
    // If st0 = st1: C3=1 -> ah&0x41=0x40 -> jne taken -> keep heart_de
    // So: result = (r_dist > heart_de) ? r_dist : heart_de = max(r_dist, heart_de)
    double shape_de = max(r_dist, heart_de);

    // Step 5: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Orbit trap computation (0x124-0x137)
    // [edi-0x10] = OTrapOffset, fild [esi-0x18] = iteration count
    // * [edi-0x4c] = OTrapOnIts, faddp, fabs -> orbit trap color
    params[MAX_FORMULA_PARAMS - 1] = fabs(OTrapOffset + params[ITER_COUNT_SLOT] * OTrapOnIts);

    return cpu_double3(state_x, state_y, state_z);
}

cpu_double3 heartKluchIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HEARTKLUCHIFS>(p, params, de);
}

constant FormulaInfo formula_info_heartKluchIFS = {
    .id = FORMULA_HEARTKLUCHIFS,
    .name = "heartKluchIFS",
    .param_count = 16,
    .complexity = 315,
    .uses_transcendental = false
};

/*
Original M3 Description:
  Isosurface invented and shared by Alex Kluchikov (22 Aug 2003).

  p := abs(y+1.4);
  p = z*z + 0.25*x*x/(p*p*p*p+0.001);
  p = abs(y*1.1-0.8*sqrt(sqrt(p)));
  DE = (sqrt(6.25 *x*x + z*z + p*p ) - 1);

  Note: ASM multiplies z^2 by 6.25 (not x^2 as in description).
  The axes may be swapped relative to the description due to rotation.
  Trust the ASM.

ASM Trace Summary:
  0x00-0x40: SSE2 IFS preamble
  0x4B-0x9C: 3x3 rotation matrix (3SingleAngles Rotate), edi adjusted -0x2c/+0x2c
  0xA0-0xA6: pp = |y - 1.4|
  0xA8-0xC4: x_sq, z_sq, y_sq, r_sq = x^2+y^2+z^2
  0xC6: r = sqrt(r_sq)
  0xC8-0xCE: pp^4 + 0.01
  0xD1-0xD7: (0.25 * z_sq) / (pp^4 + 0.01)
  0xD9-0xDF: fourth_root(inner_frac + x_sq)
  0xE1-0xED: combined = |y*(-1.1) + (-0.8)*fourth_root(...)|
  0xEF-0xFD: heart_de = sqrt(combined^2 + x_sq + 6.25*z_sq) - 1
  0x103-0x116: shape_de = max(r - 1.445, heart_de)
  0x118-0x137: Scale correction + orbit trap

Parameters:
  [0] OTrapOffset: double (default: 0)
  [1] Scale: double (default: 2) (.2Doubles)
  [2] Z_add: double (default: 0) (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Apply_scale_add: int (default: 0)
  [6] OTrapOnIts: double (default: 0.25)
  [7..15] Rotate: 3SingleAngles (9 floats)

  [CONSTANTS] 1.4, 0.01, 0.25, -1.1, -0.8, 6.25, 1.445
*/
