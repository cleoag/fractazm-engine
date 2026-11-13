// transform2IFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: transform2IFS
// Hand-ported from ASM: 109 bytes, Capstone-verified
// DEoption: 21 (dIFS transform — inverse transform: abs, rotate, add)
// "Inverse" of transformIFS: does abs first, then rotates, then adds

constexpr unsigned int FORMULA_TRANSFORM2IFS = 10243;

template<>
cpu_double3 execute_formula<FORMULA_TRANSFORM2IFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters (from M3 [OPTIONS]):
    //   .Double Add X = 0           -> params[0]
    //   .Double Add Y = 0           -> params[1]
    //   .Double Add Z = 0           -> params[2]
    //   .3SingleAngles Rotation = 0 -> params[3..5] (3 Euler angles in degrees)
    //   .Integer Abs X = 0          -> params[12]
    //   .Integer Abs Y = 1          -> params[13]
    //   .Integer Abs Z = 0          -> params[14]
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // Build rotation matrix from 3 Euler angles (raw .3SingleAngles input)
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const int Abs_X = int(params[12]);
    const int Abs_Y = int(params[13]);
    const int Abs_Z = int(params[14]);

    // ASM trace:
    // 0000: fld [esi-0x68] -> load z
    // 0003: cmp [edi-0x50], 0; je; fabs  -> if Abs_Z: z = |z|
    // 000B: fld [esi-0x70] -> load y
    // 000E: cmp [edi-0x4c], 0; je; fabs  -> if Abs_Y: y = |y|
    // 0016: fld [esi-0x78] -> load x
    // 0019: cmp [edi-0x48], 0; je; fabs  -> if Abs_X: x = |x|
    //
    // Stack after loads: st0=x, st1=y, st2=z
    //
    // 0021-004B: Standard 3x3 rotation matrix multiply:
    //   rx = x*R[0,0] + y*R[0,1] + z*R[0,2]   (using [edi-0x3c], [edi-0x40], [edi-0x44])
    //   ry = x*R[1,0] + y*R[1,1] + z*R[1,2]   (using [edi-0x24], [edi-0x28], [edi-0x2c])
    //   rz = x*R[2,0] + y*R[2,1] + z*R[2,2]   (using [edi-0x30], [edi-0x34], [edi-0x38])
    //
    //   Note: rotation code uses fxch tricks to compute all three rows simultaneously.
    //   The matrix is row-major with Single-precision entries.
    //
    // 005A-0068: Add offsets and store results:
    //   x_out = rx + Add_X  -> fstp [esi-0x78]
    //   y_out = ry + Add_Y  -> fstp [esi-0x70]
    //   z_out = rz + Add_Z  -> fstp [esi-0x68]

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Conditional absolute values (done FIRST in this variant)
    if (Abs_Z != 0) z = abs(z);
    if (Abs_Y != 0) y = abs(y);
    if (Abs_X != 0) x = abs(x);

    // Step 2: Apply 3x3 rotation matrix
    cpu_double3 rv = R * cpu_double3(x, y, z);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Step 3: Add translation offsets
    // ASM: fadd [edi-0x10] = Add_X, fadd [edi-0x18] = Add_Y, fadd [edi-0x20] = Add_Z
    rx += Add_X;
    ry += Add_Y;
    rz += Add_Z;

    return cpu_double3(rx, ry, rz);
}

// Alternative direct function implementation
cpu_double3 transform2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TRANSFORM2IFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_transform2IFS = {
    .id = FORMULA_TRANSFORM2IFS,
    .name = "transform2IFS",
    .param_count = 9,
    .complexity = 380,
    .uses_transcendental = false
};

/*
Original M3 Description:
"Inverse" of transform. Does everything in reversed order, because
normal transform adds, rotates, does abs while this does abs at first,
then rotates and finally adds. Useful for some shape transforms at low iters.

Mathematical Formula:
  DEoption=21 (dIFS transform):
    Step 1: Conditional absolute value on each axis (Abs X/Y/Z flags)
    Step 2: Apply 3x3 rotation matrix (from .3SingleAngles)
    Step 3: Add translation offsets (Add X/Y/Z)

  Order: abs -> rotate -> add  (reverse of transformIFS)

Parameters:
  [0] Add_X: double (default: 0) — x translation
  [1] Add_Y: double (default: 0) — y translation
  [2] Add_Z: double (default: 0) — z translation
  [3..11] Rotation: 9 floats (row-major 3x3 matrix from .3SingleAngles)
  [12] Abs_X: int (default: 0) — 0=no abs, nonzero=abs(x)
  [13] Abs_Y: int (default: 1) — 0=no abs, nonzero=abs(y)
  [14] Abs_Z: int (default: 0) — 0=no abs, nonzero=abs(z)

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles, row-major)
  [PVar - 0x48] = Abs_X (integer)
  [PVar - 0x4c] = Abs_Y (integer)
  [PVar - 0x50] = Abs_Z (integer)

Note: Abs checks use cmp [edi-offset], 0 — any nonzero value enables abs.
109 bytes — simple transform formula.
*/
