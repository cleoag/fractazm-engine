// Sierpinski3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Sierpinski3
// Hand-ported from ASM: 229 bytes, DEoption=2
// Sierpinski tetrahedron with dual rotation matrices

constexpr unsigned int FORMULA_SIERPINSKI3 = 10170;

// ASM trace summary:
//   0x0D-0x11: Load z, y, x onto FPU stack -> ST0=x, ST1=y, ST2=z
//   0x13-0x4C: Apply Rotation1 3x3 matrix (singles at [esi-0x44]..[esi-0xC0])
//              Result: ST0=r0, ST1=r1, ST2=r2
//   0x4C-0x76: Sierpinski fold:
//     fchs -> ST0 = -r0
//     if -r0 < r1: swap(ST0,ST1)  (ensures max goes to ST0)
//     if ST0 < r2: swap(ST0,ST2)  (ensures max goes to ST0)
//     fchs -> ST0 = -ST0
//     xchg ST1, fchs -> negate ST1
//     if ST1 < r2: swap(ST1,ST2)
//     fchs -> restore sign
//     xchg ST1
//   0x76-0xAF: Apply Rotation2 3x3 matrix (singles at [esi-0x68]..[esi-0x64])
//   0xAF-0xC4: Scale + DE tracking:
//     de = de * Scale
//     x *= Scale, y *= Scale, z *= Scale
//     k = Scale - 1
//   0xC4-0xD5: Offset:
//     z -= k * CScaleZ
//     y -= k * CScaleY
//     x -= k * CScaleX
//   0xD7-0xDB: Store x, y, z

template<>
cpu_double3 execute_formula<FORMULA_SIERPINSKI3>(cpu_double3 p, double* params, double &de) {
    const double Scale   = params[0];   // [PVar-0x10] default: 2
    const double CScaleX = params[1];   // [PVar-0x18] default: 1
    const double CScaleY = params[2];   // [PVar-0x20] default: 1
    const double CScaleZ = params[3];   // [PVar-0x28] default: 1

    // Rotation1: pre-computed matrix from .3SingleAngles (9 floats at params[4..12])
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2: pre-computed matrix from .3SingleAngles (9 floats at params[13..21])
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Apply Rotation1 (R1 * [x, y, z])
    // PVar layout: M[0] at -0x2C, M[1] at -0x38, M[2] at -0x44
    // ASM computes Row0=M[2]·p (deepest→ST2), Row1=M[0]·p (→ST1), Row2=M[1]·p (last→ST0)
    // After R1: ST0=Row1=M[0]·p=a.x, ST1=Row2=M[1]·p=a.y, ST2=Row0=M[2]·p=a.z
    cpu_double3 a = R1 * cpu_double3(x, y, z);
    double A = a.x;   // ST0 = Row1 = M[0]·p
    double B = a.y;   // ST1 = Row2 = M[1]·p
    double C = a.z;   // ST2 = Row0 = M[2]·p

    // Step 2: Sierpinski tetrahedron fold
    // 0x4C: fchs -> -A; compare -A vs B; conditional swap
    double s0 = -A;
    if (s0 >= B) { double t = s0; s0 = B; B = t; }
    // 0x58: compare s0 vs C; conditional swap
    if (s0 >= C) { double t = s0; s0 = C; C = t; }
    // 0x62: fchs -> s0 = -s0
    s0 = -s0;

    // 0x64: fxch st(1) -> swap; 0x66: fchs -> negate
    double s1 = -B;
    // 0x68: compare s1 vs C; conditional swap
    if (s1 >= C) { double t = s1; s1 = C; C = t; }
    // 0x72: fchs
    s1 = -s1;

    // 0x74: fxch st(1) -> final reorder: ST0=s0, ST1=s1, ST2=C
    double f0 = s0;
    double f1 = s1;
    double f2 = C;

    // Step 3: Apply Rotation2 (R2 * [f0, f1, f2])
    // R2 input: ST0=f0 pushed first via fld st(0) at 0x76
    // After R2, stack is ST0=R2r1, ST1=R2r2, ST2=R2r0
    cpu_double3 b = R2 * cpu_double3(f0, f1, f2);

    // Step 4: Scale + DE tracking
    de = de * Scale;

    // Step 5: Offset
    // After R2: ST0=R2r1(=b.x), ST1=R2r2(=b.y), ST2=R2r0(=b.z)
    // 0xC6: k*CScaleZ -> fsubp st(4) -> R2r0*S -= k*CZ  (b.z)
    // 0xCD: k*CScaleY -> fsubp st(3) -> R2r2*S -= k*CY  (b.y)
    // 0xD2: k*CScaleX -> fsubp st(1) -> R2r1*S -= k*CX  (b.x)
    // Store: [ebx]=x=R2r1*S-k*CX, [edx]=y=R2r2*S-k*CY, [ecx]=z=R2r0*S-k*CZ
    double k = Scale - 1.0;
    double out_x = b.x * Scale - k * CScaleX;
    double out_y = b.y * Scale - k * CScaleY;
    double out_z = b.z * Scale - k * CScaleZ;

    return cpu_double3(out_x, out_y, out_z);
}

// Formula metadata
constant FormulaInfo formula_info_Sierpinski3 = {
    .id = FORMULA_SIERPINSKI3,
    .name = "Sierpinski3",
    .param_count = 10,
    .complexity = 700,
    .uses_transcendental = false
};

/*
Original M3 Description:
(No description in M3 file)

Mathematical Formula:
  1. Apply Rotation1 matrix to (x, y, z)
  2. Sierpinski tetrahedron fold:
     - Negate x, sort to bring max to front
     - Negate second, sort remaining pair
  3. Apply Rotation2 matrix
  4. Scale all by Scale, DE *= Scale
  5. Offset: subtract (Scale-1) * CScale from each axis

Parameters:
  [0]  Scale: double (default: 2)
  [1]  CScale_X: double (default: 1)
  [2]  CScale_Y: double (default: 1)
  [3]  CScale_Z: double (default: 1)
  [4-12]  Rotation1 matrix: 9 singles (from .3SingleAngles, default: identity)
  [13-21] Rotation2 matrix: 9 singles (from .3SingleAngles, default: identity)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScale_X
  [PVar-0x20] = CScale_Y
  [PVar-0x28] = CScale_Z
  [PVar-0x2C..-0x4C] = Rotation1 matrix (9 singles)
  [PVar-0x50..-0x70] = Rotation2 matrix (9 singles)

Note: Parameters [4-21] are pre-computed rotation matrix entries from
.3SingleAngles. When all angles are 0, this is the identity matrix.
*/
