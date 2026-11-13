// Original x87 instruction/store order on Win64; portable fallback is not certified.
constexpr unsigned int FORMULA_ABOXPLATINUM = 10047;

// ASM trace summary (esi = PVar - 0x40, so offsets are shifted):
//   1. Rotate (x, z) with the first FoldX sin/cos pair
//   2. Rotate that result with y using the second FoldX sin/cos pair
//   3. TgladFold on the rotated x lane
//   4. Undo the second FoldX rotation
//   5. Undo the first FoldX rotation -> get folded x / restored y,z
//   6. Rotate the restored (y, z) with the second FoldY sin/cos pair
//   7. Rotate that result with folded x using the first FoldY sin/cos pair
//   8. TgladFold on the rotated y lane
//   9. Undo the first FoldY rotation
//  10. Undo the second FoldY rotation -> get folded y / restored z
//  11. TgladFold on the restored z lane
//  12. Conditional abs() on x, y, z based on Abs XYZ switches (bit flags)
//  13. Add InvCenter offsets (InvXC, InvYC, InvZC)
//  14. Sphere fold: rr = (x^2+y^2+z^2)*factor
//  15. DE tracking: de *= m
//  16. Scale and subtract InvCenter*Scale
//  17. Apply 3x3 rotation matrix
//  18. Wrapper-side +C

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxPlatinumOriginal_x87(double*, const double*, int absSwitch);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXPLATINUM>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de,0.};
    ABoxPlatinumOriginal_x87(state,params,int(params[25]));
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Parameters:
    const double Scale = params[0];
    const double ScaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 4);

    const double InvXC = params[13];
    const double InvYC = params[14];
    const double InvZC = params[15];
    const double InvRadFactor = params[16];

    // Read the fold angle pairs in actual PVar byte-order.
    const double sinFoldX_a = params[19];
    const double cosFoldX_a = params[20];
    const double sinFoldX_b = params[17];
    const double cosFoldX_b = params[18];
    const double sinFoldY_a = params[21];
    const double cosFoldY_a = params[22];
    const double sinFoldY_b = params[23];
    const double cosFoldY_b = params[24];

    const int absSwitch = (int)params[25]; // Integer: bit 0=abs_x, bit 1=abs_y, bit 2=abs_z

    // Julia C

    double x = p.x;
    double y = p.y;
    double z = p.z;

    auto tglad_fold = [Fold](double v) {
        return fabs(v + Fold) - fabs(v - Fold) - v;
    };

    // FoldX stage
    double x1 = x * cosFoldX_a - z * sinFoldX_a;
    double z1 = x * sinFoldX_a + z * cosFoldX_a;
    double x2 = x1 * cosFoldX_b - y * sinFoldX_b;
    double y2 = x1 * sinFoldX_b + y * cosFoldX_b;
    double fold_x_rot = tglad_fold(x2);

    // Undo FoldX rotations
    double x_after_b = fold_x_rot * cosFoldX_b - y2 * sinFoldX_b;
    double y_after_x = fold_x_rot * sinFoldX_b + y2 * cosFoldX_b;
    double x_after_x = x_after_b * cosFoldX_a - z1 * sinFoldX_a;
    double z_after_x = x_after_b * sinFoldX_a + z1 * cosFoldX_a;

    // FoldY stage: ASM rotates restored (y, z) first, then mixes with folded x.
    double y1 = y_after_x * cosFoldY_b - z_after_x * sinFoldY_b;
    double z1y = y_after_x * sinFoldY_b + z_after_x * cosFoldY_b;
    double y2r = y1 * cosFoldY_a - x_after_x * sinFoldY_a;
    double x2r = x_after_x * cosFoldY_a + y1 * sinFoldY_a;
    double fold_y_rot = tglad_fold(y2r);

    // Undo FoldY rotations
    double y_after_a = fold_y_rot * cosFoldY_a - x2r * sinFoldY_a;
    double fx_final = x2r * cosFoldY_a + fold_y_rot * sinFoldY_a;
    double fy_final = y_after_a * cosFoldY_b - z1y * sinFoldY_b;
    double fz_pre = y_after_a * sinFoldY_b + z1y * cosFoldY_b;

    // Step 11: TgladFold on restored z lane
    double fz_final = tglad_fold(fz_pre);

    // Step 12: Conditional abs based on switch bits
    if (absSwitch & 4) fz_final = fabs(fz_final);
    if (absSwitch & 2) fy_final = fabs(fy_final);
    if (absSwitch & 1) fx_final = fabs(fx_final);

    // Step 13: Add InvCenter offsets
    fx_final += InvXC;
    fy_final += InvYC;
    fz_final += InvZC;

    // Step 14: Sphere fold
    double rr = (fx_final * fx_final + fy_final * fy_final + fz_final * fz_final) * InvRadFactor;

    double m;
    if (rr < minR2) {
        m = ScaleOverMinR2;
    } else if (rr >= 1.0) {
        m = Scale;
    } else {
        m = Scale / rr;
    }

    // Step 15: DE tracking
    de *= m;

    // Step 16: Scale and subtract InvCenter*Scale
    fx_final = fx_final * m - InvXC * Scale;
    fy_final = fy_final * m - InvYC * Scale;
    fz_final = fz_final * m - InvZC * Scale;

    // Native helper adds C before narrowing; portable fallback adds C here.
    return R * cpu_double3(fx_final, fy_final, fz_final) + cpu_double3(params[26],params[27],params[28]);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_ABoxPlatinum = {
    .id = FORMULA_ABOXPLATINUM,
    .name = "ABoxPlatinum",
    .param_count = 19,
    .complexity = 512,
    .uses_transcendental = false
};

/*
Original M3 Description:
'Platinum' edition is inspired by Buddhi's version. Lots of added features, but slower.
The result will always be infinitely zoomable without distortion (conformal fractal).

1. You can rotate it as liked. The rotation is applied just before the C addition (Buddhi's method).
2. The sphere inversion can be fully customized; center, radius and MinR.
3. X and Y folds are rotatable.
4. Additionally, you can switch on Abs() functions for x,y,z to get abs-boxes.

Luca GN 2012
Jesse 27.3.2013 speedup

Parameters:
  [0] Scale: double (default: 2)
  [1] Scale/MinR^2: precomputed by BOXSCALE
  [2] MinR^2: precomputed by BOXSCALE (from MinR/IR = 0.5)
  [3] Fold: double (default: 1)
  [4..6] Rotation: 3 Euler angles (degrees) -> buildRotMatrix3
  [7] Inv xC: double (default: 0)
  [8] Inv yC: double (default: 0)
  [9] Inv zC: double (default: 0)
  [10] Inv Radius factor: from DRecipro
  [11..12] first FoldX sin/cos pair
  [13..14] second FoldX sin/cos pair
  [15..16] first FoldY sin/cos pair
  [17..18] second FoldY sin/cos pair
  [19] Abs XYZ switches: integer (bit flags: 1=x, 2=y, 4=z)
  [20..22] Cx, Cy, Cz: Julia constants added by wrapper
*/
