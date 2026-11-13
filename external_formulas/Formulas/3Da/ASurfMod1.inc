// ASurfMod1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ASurfMod1
// Hand-ported from ASM: 268 bytes, DEoption=2
// Combination of Amazing Surface and ABoxMod1 (Kali's ideas)

constexpr unsigned int FORMULA_ASURFMOD1 = 10173;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ASurfMod1_x87(double* state);
#endif

// ASM trace summary:
//   0x00-0x0B: Setup: edi = PIteration3D, ebx = &x, esi = PVar
//   0x0E: edi += 0x80 (standard TIteration3D offset)
//   0x14-0x33: VaryScale init & update:
//     if bFirstIt <= 0: VaryScale = Scale, inc bFirstIt
//     VaryScale = VaryScale + Scale_vary * (|VaryScale| - 1)
//   0x36-0x50: Box fold X: Fold - |abs(x + FoldXMod) - Fold| - |FoldXMod|
//   0x52-0x69: Box fold Y: Fold - |abs(y + FoldYMod) - Fold| - |FoldYMod|
//   0x6B-0x71: Load z (no box fold on z), rearrange stack
//   0x73-0x83: Compute rr = (x*x + y*y + z*z) * (1/Radius) [sphere fold scaling]
//   0x86-0xAB: Sphere fold 3-way branch:
//     if rr < MinR^2: m = Scale/MinR^2  (clamped)
//     elif rr >= 1.0:  m = VaryScale  (no fold)
//     else:            m = VaryScale / rr  (sphere inversion)
//   0xAC-0xB8: DE *= m; x *= m; y *= m; z *= m (fmulp pops scale)
//   0xBA-0xF1: Apply Rotation1 3x3 matrix to (x, y, z)
//   0xF4-0x101: Add C components: z += J3, y += J1, x += J2
//   0x103: Store results

template<>
cpu_double3 execute_formula<FORMULA_ASURFMOD1>(cpu_double3 p, constant double* params, double &de) {
    const double Scale      = params[0];   // [PVar-0x10] default: 2
    const double ScaleMinR2 = params[1];   // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double MinR2      = params[2];   // BOXSCALE pre-computed: sqr(MinR)
    const double Fold       = params[3];   // [PVar-0x28] default: 2
    const double Scale_vary = params[4];   // [PVar-0x30] default: 0
    const double FoldXMod   = params[5];   // [PVar-0x38] default: 0
    const double FoldYMod   = params[6];   // [PVar-0x40] default: 0
    const double InvRadius  = params[7];   // [PVar-0x48] 1/Radius (from .Double 1/Radius, default: 1)

    // Rotation1 matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 RM = rotMatrix3FromFloats(params, 8);
    const double R_00 = RM.col0.x, R_01 = RM.col1.x, R_02 = RM.col2.x;  // row 0
    const double R_10 = RM.col0.y, R_11 = RM.col1.y, R_12 = RM.col2.y;  // row 1
    const double R_20 = RM.col0.z, R_21 = RM.col1.z, R_22 = RM.col2.z;  // row 2

    // Appended Julia C follows the expanded parameter vector after the
    // 17 mapped option values.
    const double J1 = params[17];
    const double J2 = params[18];
    const double J3 = params[19];

    // VaryScale tracking:
    // VaryScale = VaryScale + Scale_vary * (|VaryScale| - 1)
        double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

#if defined(_MSC_VER) && defined(_M_X64)
    // Keep cancellation in the folds and subsequent radius computation in
    // x87 extended precision, matching the shipped formula's instruction order.
    double state[25] = {p.x, p.y, p.z, de};
    for (int i = 0; i < 20; ++i) state[4 + i] = params[i];
    state[24] = VaryScale;
    ASurfMod1_x87(state);
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#endif

    // Box fold X: Fold - |abs(x + FoldXMod) - Fold| - |FoldXMod|
    double x = Fold - abs(abs(p.x + FoldXMod) - Fold) - abs(FoldXMod);

    // Box fold Y: Fold - |abs(y + FoldYMod) - Fold| - |FoldYMod|
    double y = Fold - abs(abs(p.y + FoldYMod) - Fold) - abs(FoldYMod);

    // Z passes through without box fold
    double z = p.z;

    // Sphere fold: compute rr = (x^2 + y^2 + z^2) * InvRadius
    double rr = (x * x + y * y + z * z) * InvRadius;

    // 3-way sphere fold branch
    double m;
    if (rr < MinR2) {
        // Clamped: m = Scale/MinR^2
        m = ScaleMinR2;
    } else if (rr >= 1.0) {
        // Outside unit sphere: m = VaryScale
        m = VaryScale;
    } else {
        // Sphere inversion: m = VaryScale / rr
        m = VaryScale / rr;
    }

    // DE tracking
    de = de * m;

    // Scale coordinates
    x *= m;
    y *= m;
    z *= m;

    // Apply Rotation1 matrix
    double rx = x * R_00 + y * R_01 + z * R_02;
    double ry = x * R_10 + y * R_11 + z * R_12;
    double rz = x * R_20 + y * R_21 + z * R_22;

    // Add C components (cross-mapped via J fields)
    // 0xF4: [ecx]=z gets J3, 0xF9: [edx]=y gets J1, 0xFE: [ebx]=x gets J2
    rx += J2;
    ry += J1;
    rz += J3;

    return cpu_double3(rx, ry, rz);
}

// Formula metadata
constant FormulaInfo formula_info_ASurfMod1 = {
    .id = FORMULA_ASURFMOD1,
    .name = "ASurfMod1",
    .param_count = 10,
    .complexity = 800,
    .uses_transcendental = false
};

/*
Original M3 Description:
Combination of Kali's ideas; Amazing Surface & AboxMod1.
Luca GN 2015

Mathematical Formula:
  1. VaryScale update: VaryScale += Scale_vary * (|VaryScale| - 1)
  2. Box fold X: Fold - |abs(x + FoldXMod) - Fold| - |FoldXMod|
  3. Box fold Y: Fold - |abs(y + FoldYMod) - Fold| - |FoldYMod|
  4. Z passes through unchanged
  5. Sphere fold: rr = (x^2 + y^2 + z^2) * InvRadius
     - if rr < MinR^2:  m = Scale/MinR^2
     - elif rr >= 1.0:  m = VaryScale
     - else:            m = VaryScale / rr
  6. DE *= m; xyz *= m
  7. Apply rotation matrix to (x, y, z)
  8. Add C components: z += Cz, y += Cx, x += Cy (cross-mapped)

Parameters:
  [0]  Scale: double (default: 2)
  [1]  Scale/MinR^2: double (BOXSCALE precomputed from Min_R)
  [2]  MinR^2: double (BOXSCALE precomputed from Min_R, default: 0)
  [3]  Fold: double (default: 2)
  [4]  Scale_vary: double (default: 0)
  [5]  FoldXMod: double (default: 0)
  [6]  FoldYMod: double (default: 0)
  [7]  1/Radius: double (default: 1)
  [8-10]  Rotation angles: 3 Euler angles in degrees (from .3SingleAngles, default: 0)
  [11] Cx (J1): from TIteration3D (default: 0)
  [12] Cy (J2): from TIteration3D (default: 0)
  [13] Cz (J3): from TIteration3D (default: 0)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = Scale/sqr(MinR) (BOXSCALE)
  [PVar-0x20] = sqr(MinR) (BOXSCALE)
  [PVar-0x28] = Fold
  [PVar-0x30] = Scale_vary
  [PVar-0x38] = FoldXMod
  [PVar-0x40] = FoldYMod
  [PVar-0x48] = 1/Radius
  [PVar-0x4C..-0x6C] = Rotation matrix (9 singles)

NOTE: VaryScale requires persistent iteration state. When Scale_vary=0
(default), VaryScale remains equal to Scale on every iteration.
*/
