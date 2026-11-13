// MitreIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MitreIFS
// Hand-ported from ASM: 327 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS transform + x87 mitre quartic surface DE)
// Simple finite quartic surface with 3SingleAngles rotation

constexpr unsigned int FORMULA_MITREIFS = 10212;

// ASM trace summary:
//   0x00-0x40: Standard SSE2 IFS preamble (Scale, X/Y/Z_add, Apply_scale_add)
//   0x4B-0x54: Initialize [esp+0x18]=0, [esp+0x20]=0 (pos/neg accumulators)
//   0x55-0xA7: 3x3 rotation matrix multiply (3SingleAngles MitreRot)
//              edi adjusted -0x2c, then restored +0x2c
//   0xAA-0xBE: Compute r_sq = x^2 + y^2 + z^2, store to [esp+0x28]
//   0xC4-0xD0: pos_acc = X2R_mul * r_sq * x * x (quartic term in x)
//   0xD1-0xEB: Compute -(y^2) - (z^2) + YZ_add = YZ_add - y^2 - z^2
//              Multiply by y * y -> val = (YZ_add - y^2 - z^2) * y^2
//   0xF1-0x102: Split pos/neg: if val < 0, subtract from pos_acc; else add to neg_acc
//   0x10C-0x11E: fourth_root(pos_acc) - fourth_root(neg_acc) = raw shape DE
//   0x120-0x133: Spherical correction: r_correction = sqrt(r_sq) - Sph_Radius
//               corrected = r_correction + |r_correction|  (=2*max(0,r_correction))
//               doubled twice: * 4
//   0x133-0x135: shape_de = (raw + 4*max(0,r-R)) * constant
//   0x137-0x146: Scale correction + store

template<>
cpu_double3 execute_formula<FORMULA_MITREIFS>(cpu_double3 p, constant double* params, double &de) {
    const double YZ_add = params[0];            // [PVar - 0x10] default 4.1
    const double Scale = params[1];             // [PVar - 0x20] .2Doubles
    const double Z_add = params[2];             // [PVar - 0x30] .2Doubles
    const double Y_add = params[3];             // [PVar - 0x38]
    const double X_add = params[4];             // [PVar - 0x40]
    const double X2R_mul = params[5];           // [PVar - 0x48]
    const int Apply_scale_add = int(params[6]); // [PVar - 0x4c]
    // 3SingleAngles MitreRot (9 float32 values)
    const double r00 = params[7];
    const double r10 = params[8];
    const double r20 = params[9];
    const double r01 = params[10];
    const double r11 = params[11];
    const double r21 = params[12];
    const double r02 = params[13];
    const double r12 = params[14];
    const double r22 = params[15];

    // Constants from [CONSTANTS]:
    const double K_SCALE_CONST = params[16];  // [PVar + 0x00] = 0.3

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Initialize accumulators
    double pos_acc = 0.0;  // [esp+0x18]
    double neg_acc = 0.0;  // [esp+0x20]

    // Step 3: Apply 3x3 rotation (3SingleAngles MitreRot)
    // ASM at 0x55-0x9C: edi adjusted -0x2c, 3x3 matrix multiply, store back, edi += 0x2c
    double rx = x * r00 + y * r10 + z * r20;
    double ry = x * r01 + y * r11 + z * r21;
    double rz = x * r02 + y * r12 + z * r22;
    x = rx; y = ry; z = rz;

    // Step 4: Compute r_sq = x^2 + y^2 + z^2
    // ASM at 0xAA-0xBE:
    double x_sq = x * x;
    double y_sq = y * y;
    double z_sq = z * z;
    double r_sq = x_sq + y_sq + z_sq;

    // Store r_sq to [esp+0x28]

    // Step 5: Quartic term: X2R_mul * r_sq * x * x = X2R_mul * r_sq * x_sq
    // ASM at 0xC4-0xD0:
    // 0xC4: fmul [edi-0x48] = X2R_mul -> r_sq * X2R_mul
    // 0xC7: fmul [esp] = x... wait: [esp] = x (rotated, stored at 0x9C)
    // 0xCA: fmul [esp] = x again -> r_sq * X2R_mul * x * x
    // 0xCD: fstp [esp+0x18] -> pos_acc = X2R_mul * r_sq * x^2
    pos_acc = X2R_mul * r_sq * x * x;

    // Step 6: Compute val = (YZ_add - y^2 - z^2) * y^2
    // ASM at 0xD1-0xEB:
    // 0xD1: fld [edi-0x10] = YZ_add (first param at [PVar-0x10])
    // Wait: but the constant at [edi-0x10] would be YZ_add. But looking at ASM:
    // 0xD1: DD 47 F0 = fld qword ptr [edi - 0x10]
    // [edi-0x10] = YZ_add = 4.1
    // 0xD5: fld y_sq (from [esp+8]), fmul st(0) -> y^2... no:
    // 0xD5: DD 44 24 08 = fld [esp+8] = y (stored at rotation output)
    // 0xD9: fmul st(0) -> y^2
    // 0xDB: fchs -> -y^2
    // 0xDD: fld [esp+0x10] = z
    // 0xE1: fmul st(0) -> z^2
    // 0xE3: fchs -> -z^2
    // 0xE5: faddp -> -y^2 + (-z^2) = -(y^2 + z^2)
    // 0xE7: faddp -> YZ_add - y^2 - z^2
    // 0xE9: fmul [esp+8] = y
    // 0xED: fmul [esp+8] = y -> (YZ_add - y^2 - z^2) * y^2
    double bracket = YZ_add - y_sq - z_sq;
    double val = bracket * y * y;

    // Step 7: Split into pos/neg accumulators
    // 0xF1: ftst -> compare val with 0
    // 0xF6: shr ah,1; jae 0x104 -> if val >= 0, jump
    // If val < 0: fsubr [esp+0x18] -> pos_acc = pos_acc - val (=pos_acc + |val|)
    //   Hmm wait: DC 6C 24 18 = fsubr [esp+0x18]
    //   fsubr: st0 = [esp+0x18] - st0 = pos_acc - val
    //   since val < 0: pos_acc - val = pos_acc + |val|
    //   Store to pos_acc
    // If val >= 0: fadd [esp+0x20] -> neg_acc += val; store to neg_acc
    if (val < 0.0) {
        pos_acc = pos_acc - val;  // pos_acc + |val| since val is negative
    } else {
        neg_acc = neg_acc + val;
    }

    // Step 8: Fourth root difference
    // 0x10C: fld pos_acc, fsqrt, fsqrt -> fourth_root(pos_acc)
    // 0x114: fld neg_acc, fsqrt, fsqrt -> fourth_root(neg_acc)
    // 0x11C: fchs -> negate
    // 0x11E: faddp -> fourth_root(pos_acc) - fourth_root(neg_acc)
    double raw_de = sqrt(sqrt(pos_acc)) - sqrt(sqrt(neg_acc));

    // Step 9: Spherical correction
    // 0x120: fld [esp+0x28] = r_sq
    // 0x124: fsqrt -> r
    // 0x126: fsub [edi-0x10] = YZ_add... wait. Let me re-check:
    // 0x126: DC 67 F0 = fsub qword ptr [edi - 0x10]
    // [edi-0x10] = YZ_add? But that's the first parameter.
    // Hmm, in this formula YZ_add serves dual purpose as both the quartic
    // bracket constant AND the sphere radius for correction?
    // Actually, looking at the description:
    // "x2Rmul * x*x * (x*x + y*y + z*z) - y*y * (YZadd - y*y + z*z) = 0"
    // The YZ_add parameter appears in the bracket.
    // But the spherical correction uses [edi-0x10] which IS YZ_add.
    // So: r_correction = sqrt(r_sq) - YZ_add
    // This acts as a bounding sphere of radius YZ_add (default 4.1)
    double r_correction = sqrt(r_sq) - YZ_add;

    // 0x129: fld st(0) -> dup r_correction
    // 0x12B: fabs
    // 0x12D: faddp -> r_correction + |r_correction|
    //   = 2 * max(0, r_correction) if r_correction >= 0
    //   = 0 if r_correction < 0
    double clamped = r_correction + fabs(r_correction);  // = 2 * max(0, r_correction)

    // 0x12: fadd st(0) -> double it: 4 * max(0, r_correction)
    clamped = clamped + clamped;  // Now 4 * max(0, r_correction)

    // 0x131: fadd st(0) -> double again: 8 * max(0, r_correction)
    clamped = clamped + clamped;  // Now 8 * max(0, r_correction)

    // 0x133: faddp -> raw_de + 8*max(0, r-YZ_add)
    double corrected = raw_de + clamped;

    // 0x135: fmul [edi] = K_SCALE_CONST (0.3) -> final shape DE
    double shape_de = corrected * K_SCALE_CONST;

    // Step 10: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(state_x, state_y, state_z);
}

cpu_double3 MitreIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_MITREIFS>(p, params, de);
}

constant FormulaInfo formula_info_MitreIFS = {
    .id = FORMULA_MITREIFS,
    .name = "MitreIFS",
    .param_count = 16,
    .complexity = 327,
    .uses_transcendental = false
};

/*
Original M3 Description:
  Simple finite quartic surface. Added rotation and customizers.
  Equation: x2Rmul * x*x * (x*x + y*y + z*z) - y*y * (YZadd - y*y + z*z) = 0
  Luca GN 2012

  NOTE: The description says "(YZadd - y*y + z*z)" but ASM computes
  "(YZ_add - y^2 - z^2)". The sign on z^2 differs!
  Trust the ASM: bracket = YZ_add - y^2 - z^2.

ASM Trace Summary:
  0x00-0x40: SSE2 IFS preamble
  0x4B-0x54: Init pos/neg accumulators to 0
  0x55-0xA7: 3x3 rotation (3SingleAngles MitreRot, edi -0x2c/+0x2c)
  0xAA-0xBE: r_sq = x^2 + y^2 + z^2
  0xC4-0xD0: pos_acc = X2R_mul * r_sq * x^2
  0xD1-0xEB: val = (YZ_add - y^2 - z^2) * y^2
  0xF1-0x10A: Split val into pos/neg accumulators
  0x10C-0x11E: raw_de = fourth_root(pos) - fourth_root(neg)
  0x120-0x133: Spherical correction: + 8*max(0, r - YZ_add)
  0x135: shape_de = corrected * 0.3
  0x137-0x146: Scale correction + store

Parameters:
  [0] YZ_add: double (default: 4.1)
  [1] Scale: double (default: 2) (.2Doubles)
  [2] Z_add: double (default: 0) (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] X2R_mul: double (default: 1.6)
  [6] Apply_scale_add: int (default: 1)
  [7..15] MitreRot: 3SingleAngles (9 floats, default angle=90)

  [CONSTANTS] 0.3
*/
