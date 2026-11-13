// IcosahedronIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: IcosahedronIFS
// Hand-ported from ASM: 387 bytes, DEoption=2
// Icosahedral IFS with 3 reflections + dual rotation + scale + offset

constexpr unsigned int FORMULA_ICOSAHEDRONIFS = 10176;

// ASM trace summary:
//   0x0D-0x11: Load z, y, x -> ST0=x, ST1=y, ST2=z
//   0x13-0x52: Apply Rotation1 matrix, fabs after each row:
//     r0 = |x*R1[0,0] + y*R1[0,1] + z*R1[0,2]|
//     r1 = |x*R1[1,0] + y*R1[1,1] + z*R1[1,2]|
//     r2 = |x*R1[2,0] + y*R1[2,1] + z*R1[2,2]|
//   0x56-0x88: Reflection 1: dot = x*N0 + y*N1 + z*N2
//     if dot < 0: v -= 2*dot*N (mirror across plane)
//     then |y|, |z|
//   0x96-0xC8: Reflection 2 (same pattern)
//   0xD6-0x108: Reflection 3 (same pattern)
//   0x10A-0x112: Final fabs on two components
//   0x114-0x14B: Apply Rotation2 matrix
//   0x14D-0x160: Scale + DE: de *= Scale; all *= Scale; k = Scale - 1
//   0x162-0x173: Offset:
//     ST4 -= k * CScaleX [esi-0x18]
//     ST2 -= k * CScaleY [esi-0x20]
//     ST3 -= k * CScaleZ [esi-0x28]
//   0x175-0x179: Store [edx]=y, [ebx]=x, [ecx]=z

template<>
cpu_double3 execute_formula<FORMULA_ICOSAHEDRONIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // [PVar-0x10] default: 2
    const double CScaleX = params[1];   // [PVar-0x18] default: 0.809016994374947
    const double CScaleY = params[2];   // [PVar-0x20] default: 0.5
    const double CScaleZ = params[3];   // [PVar-0x28] default: 0

    // Rotation1 from 3 Euler angles (degrees)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 3 Euler angles (degrees)
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // [CONSTANTS] embedded as compile-time values:
    const double N0 = 0.309016994374947;   // [PVar+0x00]
    const double N1 = -0.5;               // [PVar+0x08]
    const double N2 = -0.809016994374947;  // [PVar+0x10]

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Apply Rotation1 with absolute value on each row
    cpu_double3 r1v = R1 * cpu_double3(x, y, z);
    double a0 = abs(r1v.x);
    double a1 = abs(r1v.y);
    double a2 = abs(r1v.z);

    x = a0;
    y = a1;
    z = a2;

    // Step 2: Three icosahedral reflections
    // Identity-hotspot parity shows the Pascal path uses the standard plane test
    // and correction order on the current (x, y, z) tuple.
    for (int i = 0; i < 3; i++) {
        double dot = x * N0 + y * N1 + z * N2;
        if (dot < 0.0) {
            double d2 = 2.0 * dot;
            x -= d2 * N0;
            y -= d2 * N1;
            z -= d2 * N2;
        }
        // After each reflection, take |y| and |z|
        // ASM: fxch/fabs pattern at 0x8A-0x94
        y = abs(y);
        z = abs(z);
    }

    // Step 3: Apply Rotation2 matrix
    // The x87 reload preserves the reflected component order as (x, y, z).
    cpu_double3 b_vec = R2 * cpu_double3(x, y, z);
    double b0 = b_vec.x;
    double b1 = b_vec.y;
    double b2 = b_vec.z;

    // Step 4: Scale + DE tracking
    de = de * Scale;
    b0 *= Scale;
    b1 *= Scale;
    b2 *= Scale;

    // Step 5: Offset (k = Scale - 1)
    double k = Scale - 1.0;
    // ASM: 0x166: k*CScaleX -> fsubp st(4) -> b1
    // 0x16B: k*CScaleY -> fsubp st(2) -> b0 (after stack shift)
    // 0x170: k*CScaleZ -> fsubp st(3) -> b2
    b0 -= k * CScaleX;
    b1 -= k * CScaleY;
    b2 -= k * CScaleZ;

    // Store: [edx]=y(b1), [ebx]=x(b0), [ecx]=z(b2)
    return cpu_double3(b0, b1, b2);
}

// Formula metadata
constant FormulaInfo formula_info_IcosahedronIFS = {
    .id = FORMULA_ICOSAHEDRONIFS,
    .name = "IcosahedronIFS",
    .param_count = 10,
    .complexity = 950,
    .uses_transcendental = true
};

/*
Original M3 Description:
(No description in M3 file - Icosahedral IFS)

Mathematical Formula:
  1. Apply Rotation1 matrix to (x, y, z), taking |result| of each row
  2. Three icosahedral reflections:
     dot = x*N0 + y*N1 + z*N2
     if dot < 0: p -= 2*dot*N
     then y=|y|, z=|z|
  3. Apply Rotation2 matrix
  4. Scale all by Scale, DE *= Scale
  5. Offset: subtract (Scale-1) * CScale from each axis

Parameters:
  [0]  Scale: double (default: 2)
  [1]  CScale_X: double (default: 0.809016994374947)
  [2]  CScale_Y: double (default: 0.5)
  [3]  CScale_Z: double (default: 0)
  [4-6]  Rotation1 angles (degrees, from .3SingleAngles)
  [7-9]  Rotation2 angles (degrees, from .3SingleAngles)
  [22] N0: double constant (0.309016994374947)
  [23] N1: double constant (-0.5)
  [24] N2: double constant (-0.809016994374947)

Icosahedral normal vector:
  N = (0.309016994374947, -0.5, -0.809016994374947)
  These are related to the golden ratio: cos(72deg)=0.309..., sin(30deg)=0.5, cos(36deg)=0.809...

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScale_X
  [PVar-0x20] = CScale_Y
  [PVar-0x28] = CScale_Z
  [PVar-0x2C..-0x4C] = Rotation1 matrix (9 singles)
  [PVar-0x50..-0x70] = Rotation2 matrix (9 singles)
  [PVar+0x00] = N0 constant
  [PVar+0x08] = N1 constant
  [PVar+0x10] = N2 constant
*/
