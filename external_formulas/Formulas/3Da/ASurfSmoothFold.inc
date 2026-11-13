// ASurfSmoothFold.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ASurfSmoothFold
// Hand-ported from ASM (358 bytes, Capstone-verified)
// DEoption: 11 (analytic DE via w tracking)
// Extended API required: VaryScale (iteration state), J1/J2/J3 (C components)
//
// Modification of ABoxSmoothFold: z-fold removed, rotation added.
// Smooth folding on x and y only, then sphere fold, scale, rotate, + C.

constexpr unsigned int FORMULA_ASURFSMOOTHFOLD = 10183;

// Original SHR/JZ returns the input for exponents 0 and 1. Larger
// exponents use logical (unsigned) shifts, including negative int32 inputs.
static double asurf_fast_int_pow(double base, unsigned int exp) {
    if (exp <= 1u) return base;
    double result = 1.0;
    double b = base;
    while (exp > 0u) {
        if (exp & 1u) result *= b;
        exp >>= 1;
        if (exp > 0u) b *= b;
    }
    return result;
}

// ---------------------------------------------------------------
// smooth_box_fold: same subroutine as ABoxSmoothFold (ASM at 0x0031)
//   xp = fix * |v|^p
//   result = sign(v) * (|v| + (2*Fold - |v|) * xp) / (xp + 1)
// ---------------------------------------------------------------
static double asurf_smooth_box_fold(double v, double Fold, int sharpness, double fix) {
    double av = abs(v);
    double xp = fix * asurf_fast_int_pow(av, sharpness);
    double result = (av + (2.0 * Fold - av) * xp) / (xp + 1.0);
    return std::signbit(v) ? -result : result;
}

template<>
cpu_double3 execute_formula<FORMULA_ASURFSMOOTHFOLD>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];            // Scale
    const double ScaleOverMinR2_unused = params[1]; // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];             // Fold
    const double Scale_vary = params[4];       // Scale vary
    const int Sharpness_box = int(params[5]); // original integer, including zero
    const double Fix_box = params[6];          // Fix (BoxFold)
    const int Sharpness_ball = int(params[7]);// integer (forced >= 3)
    const double Fix_ball = params[8];         // Fix (BallFold)

    // Rotation matrix elements (from .3SingleAngles pre-computation)
    // These are single (float32) values stored at negative offsets from edi
    // ASM uses edi = PVar. The rotation matrix is stored as 9 singles.
    // From ASM offsets at 0x0116-0x014B:
    //   Row 0: [edi-0x64], [edi-0x68], [edi-0x6C]  -> R00, R01, R02
    //   Row 1: [edi-0x4C], [edi-0x50], [edi-0x54]  -> R10, R11, R12
    //   Row 2: [edi-0x58], [edi-0x5C], [edi-0x60]  -> R20, R21, R22
    // Wait: let me re-trace the rotation multiplication from ASM:
    //   After scaling, stack: [z'*m, y'*m, x'*m]  (z on top)
    //   0x0114: fld st(0) -> [z'*m, z'*m, y'*m, x'*m]
    //   0x0116: fmul [edi-0x64] (single) -> [z'*m * R_a, z'*m, y'*m, x'*m]
    //     Actually from the ASM: dword ptr [edi-0x64] means single double
    //     Let me map the 9 accesses:
    //   First output component (stored to [ecx] = z_out):
    //     z'*m * [edi-0x64] + y'*m * [edi-0x68] + x'*m * [edi-0x6C]
    //   -> Wait, I need to re-read more carefully.
    //   0x0114: fld st(0)         ; dup z'*m
    //   0x0116: fmul [edi-0x64]   ; z'*m * M[a]
    //   0x0119: fld st(2)         ; y'*m
    //   0x011B: fmul [edi-0x68]   ; y'*m * M[b]
    //   0x011E: faddp st(1)       ; z'*M[a] + y'*M[b]
    //   0x0120: fld st(3)         ; x'*m
    //   0x0122: fmul [edi-0x6C]   ; x'*m * M[c]
    //   0x0125: faddp st(1)       ; sum1 = z'*M[a] + y'*M[b] + x'*M[c]
    //
    //   Second output (stored to [edx] = y_out):
    //   0x0127: fld st(1)         ; z'*m (from original stack)
    //   0x0129: fmul [edi-0x4C]   ; z'*m * M[d]
    //   0x012C: fld st(3)         ; y'*m
    //   0x012E: fmul [edi-0x50]   ; y'*m * M[e]
    //   0x0131: faddp st(1)       ;
    //   0x0133: fld st(4)         ; x'*m
    //   0x0135: fmul [edi-0x54]   ; x'*m * M[f]
    //   0x0138: faddp st(1)       ; sum2 = z'*M[d] + y'*M[e] + x'*M[f]
    //
    //   Third output (stored to [ebx] = x_out):
    //   0x013A: fxch st(2)        ; bring z'*m up
    //   0x013C: fmul [edi-0x58]   ; z'*m * M[g]
    //   0x013: fxch st(3)        ; bring y'*m
    //   0x0141: fmul [edi-0x5C]   ; y'*m * M[h]
    //   0x0144: faddp st(3)       ;
    //   0x0146: fxch st(3)        ; bring x'*m
    //   0x0148: fmul [edi-0x60]   ; x'*m * M[i]
    //   0x014B: faddp st(2)       ; sum3 = z'*M[g] + y'*M[h] + x'*M[i]
    //
    //   Storage: sum1 -> [ecx] (z), sum2 -> [edx] (y), sum3 -> [ebx] (x)
    //   Then + C: [esi-0x48] (J3) added to z, [esi-0x50] (J2) to y, [esi-0x58] (J1) to x

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 RM = rotMatrix3FromFloats(params, 9);
    const double R00 = RM.col0.x, R01 = RM.col1.x, R02 = RM.col2.x;  // row 0
    const double R10 = RM.col0.y, R11 = RM.col1.y, R12 = RM.col2.y;  // row 1
    const double R20 = RM.col0.z, R21 = RM.col1.z, R22 = RM.col2.z;  // row 2

    // [CONSTANTS] embedded as compile-time values:
    const double const_0_99 = 0.99;  // [PVar + 0x00]

    // C values passed by engine from context

    // VaryScale tracking
        double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

    // ---------------------------------------------------------------
    // Smooth box fold on x and y ONLY (z is NOT folded)
    // ASM at 0x0061-0x006D:
    //   call smooth_box_fold(x)  (eax = &x)
    //   mov eax, edx
    //   call smooth_box_fold(y)  (eax = &y)
    //   fld [ecx] (z)           ; z loaded directly, no fold
    //   Stack: [z, y', x']
    // ---------------------------------------------------------------
    double x = asurf_smooth_box_fold(p.x, Fold, Sharpness_box, Fix_box);
    double y = asurf_smooth_box_fold(p.y, Fold, Sharpness_box, Fix_box);
    double z = p.z;  // z is NOT folded (this is the "Surf" difference)

    // ---------------------------------------------------------------
    // Smooth sphere fold (identical to ABoxSmoothFold)
    // ---------------------------------------------------------------
    double r_eff;
    if (minR2 >= const_0_99) {
        r_eff = 1.0;
    } else {
        double rr = x * x + y * y + z * z;
        double m_half = (1.0 + minR2) * 0.5;
        double n_half = (1.0 - minR2) * 0.5;

        double r1_raw = rr - m_half;
        double rs = (r1_raw < 0.0) ? -1.0 : 1.0;
        double r1_abs_norm = abs(r1_raw) / n_half;

        double rp;
        if (Sharpness_ball & 1) {
            rp = asurf_fast_int_pow(sqrt(r1_abs_norm), Sharpness_ball) * Fix_ball;
        } else {
            rp = asurf_fast_int_pow(r1_abs_norm, static_cast<unsigned int>(Sharpness_ball) >> 1) * Fix_ball;
        }

        double ratio = (r1_abs_norm + rp) / (1.0 + rp);
        r_eff = m_half + rs * ratio * n_half;
    }

    double m_final = VaryScale / r_eff;

    // DE tracking
    de = de * m_final;

    // Apply scaling
    double sx = x * m_final;
    double sy = y * m_final;
    double sz = z * m_final;

    // ---------------------------------------------------------------
    // Apply rotation matrix R * [sz, sy, sx]
    // Note: rotation is applied AFTER scaling, BEFORE C addition
    // ASM stores: z_rot to [ecx], y_rot to [edx], x_rot to [ebx]
    //   z_rot = sz*R00 + sy*R01 + sx*R02
    //   y_rot = sz*R10 + sy*R11 + sx*R12
    //   x_rot = sz*R20 + sy*R21 + sx*R22
    // ---------------------------------------------------------------
    double z_rot = sz * R00 + sy * R01 + sx * R02;
    double y_rot = sz * R10 + sy * R11 + sx * R12;
    double x_rot = sz * R20 + sy * R21 + sx * R22;

    // Add C
    cpu_double3 result;
    result.z = z_rot;
    result.y = y_rot;
    result.x = x_rot;
    return result;
}

// Alternative direct function implementation
cpu_double3 ASurfSmoothFold_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ASURFSMOOTHFOLD>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ASurfSmoothFold = {
    .id = FORMULA_ASURFSMOOTHFOLD,
    .name = "ASurfSmoothFold",
    .param_count = 11,
    .complexity = 1050,
    .uses_transcendental = true
};

/*
Original M3 Description:
Modification of ABoxSmoothFold taking off z-fold and adding a rotation.

Mathematical Formula:
  Same smooth folding as ABoxSmoothFold, but:
  - Only x and y are smooth-folded (z passes through unchanged)
  - After scaling, a pre-computed rotation matrix is applied
  - Then C is added

Parameters:
  [0] Scale: double (default: 2)
  [1] ScaleOverMinR2: BOXSCALE precomputed
  [2] minR2: BOXSCALE precomputed
  [3] Fold: double (default: 1)
  [4] Scale_vary: double (default: 0)
  [5] Sharpness_box: integer (default: 6)
  [6] Fix_box: double (default: 1)
  [7] Sharpness_ball: integer (default: 4)
  [8] Fix_ball: double (default: 0.3)
  [9-11] Rotation angles: 3 Euler angles in degrees (from .3SingleAngles)
  [12] Cx: from TIteration3D
  [13] Cy: from TIteration3D
  [14] Cz: from TIteration3D

NOTES:
  - "Surf" variant = z is not folded (same difference as Amazing Surf vs ABox)
  - Rotation is applied after scaling but before C addition
  - Smooth fold parameters are identical to ABoxSmoothFold
*/
