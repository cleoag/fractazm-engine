// Sierpinski4ex.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Sierpinski4ex
// Hand-ported from ASM: 379 bytes, DEoption=6
// 4D IFS formula: uses de parameter as w coordinate input/output
// 4D Sierpinski tetrahedron with rotation center offsets (extended Sierpinski4)

constexpr unsigned int FORMULA_SIERPINSKI4EX = 10350;

// ASM trace:
//   Same fold as Sierpinski4/MixPinski4: Sierpinski conditional swap+negate for all 6 pairs.
//   DIFFERENCE from Sierpinski4: rotation center offsets + simple linear scaling.
//
//   Fold (0x0018-0x0073): Sierpinski-type fold (identical to MixPinski4ex fold)
//     if(x+y<0) { t=x; x=-y; y=-t; }
//     if(x+z<0) { t=x; x=-z; z=-t; }
//     if(y+z<0) { t=y; y=-z; z=-t; }
//     if(x+w<0) { t=x; x=-w; w=-t; }
//     if(y+w<0) { t=y; y=-w; w=-t; }
//     if(z+w<0) { t=z; z=-w; w=-t; }
//
//   After fold, BEFORE rotation (0x0076-0x0090):
//     x += Off_B  ([edi-0x80])
//     y += Off_A  ([edi-0x78])
//     z += Off_C  ([edi-0x88])
//     w += Off_D  ([edi-0x90])
//
//   Then: same 4x4 rotation matrix multiply (0x0091-0x0100)
//   Row 3 (w) stored directly to [ecx+8] at 0x0100.
//
//   After rotation (0x0103-0x0121):
//     Restore PVar (add edi, 0x28)
//     Reload all 4 rotation results and subtract offsets:
//       w_rot -= Off_D  ([edi-0x90])
//       z_rot -= Off_C  ([edi-0x88])
//       y_rot -= Off_A  ([edi-0x78])
//       x_rot -= Off_B  ([edi-0x80])
//
//   Then: simple linear scaling (NOT Menger z-fold, NOT cross-mapped):
//     Stack: [Scale, x_adj, y_adj, z_adj, w_adj]
//     Scale all 4: *= Scale
//     Compute (1-Scale) = -(Scale-1)
//     x_out = x_adj*Scale + CX*(1-Scale) = Scale*x_adj - CX*(Scale-1)
//     y_out = Scale*y_adj - CY*(Scale-1)
//     z_out = Scale*z_adj - CZ*(Scale-1)
//     w_out = Scale*w_adj - CW*(Scale-1)
//
//   NO cross-mapping: x→x, y→y, z→z, w→w (unlike Menger4ex/MixPinski4ex)
//   Stores: [ebx]=x_out, [edx]=y_out, [ecx]=z_out, [ecx+8]=w_out

template<>
cpu_double3 execute_formula<FORMULA_SIERPINSKI4EX>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CX = params[1];
    const double CY = params[2];
    const double CZ = params[3];
    const double CW = params[4];

    // 4D rotation matrix: 6 raw angles (degrees) → buildRotMatrix4d
    // 4D rotation matrix: 16 pre-computed floats at params[5..20]

    // Rotation center offsets ("ex" parameters)
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

    // Step 3: Apply 4D rotation
    double r0 = x*params[5] + y*params[6] + z*params[7] + w*params[8];
    double r1 = x*params[9] + y*params[10] + z*params[11] + w*params[12];
    double r2 = x*params[13] + y*params[14] + z*params[15] + w*params[16];
    double r3 = x*params[17] + y*params[18] + z*params[19] + w*params[20];

    // Step 4: Subtract rotation center offsets AFTER rotation
    r0 -= Off_B;
    r1 -= Off_A;
    r2 -= Off_C;
    r3 -= Off_D;

    // Step 5: Simple linear scale + offset for all 4 coordinates
    // NO cross-mapping, NO Menger z-fold (unlike Menger4ex/MixPinski4ex)
    double k = Scale - 1.0;
    double x_out = Scale * r0 - k * CX;
    double y_out = Scale * r1 - k * CY;
    double z_out = Scale * r2 - k * CZ;
    double w_out = Scale * r3 - k * CW;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

constant FormulaInfo formula_info_Sierpinski4ex = {
    .id = FORMULA_SIERPINSKI4EX,
    .name = "Sierpinski4ex",
    .param_count = 15,
    .complexity = 379,
    .uses_transcendental = false
};

/*
Original M3 Description:
See Sierpinski4. This is a 4D extension of the 3D Sierpinski KIFS, with a
built-in 4D rotation, plus you can move arbitrarily in 4D the rotation center.
Of course this has no effect until you change the rotation angles - if you leave
zero, no rotation and no offset!

Algorithm:
  1. Sierpinski fold (conditional swap+negate for all 6 pairs)
  2. Add rotation offsets to folded values
  3. Apply 4D rotation matrix
  4. Subtract rotation offsets from rotated values
  5. Simple linear scale+offset (no cross-mapping, no Menger z-fold)

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 1)
  [4] CScale_W: double (default: 1)
  [5..10] 6SingleAngles: 6 raw angles in degrees (buildRotMatrix4d builds 4x4 internally)
  [11] Rot X-off: double (default: 0.5)
  [12] Rot Y-off: double (default: 0.5)
  [13] Rot Z-off: double (default: 0.5)
  [14] Rot W-off: double (default: 0.5)
*/
