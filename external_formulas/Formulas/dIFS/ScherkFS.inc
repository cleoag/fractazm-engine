// ScherkFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ScherkFS
// Hand-ported from ASM: 276 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS + 3x3 rotation + Scherk minimal surface DE)
// Scherk's minimal surface: DE = z_rot - strength * logbase(|cos(y_rot) / cos(x_rot)|)

constexpr unsigned int FORMULA_SCHERKFS = 10200;

template<>
cpu_double3 execute_formula<FORMULA_SCHERKFS>(cpu_double3 p, constant double* params, double &de) {
    // Raw param layout: [0] LogBase, [1] Scale (.2Doubles raw=1), [2] Z_add (.2Doubles raw=1),
    // [3] Y_add, [4] X_add, [5] Saddle_str, [6] Apply_scale_add,
    // [7..9] 3 angles (.3SingleAngles raw=3) → buildRotMatrix3
    const double LogBase         = params[0];
    const double Scale           = params[1];    // .2Doubles — 1 raw value
    const double Z_add           = params[2];    // .2Doubles — 1 raw value
    const double Y_add           = params[3];
    const double X_add           = params[4];
    const double Saddle_str      = params[5];
    const int Apply_scale_add   = int(params[6]);
    // Constant: [edi+0x00] = 1.5

    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Apply 3x3 rotation matrix (3 raw angles → buildRotMatrix3)
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 7);
    // The x87 rotation block stores row0,row1,row2 to [esp],[esp+8],[esp+0x10].
    // With the column-view helper, row i is composed from col0.i / col1.i / col2.i.
    double rx = x * Rot.col0.x + y * Rot.col1.x + z * Rot.col2.x;
    double ry = x * Rot.col0.y + y * Rot.col1.y + z * Rot.col2.y;
    double rz = x * Rot.col0.z + y * Rot.col1.z + z * Rot.col2.z;

    // Step 3: Scherk surface computation
    // ASM trace (0xA1-0xBA):
    //   fld rx;       fcos -> cos(rx)
    //   fld ry;       fcos -> cos(ry)
    //   fdivp st(1)          -> cos(ry) / cos(rx)  (DE F9 = fdivp)
    //   fld LogBase          -> LogBase, ratio
    //   fxch st(1)           -> ratio, LogBase
    //   fabs                 -> |ratio|, LogBase
    //   fyl2x                -> LogBase * log2(|ratio|)
    //   fmul Saddle_str      -> Saddle_str * LogBase * log2(|ratio|)
    //   fsub z_rot            -> Saddle_str * LogBase * log2(|ratio|) - rz
    //
    // Note: fyl2x computes st(1) * log2(st(0)), pops st(0)
    // So result = LogBase * log2(|cos(ry)/cos(rx)|)
    // Then multiply by Saddle_str and subtract rz
    // shape_de = Saddle_str * LogBase * log2(|cos(ry)/cos(rx)|) - rz
    //
    // NOTE: the subtraction is fsub [esp+0x10] which is rz (stored after rotation)
    // So shape_de = strength * log_val - rz

    // Capstone opcode DE F9 is FDIVP ST(1),ST(0): after loading cos(rx) then cos(ry),
    // the quotient is cos(rx) / cos(ry), not the inverse.
    double ratio = cos(rx) / cos(ry);
    double log_val = LogBase * log2(fabs(ratio));
    double scherk_val = Saddle_str * log_val - rz;

    // Step 4: Compute max(|rz|, |ry|, |rx|)
    // ASM (0xBF-0xEF): three-way max of absolute values
    //   fld rz; fabs -> |rz|
    //   fld ry; fabs -> |ry|
    //   fcom st(1); if |ry| < |rz|: keep |rz|, else keep |ry| -> max_yz
    //   fld rx; fabs -> |rx|
    //   fcom st(1); if |rx| < max_yz: keep max_yz, else keep |rx| -> max_xyz
    double max_xyz = max(fabs(rx), max(fabs(ry), fabs(rz)));

    // Step 5: Cube distance
    // ASM (0xF0-0xF2): fld [edi] = constant 1.5; fsubp st(1) -> max_xyz - 1.5
    // Wait: DE E9 = fsubp st(1) = st(1) = st(1) - st(0), pop
    // Stack was [1.5, max_xyz, scherk_val]. After fsubp: [max_xyz - 1.5, scherk_val]
    double cube_dist = max_xyz - 1.5;

    // Step 6: Shape DE = max(cube_dist, scherk_val)
    // ASM (0xF5-0x102): fcom st(1); if cube_dist < scherk_val: keep scherk_val, else keep cube_dist
    double shape_de = max(cube_dist, scherk_val);

    // Step 7: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(state_x, state_y, state_z);
}

cpu_double3 ScherkFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_SCHERKFS>(p, params, de);
}

constant FormulaInfo formula_info_ScherkFS = {
    .id = FORMULA_SCHERKFS,
    .name = "ScherkFS",
    .param_count = 10,
    .complexity = 600,
    .uses_transcendental = true
};

/*
Original M3 Description:
Scherk's minimal surface (or at least, looks like it).
Due to this odd shape, the normals are not well defined everywhere especially
for high multiplier values, so keep them in a reasonable range,
or you will be forced to use very low raysteps!
I used this parametrization, don't know if it's really correct;

With logq = log in a given base
DE = z - strength* logq ( cos ( y ) / cos ( x ) )

Luca GN 2012

Mathematical Formula (from ASM trace):
  DEoption=20 (dIFS shape -- SSE2 IFS + rotation + Scherk surface):
    Step 1: IFS transform (SSE2 preamble)
    Step 2: 3x3 rotation from .3SingleAngles ScherkRot
    Step 3: Scherk surface:
      ratio = cos(ry) / cos(rx)
      scherk_val = Saddle_strength * LogBase * log2(|ratio|) - rz
    Step 4: Cube intersection: max(|rx|, |ry|, |rz|) - 1.5
    Step 5: shape_de = max(cube_dist, scherk_val)

Parameters:
  [0]  LogBase: double (default: 0.6) -- logarithm base coefficient
  [1]  Scale: double (default: 2) -- IFS scale (.2Doubles)
  [2]  Z_add: double (default: 0) -- Z offset (.2Doubles)
  [3]  Y_add: double (default: 0)
  [4]  X_add: double (default: 0)
  [5]  Saddle_strength: double (default: 0.6) -- surface strength
  [6]  Apply_scale_add: int (default: 0)
  [7..15]  ScherkRot: 9 singles (rotation matrix from .3SingleAngles)
  Constant: [edi+0x00] = 1.5

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = LogBase (double)
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Saddle_strength (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x50..-0x70] = Rotation matrix (9 singles)
  [PVar + 0x00] = 1.5 (constant)

Code structure:
  0x00-0x40: Standard SSE2 IFS preamble
  0x40-0x4B: Store to stack, nop
  0x4C-0x9E: 3x3 rotation (add edi,-0x2c; matrix mul; add edi,+0x2c)
  0xA1-0xA4: cos(rx)
  0xA6-0xA9: cos(ry)
  0xAB: fdivp -> cos(ry)/cos(rx)
  0xAE-0xB7: LogBase * log2(|ratio|) * Saddle_str
  0xBA: subtract rz
  0xBF-0xEF: max(|rz|, |ry|, |rx|)
  0xF0-0xF2: cube_dist = max_xyz - 1.5
  0xF5-0x102: max(cube_dist, scherk_val)
  0x104-0x113: Scale correction + store + return
*/
