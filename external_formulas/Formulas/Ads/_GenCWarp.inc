// _GenCWarp.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _GenCWarp
// Hand-ported from ASM: 2042 bytes, DEoption=-1

constexpr unsigned int FORMULA__GENCWARP = 10022;
template<> struct ExternalFormulaAlwaysWritesJuliaXYZ<FORMULA__GENCWARP> { static constexpr bool value = true; };

// ASM trace:
//   This is a "General C Warp" transform that rotates and warps the Julia C
//   vector using spherical coordinate manipulation with configurable angle
//   selection, integer powers, and multiple recipe modes.
//
//   Algorithm:
//   1. Save original C: orig_Cx, orig_Cy, orig_Cz
//   2. Apply pre-rotation matrix (from .3SingleAngles) to C
//   3. Select theta angle arguments based on Th parameter (0-23)
//      → theta = atan2(argA, argB) * Theta_pow
//      → compute cos_theta, sin_theta
//   4. Select phi angle arguments based on Ph parameter (0-23)
//      → phi = atan2(argA, argB) * Phi_pow
//      → compute cos_phi, sin_phi
//   5. Compute |C| = sqrt(Cx'^2 + Cy'^2 + Cz'^2)
//   6. Reconstruct C from angles using selected Recipe:
//      Recipe 0 (NORMAL):  Cx = cos_th*cos_ph*r, Cy = sin_th*cos_ph*r, Cz = sin_ph*r
//      Recipe 1 (COSINE):  Cx = cos_th*sin_ph*r, Cy = sin_th*sin_ph*r, Cz = cos_ph*r
//      Recipe 2 (REVERSED): Cx = cos_th*cos_ph*r, Cy = sin_th*r, Cz = cos_th*sin_ph*r
//   7. Blend with original C: C = (1-Strength)*orig_C + Strength*warped_C
//   8. Blend with position:   C = Xyz_Strength*pos + (1-Xyz_Strength)*C
//
//   Arg selection modes (0-23) for both Theta and Phi:
//     Each mode pushes two values (a, b) for atan2(a, b):
//     0:  a=Cx, b=Cy         6:  a=Cx, b=sqrt(Cy^2+Cz^2)
//     1:  a=Cx, b=Cz         7:  a=Cy, b=sqrt(Cx^2+Cz^2)
//     2:  a=Cy, b=Cx         8:  a=Cz, b=sqrt(Cx^2+Cy^2)
//     3:  a=Cy, b=Cz         9:  a=sqrt(Cy^2+Cz^2), b=Cx
//     4:  a=Cz, b=Cx         10: a=sqrt(Cx^2+Cz^2), b=Cy
//     5:  a=Cz, b=Cy         11: a=sqrt(Cx^2+Cy^2), b=Cz
//     12: a=Cx, b=max(|Cz|,|Cy|)   15: a=max(|Cz|,|Cy|), b=Cx  (fxch)
//     13: a=Cy, b=max(|Cz|,|Cx|)   16: a=max(|Cz|,|Cx|), b=Cy  (fxch)
//     14: a=Cz, b=max(|Cx|,|Cy|)   17: a=max(|Cx|,|Cy|), b=Cz  (fxch)
//     18: a=Cx, b=min(|Cz|,|Cy|)   21: a=min(|Cz|,|Cy|), b=Cx  (fxch)
//     19: a=Cy, b=min(|Cz|,|Cx|)   22: a=min(|Cz|,|Cx|), b=Cy  (fxch)
//     20: a=Cz, b=min(|Cx|,|Cy|)   23: a=min(|Cx|,|Cy|), b=Cz  (fxch)
//
//   NOTE: fpatan in x87 computes atan2(st1, st0).
//   The ASM pushes first value to st1, second to st0.
//   So push order is: first=a (goes to st1), then second=b (goes to st0).
//   fpatan → atan2(a, b) = atan2(st1, st0)
//
// SPECIAL: C-ONLY transform modifying Cx/Cy/Cz, with optional position blending.
// Has .3SingleAngles which requires 9 pre-computed rotation matrix elements.

// Helper: select angle arguments based on mode (0-23)
// Returns cpu_double2(a, b) where the angle = atan2(a, b)
static cpu_double2 select_angle_args(double Cx, double Cy, double Cz, int mode) {
    // Modes 0-5: direct axis pairs
    // Modes 6-11: axis vs hypot of other two
    // Modes 12-17: axis vs max(abs) of other two
    // Modes 18-23: axis vs min(abs) of other two
    // Modes 9-11, 15-17, 21-23: swapped (fxch in ASM)

    switch (mode) {
        // Direct axis pairs (atan2)
        case 0:  return cpu_double2(Cx, Cy);     // a=Cx, b=Cy
        case 1:  return cpu_double2(Cx, Cz);     // a=Cx, b=Cz
        case 2:  return cpu_double2(Cy, Cx);     // a=Cy, b=Cx
        case 3:  return cpu_double2(Cy, Cz);     // a=Cy, b=Cz
        case 4:  return cpu_double2(Cz, Cx);     // a=Cz, b=Cx
        case 5:  return cpu_double2(Cz, Cy);     // a=Cz, b=Cy

        // Hypot pairs (arcsin/arccos style)
        case 6:  return cpu_double2(Cx, sqrt(Cy*Cy + Cz*Cz));
        case 7:  return cpu_double2(Cy, sqrt(Cx*Cx + Cz*Cz));
        case 8:  return cpu_double2(Cz, sqrt(Cx*Cx + Cy*Cy));
        case 9:  return cpu_double2(sqrt(Cy*Cy + Cz*Cz), Cx);   // swapped
        case 10: return cpu_double2(sqrt(Cx*Cx + Cz*Cz), Cy);   // swapped
        case 11: return cpu_double2(sqrt(Cx*Cx + Cy*Cy), Cz);   // swapped

        // Max(abs) pairs — "squarry" bulbs
        case 12: return cpu_double2(Cx, max(fabs(Cz), fabs(Cy)));
        case 13: return cpu_double2(Cy, max(fabs(Cz), fabs(Cx)));
        case 14: return cpu_double2(Cz, max(fabs(Cx), fabs(Cy)));
        case 15: return cpu_double2(max(fabs(Cz), fabs(Cy)), Cx);   // swapped
        case 16: return cpu_double2(max(fabs(Cz), fabs(Cx)), Cy);   // swapped
        case 17: return cpu_double2(max(fabs(Cx), fabs(Cy)), Cz);   // swapped

        // The shipped x86 implementation is not min(abs): AND AH,0x41
        // clears CF immediately before JB, so JB is never taken. FSTP ST(1)
        // retains the last loaded absolute axis. Mode 23 loads X last.
        case 18: return cpu_double2(Cx, fabs(Cy));
        case 19: return cpu_double2(Cy, fabs(Cx));
        case 20: return cpu_double2(Cz, fabs(Cy));
        case 21: return cpu_double2(fabs(Cy), Cx);
        case 22: return cpu_double2(fabs(Cx), Cy);
        case 23: return cpu_double2(fabs(Cx), Cz);

        default: return cpu_double2(Cx, Cy);  // fallback to mode 0
    }
}

template<>
cpu_double3 execute_formula<FORMULA__GENCWARP>(cpu_double3 p, constant double* params, double &de) {
    // Parameters layout:
    //   params[0..8]  = 3x3 rotation matrix elements (SINGLE precision from .3SingleAngles)
    //                   [PVar-0x0C] through [PVar-0x2C]
    //                   Matrix is applied as:
    //                     Cx_new = params[0]*Cx + params[1]*Cy + params[2]*Cz
    //                     Cy_new = params[3]*Cx + params[4]*Cy + params[5]*Cz
    //                     Cz_new = params[6]*Cx + params[7]*Cy + params[8]*Cz
    //   params[9]     = Theta-AnglePow    [PVar-0x34] (double, default: 8)
    //   params[10]    = Phi-AnglePow      [PVar-0x3C] (double, default: 8)
    //   params[11]    = Th (integer 0-23) [PVar-0x40] (default: 2)
    //   params[12]    = Ph (integer 0-23) [PVar-0x44] (default: 8)
    //   params[13]    = Recipe (int 0-2)  [PVar-0x48] (default: 0)
    //   params[14]    = Strength          [PVar-0x50] (double, 0-1, default: 1)
    //   params[15]    = Xyz-Strength      [PVar-0x58] (double, 0-1, default: 0)
    //
    // Engine-provided C values (for modification):
    //   params[16] is the file constant; params[17..19] are Julia Cx/Cy/Cz
    //
    // NOTE: This formula modifies Cx/Cy/Cz in the iteration context.
    // The Metal engine must handle writing back modified C values.

    // Rotation matrix elements
    const double R00 = params[0];
    const double R01 = params[1];
    const double R02 = params[2];
    const double R10 = params[3];
    const double R11 = params[4];
    const double R12 = params[5];
    const double R20 = params[6];
    const double R21 = params[7];
    const double R22 = params[8];

    const double theta_pow = params[9];
    const double phi_pow = params[10];
    const int th_mode = (int)params[11];
    const int ph_mode = (int)params[12];
    const int recipe = (int)params[13];
    const double strength = params[14];
    const double xyz_strength = params[15];

    // Engine-provided C values
    // Slot 16 is the .m3f constant; appended Julia lanes start at 17.
    double Cx = params[17];
    double Cy = params[18];
    double Cz = params[19];

    // Save originals
    double orig_Cx = Cx;
    double orig_Cy = Cy;
    double orig_Cz = Cz;

    // Step 1: Apply rotation matrix to C
    double new_Cx = R00 * Cx + R01 * Cy + R02 * Cz;
    double new_Cy = R10 * Cx + R11 * Cy + R12 * Cz;
    double new_Cz = R20 * Cx + R21 * Cy + R22 * Cz;
    Cx = new_Cx;
    Cy = new_Cy;
    Cz = new_Cz;

    // Step 2: Select theta arguments and compute angle
    cpu_double2 theta_args = select_angle_args(Cx, Cy, Cz, th_mode);
    double theta = atan2(theta_args.x, theta_args.y);
    theta *= theta_pow;
    double cos_theta = cos(theta);
    double sin_theta = sin(theta);

    // Step 3: Select phi arguments and compute angle
    cpu_double2 phi_args = select_angle_args(Cx, Cy, Cz, ph_mode);
    double phi = atan2(phi_args.x, phi_args.y);
    phi *= phi_pow;
    double cos_phi = cos(phi);
    double sin_phi = sin(phi);

    // Step 4: Compute |C|
    double r = sqrt(Cx * Cx + Cy * Cy + Cz * Cz);

    // Step 5: Reconstruct C from angles using recipe
    double warped_Cx, warped_Cy, warped_Cz;

    if (recipe == 0) {
        // NORMAL: standard spherical mapping
        // Cx = cos_theta * cos_phi * r
        // Cy = sin_theta * cos_phi * r
        // Cz = sin_phi * r
        warped_Cx = cos_theta * cos_phi * r;
        warped_Cy = sin_theta * cos_phi * r;
        warped_Cz = sin_phi * r;
    } else if (recipe == 1) {
        // COSINE: swap sin_phi/cos_phi for z
        // Cx = cos_theta * sin_phi * r
        // Cy = sin_theta * sin_phi * r
        // Cz = cos_phi * r
        warped_Cx = cos_theta * sin_phi * r;
        warped_Cy = sin_theta * sin_phi * r;
        warped_Cz = cos_phi * r;
    } else {
        // REVERSED (recipe 2 or default):
        // Cx = cos_theta * cos_phi * r
        // Cy = sin_theta * r
        // Cz = cos_theta * sin_phi * r
        warped_Cx = cos_theta * cos_phi * r;
        warped_Cy = sin_theta * r;
        warped_Cz = cos_theta * sin_phi * r;
    }

    // Step 6: Blend with original C using Strength
    // C = (1-Strength)*orig_C + Strength*warped_C
    double inv_str = 1.0 - strength;
    Cx = inv_str * orig_Cx + strength * warped_Cx;
    Cy = inv_str * orig_Cy + strength * warped_Cy;
    Cz = inv_str * orig_Cz + strength * warped_Cz;

    // Step 7: Blend C with position using Xyz-Strength
    // C = Xyz_Strength*pos + (1-Xyz_Strength)*C
    double inv_xyz = 1.0 - xyz_strength;
    Cx = xyz_strength * p.x + inv_xyz * Cx;
    Cy = xyz_strength * p.y + inv_xyz * Cy;
    Cz = xyz_strength * p.z + inv_xyz * Cz;

    params[WRITE_J1_SLOT] = Cx;
    params[WRITE_J2_SLOT] = Cy;
    params[WRITE_J3_SLOT] = Cz;
    return p;
}

// Alternative direct function implementation
cpu_double3 _GenCWarp_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__GENCWARP>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info__GenCWarp = {
    .id = FORMULA__GENCWARP,
    .name = "_GenCWarp",
    .param_count = 17,
    .complexity = 2042,
    .uses_transcendental = true
};

/*
Original M3 Description:
Combination of _RotateC and Aexion's "C Iteration" concept, but with
"General" formula to include an immense number of combinations.

This formula warps the Julia C vector using spherical coordinate decomposition
with configurable angle arguments, integer powers, and multiple reconstruction
recipes. A pre-rotation matrix (from .3SingleAngles) is applied first.

Algorithm:
  1. Pre-rotate C using 3x3 rotation matrix from .3SingleAngles
  2. Select theta angle arguments (24 modes), compute theta = atan2(a,b) * pow
  3. Select phi angle arguments (24 modes), compute phi = atan2(a,b) * pow
  4. Compute |C| magnitude
  5. Reconstruct C from (theta, phi, |C|) using Recipe:
     Recipe 0 (NORMAL):   Cx = cos(th)*cos(ph)*r, Cy = sin(th)*cos(ph)*r, Cz = sin(ph)*r
     Recipe 1 (COSINE):   Cx = cos(th)*sin(ph)*r, Cy = sin(th)*sin(ph)*r, Cz = cos(ph)*r
     Recipe 2 (REVERSED): Cx = cos(th)*cos(ph)*r, Cy = sin(th)*r, Cz = cos(th)*sin(ph)*r
  6. Blend with original C using Strength (0=no warp, 1=full warp)
  7. Blend with position using Xyz-Strength (0=pure C warp, 1=C=position)

Angle argument modes (0-23):
  0-5:   Direct axis pairs for atan2
  6-11:  Axis vs hypot(other two) - Mandelbulb-style angles
  12-17: Axis vs max(|a|,|b|) of other two - "squarry" bulbs
  18-23: Axis vs min(|a|,|b|) of other two

SPECIAL: C-ONLY transform with .3SingleAngles rotation matrix.
The Metal engine must:
  1. Pre-compute the rotation matrix from the 3 angle parameters
  2. Pass the 9 matrix elements as params[0..8]
  3. Handle writing back modified Cx/Cy/Cz

Implemented by Luca G.N. 2011

Parameters:
  [0..8]  Rotation matrix elements (9 floats from .3SingleAngles)
  [9]     Theta-AnglePow: double (default: 8)
  [10]    Phi-AnglePow: double (default: 8)
  [11]    Th (args 0-23): integer (default: 2)
  [12]    Ph (args 0-23): integer (default: 8)
  [13]    Recipe (0-2): integer (default: 0)
  [14]    Strength (0-1): double (default: 1)
  [15]    Xyz-Strength (0-1): double (default: 0)
  [16]    File constant
  [17]    Cx: engine-provided Julia C.x
  [18]    Cy: engine-provided Julia C.y
  [19]    Cz: engine-provided Julia C.z
*/
