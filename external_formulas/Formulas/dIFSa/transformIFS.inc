// transformIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: transformIFS
// Hand-ported from ASM: 128 bytes, Capstone-verified
// DEoption: 21 (dIFS transform — scale, add, rotate, abs)
// Standard transform: scale, add, rotate, then conditional abs

constexpr unsigned int FORMULA_TRANSFORMIFS = 10244;

template<>
cpu_double3 execute_formula_dIFS_transform<FORMULA_TRANSFORMIFS>(cpu_double3 p, double* params,
                                                                   double &varyScale) {
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // params[3..11] are pre-computed rotation matrix elements (9 floats from .3SingleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const int Abs_X = int(params[12]);
    const int Abs_Y = int(params[13]);
    const int Abs_Z = int(params[14]);
    const double Scale = params[15];

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Scale and add offsets
    z = z * Scale + Add_Z;
    y = y * Scale + Add_Y;
    x = x * Scale + Add_X;

    // Step 1b: VaryScale tracking (w *= Scale in ASM, stored to [esi+0x70])
    varyScale *= Scale;

    // Step 2: Apply pre-computed 3x3 rotation matrix
    cpu_double3 rv = R * cpu_double3(x, y, z);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Step 3: Conditional absolute values
    if (Abs_X != 0) rx = abs(rx);
    if (Abs_Y != 0) ry = abs(ry);
    if (Abs_Z != 0) rz = abs(rz);

    return cpu_double3(rx, ry, rz);
}

template<>
cpu_double3 execute_formula<FORMULA_TRANSFORMIFS>(cpu_double3 p, double* params, double &de) {
    auto r = execute_formula_dIFS_transform<FORMULA_TRANSFORMIFS>(p, params, de);
    return r;
}

// Alternative direct function implementation
cpu_double3 transformIFS_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_TRANSFORMIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_transformIFS = {
    .id = FORMULA_TRANSFORMIFS,
    .name = "transformIFS",
    .param_count = 10,
    .complexity = 440,
    .uses_transcendental = false
};

/*
Original M3 Description:
(No description in M3 file)

Mathematical Formula:
  DEoption=21 (dIFS transform — with DE tracking via Scale):
    Step 1: Scale + Add:  p' = p * Scale + Add
    Step 1b: w *= Scale (DE tracking)
    Step 2: Rotate:  R * p'   (3x3 rotation matrix)
    Step 3: Conditional abs:  if Abs_X: |x|, if Abs_Y: |y|, if Abs_Z: |z|

  Order: scale+add -> rotate -> abs  (forward transform, opposite of transform2IFS)

Parameters:
  [0] Add_X: double (default: 0) — x offset (added before rotation)
  [1] Add_Y: double (default: 0)
  [2] Add_Z: double (default: 0)
  [3..11] Rotation: 9 floats (row-major 3x3 matrix from .3SingleAngles)
  [12] Abs_X: int (default: 0) — abs(x) after rotation
  [13] Abs_Y: int (default: 1) — abs(y) after rotation
  [14] Abs_Z: int (default: 0) — abs(z) after rotation
  [15] Scale: double (default: 1) — uniform scale factor

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles, row-major)
  [PVar - 0x48] = Abs_X (integer)
  [PVar - 0x4c] = Abs_Y (integer)
  [PVar - 0x50] = Abs_Z (integer)
  [PVar - 0x58] = Scale (double)

Key notes:
  - Unlike transform2IFS, this does scale+add FIRST, then rotate, then abs
  - DE tracking: w *= Scale (reads/writes [esi+0x70])
  - 128 bytes — 19 bytes longer than transform2IFS due to Scale
*/
