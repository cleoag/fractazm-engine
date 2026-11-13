// MixPinski4ex.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MixPinski4ex
// Hand-ported from ASM: 378 bytes, DEoption=6
// 4D IFS formula: uses de parameter as w coordinate input/output
// 4D Sierpinski-Menger mix with rotation center offsets (extended MixPinski4)

constexpr unsigned int FORMULA_MIXPINSKI4EX = 10349;
template<> struct ExternalFormulaUsesXYZState<FORMULA_MIXPINSKI4EX> { static constexpr bool value = true; };

// ASM trace:
//   Same fold as MixPinski4: Sierpinski conditional swap with negation for all 6 pairs.
//   DIFFERENCE from MixPinski4: rotation center offsets (same pattern as Menger4ex).
//
//   Fold (0x0018-0x0073): Sierpinski-type fold
//     if(x+y<0) { t=x; x=-y; y=-t; }
//     if(x+z<0) { t=x; x=-z; z=-t; }
//     if(y+z<0) { t=y; y=-z; z=-t; }
//     if(x+w<0) { t=x; x=-w; w=-t; }
//     if(y+w<0) { t=y; y=-w; w=-t; }
//     if(z+w<0) { t=z; z=-w; w=-t; }
//
//   After fold, BEFORE rotation (0x0075-0x008):
//     x += Off_B  ([edi-0x80])
//     y += Off_A  ([edi-0x78])
//     z += Off_C  ([edi-0x88])
//     w += Off_D  ([edi-0x90])
//
//   Then: same 4x4 rotation matrix multiply
//
//   After rotation, BEFORE Menger scaling (0x0102-0x012D):
//     w_rot -= Off_D
//     z_rot -= Off_C
//     y_rot -= Off_A
//     x_rot -= Off_B
//
//   Then: same Menger scaling + cross-mapping + z-fold as MixPinski4.
//
//   The post-rotation/scaling code (from 0x0075 onward) is byte-identical to Menger4ex.

template<>
cpu_double3 execute_formula<FORMULA_MIXPINSKI4EX>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    // Original x87 multiplies Deriv1 by the signed scale, without FABS.
    params[FORMULA_STATE_X_SLOT] *= Scale;
    const double CX = params[1];
    const double CY = params[2];
    const double CZ = params[3];
    const double CW = params[4];

    // 4D rotation matrix: 16 pre-computed floats from .6SingleAngles at params[5..20]
    // Rotation center offsets ("ex" parameters) at params[21..24]
    const double Off_A = params[21];  // Rot X-off [edi-0x78]
    const double Off_B = params[22];  // Rot Y-off [edi-0x80]
    const double Off_C = params[23];  // Rot Z-off [edi-0x88]
    const double Off_D = params[24];  // Rot W-off [edi-0x90]

    double x = p.x, y = p.y, z = p.z, w = de;

    // Step 1: Sierpinski-type fold (conditional swap with negation)
    if (x + y < 0.0) { double t = x; x = -y; y = -t; }
    if (x + z < 0.0) { double t = x; x = -z; z = -t; }
    if (y + z < 0.0) { double t = y; y = -z; z = -t; }
    if (x + w < 0.0) { double t = x; x = -w; w = -t; }
    if (y + w < 0.0) { double t = y; y = -w; w = -t; }
    if (z + w < 0.0) { double t = z; z = -w; w = -t; }

    // Step 2: Add rotation center offsets BEFORE rotation
    x += Off_B;
    y += Off_A;
    z += Off_C;
    w += Off_D;

    // Step 3: Apply 4D rotation using pre-computed matrix
    double r0 = x*params[5]  + y*params[6]  + z*params[7]  + w*params[8];
    double r1 = x*params[9]  + y*params[10] + z*params[11] + w*params[12];
    double r2 = x*params[13] + y*params[14] + z*params[15] + w*params[16];
    double r3 = x*params[17] + y*params[18] + z*params[19] + w*params[20];

    // Step 4: Subtract rotation center offsets AFTER rotation
    r0 -= Off_B;
    r1 -= Off_A;
    r2 -= Off_C;
    r3 -= Off_D;

    // Step 5: Menger scaling + cross-mapping + z-fold
    double k = Scale - 1.0;

    // w output
    double w_out = Scale * r3 - k * CW;
    de = w_out;

    // The shared Menger postfix subtracts CX from scaled row1 and CY
    // from scaled row0, then writes row0 to Y and row1 to X.
    double x_out = Scale * r1 - k * CX;
    double y_out = Scale * r0 - k * CY;

    // z output: Menger cross fold
    double kz = (k / Scale) * CZ;
    double z_out = (kz - abs(r2 - kz)) * Scale;

    return cpu_double3(x_out, y_out, z_out);
}

constant FormulaInfo formula_info_MixPinski4ex = {
    .id = FORMULA_MIXPINSKI4EX,
    .name = "MixPinski4ex",
    .param_count = 15,
    .complexity = 378,
    .uses_transcendental = false
};

/*
Original M3 Description:
See Sierpinski4ex and Menger4ex for any explanation.

Combines Sierpinski fold with Menger scaling, plus movable 4D rotation center.

Algorithm:
  1. Sierpinski fold (conditional swap+negate for all 6 pairs)
  2. Add rotation offsets
  3. Apply 4D rotation matrix
  4. Subtract rotation offsets
  5. Menger scale + cross-mapping + z-fold

Parameters:
  [0] Scale: double (default: 2)
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
