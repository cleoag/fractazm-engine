// ABoxMod1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ABoxMod1
// Hand-ported from ASM: 231 bytes, Capstone-verified
// DEoption: 2 (analytic DE via w tracking)
// Extended API required: VaryScale (iteration state), J1/J2/J3 (C components)

constexpr unsigned int FORMULA_ABOXMOD1 = 8024;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxMod1_x87(double* state);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXMOD1>(cpu_double3 p, double* params, double &de) {
    const double Scale = params[0];
    // params[1] = Scale/MinR^2 (BOXSCALE precomputed)
    const double ScaleOverMinR2 = params[1];
    const double minR2 = params[2];   // MinR^2 (BOXSCALE precomputed, already squared)
    const double Fold = params[3];
    const double Scale_vary = params[4];
    const double FoldXMod = params[5];
    const double FoldYMod = params[6];
    const double FoldZMod = params[7];

    // VaryScale tracking: uses iteration state from ext
    // After AppendCValues + AppendExtState:
    //   params[8..11] = J1..J4
    //   params[12] = ext->VaryScale, params[13] = ext->bFirstIt
    double VaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        VaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    // Pascal applies variation on the first call too, after initialization.
    VaryScale += Scale_vary * (abs(VaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;

#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, de, Scale, ScaleOverMinR2, minR2, Fold,
                      Scale_vary, 0, FoldXMod, FoldYMod, FoldZMod,
                      params[8], params[9], params[10], VaryScale};
    ABoxMod1_x87(state);
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#else
    // Box fold with FoldMod offsets
    double x = Fold - abs(abs(p.x + FoldXMod) - Fold) - abs(FoldXMod);
    double y = Fold - abs(abs(p.y + FoldYMod) - Fold) - abs(FoldYMod);
    double z = Fold - abs(abs(p.z + FoldZMod) - Fold) - abs(FoldZMod);

    // Sphere fold
    double rr = x * x + y * y + z * z;
    double m;
    if (rr < minR2) {
        // Clamped case: uses precomputed Scale/MinR^2
        m = ScaleOverMinR2;
    } else if (rr < 1.0) {
        m = VaryScale / rr;
    } else {
        m = VaryScale;
    }

    // DE tracking (w component)
    de = de * m;

    // C values appended by wrapper: params[8]=J1(Cx), params[9]=J2(Cy), params[10]=J3(Cz)
    // Cross-mapping: x += J2 (Cy), y += J1 (Cx), z += J3 (Cz) — verified from Pascal ASM
    cpu_double3 result;
    result.x = x * m + params[9];   // x += J2 (Cy)
    result.y = y * m + params[8];   // y += J1 (Cx)
    result.z = z * m + params[10];  // z += J3 (Cz)
    return result;
#endif
}

// Alternative direct function implementation
cpu_double3 ABoxMod1_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_ABOXMOD1>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ABoxMod1 = {
    .id = FORMULA_ABOXMOD1,
    .name = "ABoxMod1",
    .param_count = 7,
    .complexity = 690,
    .uses_transcendental = false
};

/*
Original M3 Description:
Inspired from a 2D formula proposed by Kali.

Mathematical Formula:
  VaryScale tracking (per-iteration state):
    if first_iteration: VaryScale = Scale
    VaryScale = VaryScale + Scale_vary * (|VaryScale| - 1)

  Box fold with modifiers:
    x = Fold - |abs(x + FoldXMod) - Fold| - |FoldXMod|
    y = Fold - |abs(y + FoldYMod) - Fold| - |FoldYMod|
    z = Fold - |abs(z + FoldZMod) - Fold| - |FoldZMod|

  Sphere fold:
    rr = x^2 + y^2 + z^2
    if rr < MinR^2:       m = Scale / MinR^2  (uses initial Scale, not VaryScale)
    elif rr < 1:          m = VaryScale / rr
    else:                 m = VaryScale

  Apply + C addition (cross-mapped):
    x = x * m + J2 (Cy)
    y = y * m + J1 (Cx)
    z = z * m + J3 (Cz)
    w = w * m  (DE tracking)

Parameters:
  [0] Scale: double (default: 2)
  [1] Min_R: double (default: 0) via BOXSCALE
  [2] Fold: double (default: 2)
  [3] Scale_vary: double (default: 0)
  [4] FoldXMod: double (default: 0)
  [5] FoldYMod: double (default: 0)
  [6] FoldZMod: double (default: 0)
  [7] Cx (J1): from TIteration3D (default: 0 in evaluator)
  [8] Cy (J2): from TIteration3D (default: 0 in evaluator)
  [9] Cz (J3): from TIteration3D (default: 0 in evaluator)

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = Scale/sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x20] = sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x28] = Fold (double)
  [PVar - 0x30] = Scale_vary (double)
  [PVar - 0x38] = FoldXMod (double)
  [PVar - 0x40] = FoldYMod (double)
  [PVar - 0x48] = FoldZMod (double)

TIteration3D field access (via edi = PIteration3D + 0x80):
  [edi + 0x48] = VaryScale (offset 200)
  [edi + 0x50] = bFirstIt (offset 208)
  [edi - 0x68] = J1 (offset 24)
  [edi - 0x60] = J2 (offset 32)
  [edi - 0x58] = J3 (offset 40)

NOTES:
  - VaryScale requires persistent iteration state (not available in basic evaluator)
  - When Scale_vary=0 (default), VaryScale = Scale always → simplified version is exact
  - C component cross-mapping is intentional: x gets Cy (J2), y gets Cx (J1)
  - For rr < minR2 case, ASM uses constant Scale/minR2 from BOXSCALE, not VaryScale/minR2
*/
