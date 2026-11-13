// CantorIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: CantorIFS
// Hand-ported from ASM: 235 bytes, DEoption=2
// 3D Cantor dust IFS with dual rotation matrices

constexpr unsigned int FORMULA_CANTORIFS = 10171;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CantorIFSOriginal_x87(double* state, const double* params);
#endif

// ASM trace summary:
//   0x0D-0x17: Load z (fabs), y (fabs), x (fabs) -> ST0=|x|, ST1=|y|, ST2=|z|
//   0x19-0x52: Apply Rotation1 3x3 matrix (singles at [esi-0x44]..[esi-0x40])
//   0x52-0x7A: Sierpinski-style fold (same as Sierpinski3):
//     fchs + compare/swap pattern for tetrahedron symmetry
//   0x7C-0xB3: Apply Rotation2 3x3 matrix (singles at [esi-0x70]..[esi-0x64])
//   0xB5-0xC8: Scale + DE: de *= Scale; all *= Scale; k = Scale - 1
//   0xCA-0xDB: Offset: subtract k * CScale from each axis
//     ST4 -= k * CScaleY [esi-0x20]
//     ST2 -= k * CScaleX [esi-0x18]
//     ST3 -= k * CScaleZ [esi-0x28]
//   0xDD-0xE1: Store [ebx]=x, [edx]=y, [ecx]=z

template<>
cpu_double3 execute_formula<FORMULA_CANTORIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    CantorIFSOriginal_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#else
    const double Scale   = params[0];   // [PVar-0x10] default: 3
    const double CScaleX = params[1];   // [PVar-0x18] default: 1
    const double CScaleY = params[2];   // [PVar-0x20] default: 1
    const double CScaleZ = params[3];   // [PVar-0x28] default: 1

    // Rotation1 from 3 Euler angles (degrees)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 3 Euler angles (degrees)
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // Step 1: Absolute values (Cantor dust uses absolute coordinates)
    double x = abs(p.x);
    double y = abs(p.y);
    double z = abs(p.z);

    // Step 2: Apply Rotation1 (R1 * [x, y, z])
    // PVar layout: M[0] at -0x2C, M[1] at -0x38, M[2] at -0x44
    // After R1: ST0=Row1=M[0]·p=a.x, ST1=Row2=M[1]·p=a.y, ST2=Row0=M[2]·p=a.z
    cpu_double3 a_vec = R1 * cpu_double3(x, y, z);
    double A = a_vec.x;   // ST0 = Row1 = M[0]·p
    double B = a_vec.y;   // ST1 = Row2 = M[1]·p
    double C = a_vec.z;   // ST2 = Row0 = M[2]·p

    // Step 3: Sierpinski tetrahedron fold (identical to Sierpinski3)
    // 0x52: fchs -> -A; compare with B; conditional swap
    double s0 = -A;
    if (s0 >= B) { double t = s0; s0 = B; B = t; }
    if (s0 >= C) { double t = s0; s0 = C; C = t; }
    s0 = -s0;

    double s1 = -B;
    if (s1 >= C) { double t = s1; s1 = C; C = t; }
    s1 = -s1;

    // After fold + fxch: ST0=s0, ST1=s1, ST2=C
    double f0 = s0;
    double f1 = s1;
    double f2 = C;

    // Step 4: Apply Rotation2 (R2 * [f0, f1, f2])
    // After R2: ST0=R2r1(=b.x), ST1=R2r2(=b.y), ST2=R2r0(=b.z)
    cpu_double3 b_vec = R2 * cpu_double3(f0, f1, f2);

    // Step 5: Scale + DE tracking
    de = de * Scale;

    // Step 6: Offset + Store
    // x = R2r1*S - k*CX = b.x*S - k*CX
    // y = R2r2*S - k*CY = b.y*S - k*CY
    // z = R2r0*S - k*CZ = b.z*S - k*CZ
    double k = Scale - 1.0;
    double out_x = b_vec.x * Scale - k * CScaleX;
    double out_y = b_vec.y * Scale - k * CScaleY;
    double out_z = b_vec.z * Scale - k * CScaleZ;

    return cpu_double3(out_x, out_y, out_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_CantorIFS = {
    .id = FORMULA_CANTORIFS,
    .name = "CantorIFS",
    .param_count = 10,
    .complexity = 710,
    .uses_transcendental = true
};

/*
Original M3 Description:
Modified from OctahedronIFS, another 'classic', the 3D Cantor dust.

Mathematical Formula:
  1. Take absolute values: x=|x|, y=|y|, z=|z|
  2. Apply Rotation1 matrix
  3. Sierpinski tetrahedron fold (negate-compare-swap pattern)
  4. Apply Rotation2 matrix
  5. Scale all by Scale, DE *= Scale
  6. Offset: subtract (Scale-1) * CScale from each axis

Parameters:
  [0]  Scale: double (default: 3)
  [1]  CScale_X: double (default: 1)
  [2]  CScale_Y: double (default: 1)
  [3]  CScale_Z: double (default: 1)
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
