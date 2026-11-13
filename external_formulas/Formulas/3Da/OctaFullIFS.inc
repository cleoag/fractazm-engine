// OctaFullIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: OctaFullIFS
// Hand-ported from ASM: 235 bytes, DEoption=2
// Octahedron IFS with dual rotation matrices and full sorting fold

constexpr unsigned int FORMULA_OCTAFULLIFS = 10172;

// Original code: absolute coordinates, R1, ascending sort, R2, signed scale.
// R2 consumes [middle,minimum,maximum]; stores Y then X then Z.

template<>
cpu_double3 execute_formula<FORMULA_OCTAFULLIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // [PVar-0x10] default: 2
    const double CScaleX = params[1];   // [PVar-0x18] default: 1
    const double CScaleY = params[2];   // [PVar-0x20] default: 0
    const double CScaleZ = params[3];   // [PVar-0x28] default: 0

    // Rotation1 from 3 Euler angles (degrees)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 3 Euler angles (degrees)
    // R2 is evaluated below in original x87 operand order.

    // Step 1: Absolute values
    double x = abs(p.x);
    double y = abs(p.y);
    double z = abs(p.z);

    // Step 2: Apply Rotation1 (R1 * [x, y, z])
    // After R1: ST0=Row1=M[0]·|p|=a.x, ST1=Row2=M[1]·|p|=a.y, ST2=Row0=M[2]·|p|=a.z
    cpu_double3 a_vec = R1 * cpu_double3(x, y, z);
    double a0 = a_vec.x;  // ST0 = Row1 = M[0]·|p|
    double a1 = a_vec.y;  // ST1 = Row2 = M[1]·|p|
    double a2 = a_vec.z;  // ST2 = Row0 = M[2]·|p|

    // Step 3: Octahedral fold -- ASM sorts ASCENDING (ST0=min, ST1=mid, ST2=max)
    // Traced from ASM 0x52-0x7A: jb-based comparisons yield ascending order
    if (a0 > a1) { double t = a0; a0 = a1; a1 = t; }
    if (a0 > a2) { double t = a0; a0 = a2; a2 = t; }
    if (a1 > a2) { double t = a1; a1 = a2; a2 = t; }
    // Now: a0=vmin (ST0), a1=vmid (ST1), a2=vmax (ST2)

    // Step 4: Apply Rotation2
    // The final FXCH restores [minimum,middle,maximum] before R2.
    // Effective R2 input vector: [vmid, vmin, vmax] = [a1, a0, a2]
    const double st0 = a0;
    const double st1 = a1;
    const double st2 = a2;
    const double m50 = params[13], m54 = params[14], m58 = params[15];
    const double m5c = params[16], m60 = params[17], m64 = params[18];
    const double m68 = params[19], m6c = params[20], m70 = params[21];

    const double r2y = st1 * m5c + st0 * m60 + st2 * m64;
    const double r2x = st1 * m50 + st0 * m54 + st2 * m58;
    const double r2z = st1 * m68 + st0 * m6c + st2 * m70;

    // Step 5: Scale + DE tracking
    de = de * Scale;
    double k = Scale - 1.0;

    // Step 6: Offset + Store
    // ASM trace: after R2, ST0=Row1, ST1=Row0, ST2=Row2
    // After scale: ST0=Row1*S, ST1=Row0*S, ST2=Row2*S
    // After k=S-1 and offsets: fstp [edx]=y=Row1*S-k*CY, fstp [ebx]=x=Row0*S-k*CX, fstp [ecx]=z=Row2*S-k*CZ
    // b_vec.x = M2[0]·[vmid,vmin,vmax] = Row0; b_vec.y = M2[1]·... = Row1; b_vec.z = Row2
    double out_x = r2x * Scale - k * CScaleX;
    double out_y = r2y * Scale - k * CScaleY;
    double out_z = r2z * Scale - k * CScaleZ;
    return cpu_double3(out_x, out_y, out_z);
}

// Formula metadata
constant FormulaInfo formula_info_OctaFullIFS = {
    .id = FORMULA_OCTAFULLIFS,
    .name = "OctaFullIFS",
    .param_count = 10,
    .complexity = 700,
    .uses_transcendental = true
};

/*
Original M3 Description:
(No description in M3 file - Octahedral IFS variant)

Mathematical Formula:
  1. Take absolute values: x=|x|, y=|y|, z=|z|
  2. Apply Rotation1 matrix
  3. Octahedral fold: sort descending (max >= mid >= min), no sign changes
  4. Apply Rotation2 matrix
  5. Scale all by Scale, DE *= Scale
  6. Offset: subtract (Scale-1) * CScale from each axis

Parameters:
  [0]  Scale: double (default: 2)
  [1]  CScale_X: double (default: 1)
  [2]  CScale_Y: double (default: 0)
  [3]  CScale_Z: double (default: 0)
  [4-6]  Rotation1 angles (degrees, from .3SingleAngles)
  [7-9]  Rotation2 angles (degrees, from .3SingleAngles)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScale_X
  [PVar-0x20] = CScale_Y
  [PVar-0x28] = CScale_Z
  [PVar-0x2C..-0x4C] = Rotation1 matrix (9 singles)
  [PVar-0x50..-0x70] = Rotation2 matrix (9 singles)
*/
