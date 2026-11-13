// Menger4ex.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Menger4ex
// Hand-ported from ASM: 378 bytes, DEoption=6
// 4D IFS formula: uses de parameter as w coordinate input/output
// 4D Menger sponge with rotation center offsets (extended Menger4)

constexpr unsigned int FORMULA_MENGER4EX = 10348;
template<> struct ExternalFormulaUsesXYZState<FORMULA_MENGER4EX> { static constexpr bool value = true; };

// ASM trace:
//   Absolute values followed by the original six-comparison x87 network.
//   Its W comparisons and final X/Y exchange are not a descending sort.
//   DIFFERENCE from Menger4: rotation center offsets.
//
//   After the sort fold, BEFORE rotation:
//     s0 += Off_B  ([edi-0x80], "Rot Y-off" in M3)
//     s1 += Off_A  ([edi-0x78], "Rot X-off" in M3)
//     s2 += Off_C  ([edi-0x88], "Rot Z-off" in M3)
//     s3 += Off_D  ([edi-0x90], "Rot W-off" in M3)
//
//   Then: same 4x4 rotation matrix multiply as Menger4.
//   Row 0 → stored [ebx], Row 1 → stored [edx], Row 2 → stored [ecx], Row 3 → stack
//
//   After rotation, BEFORE Menger scaling:
//     w_rot -= Off_D   (0x0102: fld [edi-0x90], fsubp)
//     z_rot -= Off_C   (0x011D: fsub [edi-0x88])
//     y_rot -= Off_A   (0x0125: fsub [edi-0x78])
//     x_rot -= Off_B   (0x012A: fsub [edi-0x80])
//
//   This is the "rotate around a point" pattern:
//     translate(+offset) → rotate → translate(-offset) → scale
//
//   Then: same Menger scaling + cross-mapping + z-fold as Menger4.
//
//   ASM parameter offsets (doubles):
//     [edi-0x10] = Scale
//     [edi-0x18] = CX
//     [edi-0x20] = CY
//     [edi-0x28] = CZ
//     [edi-0x30] = CW
//     [edi-0x38..edi-0x78] = 16 singles for 4x4 rotation matrix
//     [edi-0x78] = Rot X-off  (Off_A, params[21])
//     [edi-0x80] = Rot Y-off  (Off_B, params[22])
//     [edi-0x88] = Rot Z-off  (Off_C, params[23])
//     [edi-0x90] = Rot W-off  (Off_D, params[24])

template<>
cpu_double3 execute_formula<FORMULA_MENGER4EX>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    // Original x87 multiplies Deriv1 by the signed scale, without FABS.
    params[FORMULA_STATE_X_SLOT] *= Scale;
    const double CX = params[1];
    const double CY = params[2];
    const double CZ = params[3];
    const double CW = params[4];

    // 4D rotation matrix: 16 pre-computed floats from .6SingleAngles
    // params[5..20] = m[row][col] in order
    // Rotation center offsets (params[21..24])
    const double Off_A = params[21];  // Rot X-off [edi-0x78]
    const double Off_B = params[22];  // Rot Y-off [edi-0x80]
    const double Off_C = params[23];  // Rot Z-off [edi-0x88]
    const double Off_D = params[24];  // Rot W-off [edi-0x90]

    double x = p.x, y = p.y, z = p.z, w = de;

    // Step 1: Absolute values
    x = abs(x); y = abs(y); z = abs(z); w = abs(w);

    // Step 2: Literal x87 fold (0x1F..0x70). After the first three
    // comparisons ST(0) holds Y. Compare Y/W before X/W, then Z/W;
    // the last FXCH leaves Y,X,Z,W on the stack used by the rotation.
    if (!(x >= y)) { double t = x; x = y; y = t; }
    if (!(x >= z)) { double t = x; x = z; z = t; }
    if (!(y >= z)) { double t = y; y = z; z = t; }
    if (!(y >= w)) { double t = y; y = w; w = t; }
    if (!(x >= w)) { double t = x; x = w; w = t; }
    if (!(z >= w)) { double t = z; z = w; w = t; }
    { double t = x; x = y; y = t; }

    // Step 3: Add rotation center offsets BEFORE rotation
    // ASM: s0 += Off_B, s1 += Off_A, s2 += Off_C, s3 += Off_D
    x += Off_B;
    y += Off_A;
    z += Off_C;
    w += Off_D;

    // Step 4: Apply 4D rotation using pre-computed matrix
    double r0 = x*params[5]  + y*params[6]  + z*params[7]  + w*params[8];
    double r1 = x*params[9]  + y*params[10] + z*params[11] + w*params[12];
    double r2 = x*params[13] + y*params[14] + z*params[15] + w*params[16];
    double r3 = x*params[17] + y*params[18] + z*params[19] + w*params[20];

    // Step 5: Subtract rotation center offsets AFTER rotation
    // ASM: r0 -= Off_B, r1 -= Off_A, r2 -= Off_C, r3 -= Off_D
    r0 -= Off_B;
    r1 -= Off_A;
    r2 -= Off_C;
    r3 -= Off_D;

    // Step 6: Menger scaling + cross-mapping + z-fold
    double k = Scale - 1.0;

    // w output
    double w_out = Scale * r3 - k * CW;
    de = w_out;

    // x87 0x152 subtracts CX from scaled row1; 0x157 subtracts CY
    // from scaled row0. Stores at 0x168/0x16A write row0 to Y, row1 to X.
    double x_out = Scale * r1 - k * CX;
    double y_out = Scale * r0 - k * CY;

    double kz = (k / Scale) * CZ;
    double z_out = (kz - abs(r2 - kz)) * Scale;

    return cpu_double3(x_out, y_out, z_out);
}

constant FormulaInfo formula_info_Menger4ex = {
    .id = FORMULA_MENGER4EX,
    .name = "Menger4ex",
    .param_count = 15,
    .complexity = 378,
    .uses_transcendental = false
};

/*
Original M3 Description:
See Menger4. This is a 4D extension of the 3D Menger KIFS, with a built-in
4D rotation plus you can move arbitrarily in 4D the rotation center. Of course
this has no effect until you change the rotation angles - if you leave zero,
no rotation and no offset!

Algorithm: same as Menger4, but with rotation center offsets:
  1. abs + original comparison network (including final X/Y exchange)
  2. Add rotation offsets to sorted values
  3. Apply 4D rotation matrix
  4. Subtract rotation offsets from rotated values
  5. Menger scale + cross-mapping + z-fold

Parameters:
  [0] Scale: double (default: 3)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 0.5)
  [4] CScale_W: double (default: 0.5)
  [5..10] 6SingleAngles: 6 raw angles in degrees (buildRotMatrix4d builds 4x4 internally)
  [11] Rot X-off: double (default: 0.5)
  [12] Rot Y-off: double (default: 0.5)
  [13] Rot Z-off: double (default: 0.5)
  [14] Rot W-off: double (default: 0.5)
*/
