// _gnarl2D_2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _gnarl2D_2
// Hand-ported from ASM: 92 bytes, DEoption=-1

constexpr unsigned int FORMULA__GNARL2D_2 = 7064;

// ASM trace (asymmetric gnarl variant):
//   Phase 1 (stored to x):
//     t1 = sin(x + cos(x)*Beta) * Alpha
//     new_x = (y + X_step * sin((y+x)*Gamma - t1)) * Strength
//
//   Phase 2 (stored to y):
//     t2 = sin((y + cos(y)*Beta) * Alpha)
//     new_y = (x + X_step * sin(t2 + (x-y)*Gamma)) * Strength
//
//   z unchanged
//
// NOTE: Phase 1 uses sin-then-multiply for Alpha, Phase 2 uses
// multiply-then-sin. This asymmetry is intentional in the ASM.
// Also, Y_step parameter is declared but unused in the ASM code.

template<>
cpu_double3 execute_formula<FORMULA__GNARL2D_2>(cpu_double3 p, constant double* params, double &de) {
    const double Strength = params[0];  // default: 1.0
    const double X_step   = params[1];  // default: -0.1
    // const double Y_step = params[2]; // default: -0.1 (unused in ASM!)
    const double Alpha    = params[3];  // default: 2.0
    const double Beta     = params[4];  // default: -4.0
    const double Gamma    = params[5];  // default: -0.1

    double x = p.x, y = p.y;

    // Phase 1 → new_x (note: sin first, then * Alpha)
    double t1 = sin(x + cos(x) * Beta) * Alpha;
    double new_x = (y + X_step * sin((y + x) * Gamma - t1)) * Strength;

    // Phase 2 → new_y (note: * Alpha first, then sin — asymmetric!)
    double t2 = sin((y + cos(y) * Beta) * Alpha);
    double new_y = (x + X_step * sin(t2 + (x - y) * Gamma)) * Strength;

    return cpu_double3(new_x, new_y, p.z);
}

// Formula metadata
constant FormulaInfo formula_info__gnarl2D_2 = {
    .id = FORMULA__GNARL2D_2,
    .name = "_gnarl2D_2",
    .param_count = 6,
    .complexity = 92,
    .uses_transcendental = true
};

/*
Original M3 Description:
2D Gnarl deformation variant 2 with cos-based distortion and asymmetry.
Uses cos() in the inner chain (unlike _gnarl2D which uses nested sin).
Phase 1 and Phase 2 have different operation ordering (sin*Alpha vs sin(Alpha*)).

Phase 1: t1 = sin(x + cos(x)*Beta) * Alpha
         x' = (y + X_step * sin((y+x)*Gamma - t1)) * Strength
Phase 2: t2 = sin((y + cos(y)*Beta) * Alpha)
         y' = (x + X_step * sin(t2 + (x-y)*Gamma)) * Strength
z unchanged

Note: x↔y swap (new_x uses y, new_y uses x). Typical gnarl pattern.
Note: Y_step parameter is declared but unused in the ASM.

Parameters:
  [0] Strength: double (default: 1.0)
  [1] X step: double (default: -0.1)
  [2] Y step: double (default: -0.1)  ← UNUSED in ASM
  [3] Alpha: double (default: 2.0)
  [4] Beta: double (default: -4.0)
  [5] Gamma: double (default: -0.1)
*/
