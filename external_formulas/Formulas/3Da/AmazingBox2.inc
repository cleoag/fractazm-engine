// Original x87 arithmetic and Julia addition before Double stores on Win64.
// Portable fallback is not certified by native oracle evidence.
constexpr unsigned int FORMULA_AMAZINGBOX2 = 10045;

// ASM trace summary:
//   1. Rotate x,z by FoldX_XY angle -> (x_r1, z_r1)
//   2. Rotate x_r1, y by FoldX_XZ angle -> (x_r2, y_r2)
//   3. TgladFold on x_r2 with Fold (the standard fold pattern)
//   4. Undo rotation 2 on folded result -> x_r2b, y_r2b
//   5. Undo rotation 1 on result -> x_folded, z_folded
//   6. TgladFold on y with Fold
//   7. TgladFold on z with Fold
//   8. Add sphere inversion center offsets (InvXC, InvYC, InvZC)
//   9. Sphere fold: rr = x^2+y^2+z^2, m = clamp(max(Scale*factor/rr, Scale*factor), 0, Scale)
//  10. DE tracking: de *= m
//  11. Scale xyz by m, subtract InvCenter*Scale offsets
//  12. Apply 3x3 rotation matrix
//  13. Add Julia C (Cx, Cy, Cz)

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void AmazingBox2Original_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_AMAZINGBOX2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de,0.,0.,0.};
    AmazingBox2Original_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const E fold(params[3]), px(p.x), py(p.y), pz(p.z);
    auto box = [&](E v) { return abs(v+fold) - (abs(v-fold)+v); };
    // The second declared angle pair is applied first. Only these three
    // scratch results spill to Double (0025/003A/005B), not the folded XYZ.
    const E ax(static_cast<double>(px*E(params[20]) - pz*E(params[19])));
    const E az(static_cast<double>(px*E(params[19]) + pz*E(params[20])));
    const E ay(static_cast<double>(ax*E(params[17]) + py*E(params[18])));
    E x = (box(ax*E(params[18])-py*E(params[17]))*E(params[18])
           - ay*E(params[17]))*E(params[20]) - az*E(params[19]);
    E y = box(py), z = box(pz);
    z += E(params[15]); y += E(params[14]); x += E(params[13]);
    const E rr = ((z*z + y*y) + x*x)*E(params[16]);
    const E scale(params[0]);
    const E m = !(rr >= E(params[2])) ? E(params[1])
        : (E(1) >= rr ? scale/rr : scale);
    de = static_cast<double>(E(de)*m);
    z = z*m - E(params[15])*scale;
    y = y*m - E(params[14])*scale;
    x = x*m - E(params[13])*scale;
    // The final matrix coefficients are Single; products, sums and Julia
    // additions stay extended until the three stores at 0183/0188/018D.
    E matrix[9];
    for (int i=0; i<9; ++i) matrix[i] = E(static_cast<float>(params[4+i]));
    return cpu_double3(
        static_cast<double>((x*matrix[0]+y*matrix[1])+z*matrix[2]+E(params[21])),
        static_cast<double>((x*matrix[3]+y*matrix[4])+z*matrix[5]+E(params[22])),
        static_cast<double>((x*matrix[6]+y*matrix[7])+z*matrix[8]+E(params[23])));
#else
    // Raw parameters from M3F [OPTIONS]:
    //   params[0]  = Scale (double, default: 2)
    //   params[1]  = Min R (raw .Boxscale value, default: 0.5)
    //   params[2]  = Fold (double, default: 1)
    //   params[3..5] = Rotate angles (3 Euler angles in degrees, default: 0)
    //   params[6]  = Inv xC (double, default: 0.5)
    //   params[7]  = Inv yC (double, default: 0)
    //   params[8]  = Inv zC (double, default: 0)
    //   params[9]  = Inv Radius (raw .DRecipro value, default: 1)
    //   params[10] = FoldX XY angle (raw .DoubleAngle, default: 0)
    //   params[11] = FoldX XZ angle (raw .DoubleAngle, default: 0)

    const double Scale = params[0];

    // BOXSCALE pre-computed values
    const double ScaleOverMinR2 = params[1];
    const double minR2 = params[2];

    const double Fold = params[3];

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 RM = rotMatrix3FromFloats(params, 4);

    const double InvXC = params[13];
    const double InvYC = params[14];
    const double InvZC = params[15];

    // DRecipro pre-computed: 1/Radius (already inverted by FillCustomVBufWithVars)
    const double InvRadius2Factor = params[16];

    // sin/cos pairs (pre-computed by FillCustomVBufWithVars from .DoubleAngle)
    const double sinXY = params[17];
    const double cosXY = params[18];
    const double sinXZ = params[19];
    const double cosXZ = params[20];

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Rotate (x, z) by XY angle
    // ASM at 0x13: x*cosXY [esi-0x8c]; z*sinXY [esi-0x84]; fsubp -> x_r1 = x*cosXY - z*sinXY
    // ASM at 0x28: x*sinXY [esi-0x84]; z*cosXY [esi-0x8c]; faddp -> z_r1 = x*sinXY + z*cosXY
    double x_r1 = x * cosXY - z * sinXY;
    double z_r1 = x * sinXY + z * cosXY;

    // Step 2: Rotate (x_r1, y) by XZ angle
    // ASM at 0x43: x_r1*cosXZ [esi-0x7c]; y*sinXZ [esi-0x74]; fsubp -> x_r2 = x_r1*cosXZ - y*sinXZ
    // ASM at 0x51: x_r1*sinXZ [esi-0x74]; y*cosXZ [esi-0x7c]; faddp -> y_r2 = x_r1*sinXZ + y*cosXZ
    double x_r2 = x_r1 * cosXZ - y * sinXZ;
    double y_r2 = x_r1 * sinXZ + y * cosXZ;

    // Step 3: TgladFold on x_r2 with Fold
    // ASM at 0x60: fld x_r2; dup; fsub Fold (=ST2); fabs -> |x_r2-Fold|
    //   fadd st(1)=x_r2 -> |x_r2-Fold|+x_r2
    //   fxch st(1): swap -> x_r2, |x_r2-Fold|+x_r2
    //   fadd st(2)=Fold -> x_r2+Fold; fabs -> |x_r2+Fold|
    //   fsubrp st(1): DE E1 = FSUBRP st(1): st(1) = st(0) - st(1) = |x_r2+Fold| - (|x_r2-Fold|+x_r2)
    //   fold_x = |x_r2+Fold| - |x_r2-Fold| - x_r2
    double fold_x = fabs(x_r2 + Fold) - fabs(x_r2 - Fold) - x_r2;

    // Step 4: Undo XZ rotation on (fold_x, y_r2), then undo XY rotation
    // ASM at 0x70: fold_x * cosXZ [esi-0x7c]; y_r2 * sinXZ [esi-0x74]; fsubp -> fold_x*cosXZ - y_r2*sinXZ
    // ASM at 0x7D: result * cosXY [esi-0x8c]; z_r1 * sinXY [esi-0x84]; fsubp -> fx_final
    double fx_unrot1 = fold_x * cosXZ - y_r2 * sinXZ;
    double fx_final = fx_unrot1 * cosXY - z_r1 * sinXY;

    // Step 5: TgladFold on y with Fold
    // ASM at 0x9A: same TgladFold pattern → fold_y = |y+Fold| - |y-Fold| - y
    double fold_y = fabs(y + Fold) - fabs(y - Fold) - y;

    // Step 6: TgladFold on z with Fold
    // ASM at 0xAC: same TgladFold pattern → fold_z = |z+Fold| - |z-Fold| - z
    double fold_z = fabs(z + Fold) - fabs(z - Fold) - z;

    // Step 7: Add sphere inversion center offsets
    x = fx_final + InvXC;
    y = fold_y + InvYC;
    z = fold_z + InvZC;

    // Step 8: Sphere fold
    // ASM at 0xD6: rr = x^2 + y^2 + z^2... but stack is ST0=fold_z+InvXC, ST1=fold_y+InvYC, ST2=fold_x+InvZC
    // ASM at 0xE6: fmul [esi-0x6c]=InvRadius2Factor → rr *= InvRadius2Factor
    // ASM at 0xEA: fcom [esi-0x20]=MinR² (compare rr vs MinR²)
    double rr = (x * x + y * y + z * z) * InvRadius2Factor;

    double m;
    if (rr < minR2) {
        m = ScaleOverMinR2;           // [esi-0x18]: Scale/MinR²
    } else if (rr >= 1.0) {
        m = Scale;                    // [esi-0x10]: Scale
    } else {
        m = Scale / rr;               // fdivr [esi-0x10]: Scale/rr
    }

    // Step 9: DE tracking
    // ASM at 0x110: de *= m
    de *= m;

    // Step 10: Scale coordinates by m, then subtract InvCenter*Scale
    x = x * m - InvXC * Scale;
    y = y * m - InvYC * Scale;
    z = z * m - InvZC * Scale;

    // Step 11: Apply 3x3 rotation matrix
    {
        cpu_double3 rv = RM * cpu_double3(x, y, z);
        // Step 12: Add Julia C from context [esi+0x18/0x20/0x28]
        return cpu_double3(rv.x+params[21], rv.y+params[22], rv.z+params[23]);
    }
#endif
}

// Formula metadata
constant FormulaInfo formula_info_AmazingBox2 = {
    .id = FORMULA_AMAZINGBOX2,
    .name = "AmazingBox2",
    .param_count = 12,
    .complexity = 412,
    .uses_transcendental = true
};

/*
Original M3 Description:
(from Mandalay-KIFS description - this is a variant of the AmazingBox family)

AmazingBox2 is a 'Platinum-like' Amazing Box fractal with:
1. Dual rotatable fold axes (FoldX XY and XZ angles via DoubleAngle)
2. Sphere inversion with customizable center (Inv xC, yC, zC)
3. Boxscale sphere fold (MinR parameter)
4. DRecipro Inv Radius for the sphere fold factor
5. 3x3 rotation matrix applied after scaling
6. Julia C added internally

Raw Parameters (from M3F [OPTIONS]):
  [0]  Scale: double (default: 2)
  [1]  Min R: double (.Boxscale, default: 0.5) — engine computes MinR^2 and Scale/MinR^2
  [2]  Fold: double (default: 1)
  [3..5] Rotation angles: 3 Euler angles in degrees (.3SingleAngles, default: 0)
  [6]  Inv xC: double (default: 0.5)
  [7]  Inv yC: double (default: 0)
  [8]  Inv zC: double (default: 0)
  [9]  Inv Radius: double (.DRecipro, default: 1) — engine computes 1/value
  [10] FoldX XY angle: double (.DoubleAngle, default: 0) — engine computes sin/cos
  [11] FoldX XZ angle: double (.DoubleAngle, default: 0) — engine computes sin/cos

PVar offset layout (esi = PVar, pre-computed by engine):
  [esi-0x10] = Scale
  [esi-0x18] = Scale/MinR^2  (precomputed by engine for .Boxscale)
  [esi-0x20] = MinR^2        (precomputed by engine for .Boxscale)
  [esi-0x28] = Fold
  [esi-0x2c..-0x4c] = rotation matrix (9 singles from .3SingleAngles)
  [esi-0x54] = Inv zC  (NOTE: InvZC is applied to x-fold, InvXC to z-fold in ASM!)
  [esi-0x5c] = Inv yC
  [esi-0x64] = Inv xC
  [esi-0x6c] = 1/InvRadius (precomputed from .DRecipro)
  [esi-0x74] = sin(XZ angle) (precomputed from .DoubleAngle)
  [esi-0x7c] = cos(XZ angle)
  [esi-0x84] = sin(XY angle) (precomputed from .DoubleAngle)
  [esi-0x8c] = cos(XY angle)
*/
