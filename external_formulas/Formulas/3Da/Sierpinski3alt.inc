// Sierpinski3alt.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Sierpinski3alt
// Hand-ported from ASM: 219 bytes, DEoption=2
// Alternate Sierpinski tetrahedron with single rotation + offset center

constexpr unsigned int FORMULA_SIERPINSKI3ALT = 10024;

// ASM trace summary:
//   0x0D-0x11: Load z, y, x -> ST0=x, ST1=y, ST2=z
//   0x14: fchs -> ST0=-x
//   0x16-0x1E: Fold1: if -x >= y then swap(ST0,ST1)
//   0x20-0x28: Fold2: if ST0 >= z then swap(ST0,ST2)
//   0x2A-0x2E: Negate ST0, swap, negate -> rearrange
//   0x30-0x38: Fold3: if ST0 >= ST2 then swap(ST0,ST2)
//   0x3A-0x3C: Negate ST0, swap
//   Result: Sierpinski tetrahedron edge folds
//   0x3-0x52: Add rotation center offsets (dX to ST0, dY to ST1, dZ to ST2)
//   0x55-0x8C: Apply 3x3 rotation matrix -> r0, r1, r2 (stack: r1, r2, r0)
//   0x8-0xA2: Subtract rotation center offsets (dX from Row0=ST0, dY from Row1=ST1, dZ from Row2=ST2)
//   0xA5-0xB4: Scale + DE: de *= Scale; all *= Scale
//   0xB6-0xCB: Offset: subtract (Scale-1)*CScale from each
//   0xCD-0xD1: Store x=(Row0-dX)*S-k*CScaleX, y=(Row1-dY)*S-k*CScaleY, z=(Row2-dZ)*S-k*CScaleZ

template<>
cpu_double3 execute_formula<FORMULA_SIERPINSKI3ALT>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // [PVar-0x10] default: 2
    const double CScaleX = params[1];   // [PVar-0x18] default: 1
    const double CScaleY = params[2];   // [PVar-0x20] default: 1
    const double CScaleZ = params[3];   // [PVar-0x28] default: 1
    const double dX      = params[4];   // [PVar-0x30] default: 0.5
    const double dY      = params[5];   // [PVar-0x38] default: 0.5
    const double dZ      = params[6];   // [PVar-0x40] default: 0.5

    // --- Sierpinski tetrahedron edge folds (on signed coordinates) ---
    // Keep the x87 stack flow directly instead of the pair-sum rewrite.
    double s0 = -p.x;   // 0x14 fchs
    double s1 =  p.y;
    double s2 =  p.z;

    // 0x16-0x1E: if -x >= y then swap(ST0,ST1)
    if (s0 >= s1) { double t = s0; s0 = s1; s1 = t; }
    // 0x20-0x28: if ST0 >= z then swap(ST0,ST2)
    if (s0 >= s2) { double t = s0; s0 = s2; s2 = t; }
    // 0x2A-0x2E: negate ST0, swap, negate
    s0 = -s0;
    { double t = s0; s0 = s1; s1 = t; }
    s0 = -s0;
    // 0x30-0x38: if ST0 >= ST2 then swap(ST0,ST2)
    if (s0 >= s2) { double t = s0; s0 = s2; s2 = t; }
    // 0x3A-0x3C: negate ST0, swap
    s0 = -s0;
    { double t = s0; s0 = s1; s1 = t; }

    double fx = s0;
    double fy = s1;
    double fz = s2;

    // Add rotation center offsets
    double p0 = fx + dX;
    double p1 = fy + dY;
    double p2 = fz + dZ;

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 7);

    // Apply rotation matrix
    cpu_double3 rv = R * cpu_double3(p0, p1, p2);
    double r0 = rv.x;
    double r1 = rv.y;
    double r2 = rv.z;

    // Subtract rotation center offsets (ASM: dX sub from Row0=ST0, dY from Row1=ST1, dZ from Row2=ST2)
    // Store order: Row0-dX -> x, Row1-dY -> y, Row2-dZ -> z
    double v0 = r0 - dX;
    double v1 = r1 - dY;
    double v2 = r2 - dZ;

    // Scale + DE tracking
    de *= Scale;
    v0 *= Scale;
    v1 *= Scale;
    v2 *= Scale;

    // Offset: subtract (Scale-1) * CScale
    double k = Scale - 1.0;
    double out_x = v0 - k * CScaleX;
    double out_y = v1 - k * CScaleY;
    double out_z = v2 - k * CScaleZ;

    return cpu_double3(out_x, out_y, out_z);
}

// Alternative direct function implementation
cpu_double3 Sierpinski3alt_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_SIERPINSKI3ALT>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_Sierpinski3alt = {
    .id = FORMULA_SIERPINSKI3ALT,
    .name = "Sierpinski3alt",
    .param_count = 10,
    .complexity = 700,
    .uses_transcendental = true
};

/*
Original M3 Description:
Alternate version of Sierpinski3.
Instead of two rotations only one but with selectable center.
Rotation1 is a pre-rotation in reality (less useful I think).
It should be obvious that since you leave all rotations zero you will never notice any effect.
Plus, you will see that dX and rotation X are independent; only dY and dZ will modify it.
Same for all other axis.

"Hacked" from Jesse's formula (Luca GN 2011). Idea taken from the original KIFS thread.

Mathematical Formula:
  1. Sierpinski tetrahedron edge folds (no abs):
     if (x + y < 0) { swap+negate x,y }
     if (x + z < 0) { swap+negate x,z }
     if (y + z < 0) { swap+negate y,z }
  2. Add rotation center offset: p += d
  3. Rotate: r = R * p
  4. Subtract offset: v = (r0-dX, r1-dY, r2-dZ)
  5. Scale: v *= Scale, de *= Scale
  6. Offset: result = v - (Scale-1) * CScale

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 1)
  [4] Rot_dX: double (default: 0.5) - rotation center X
  [5] Rot_dY: double (default: 0.5) - rotation center Y
  [6] Rot_dZ: double (default: 0.5) - rotation center Z
  [7..9] Rotation2 angles (degrees)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScale_X
  [PVar-0x20] = CScale_Y
  [PVar-0x28] = CScale_Z
  [PVar-0x30] = Rot_dX
  [PVar-0x38] = Rot_dY
  [PVar-0x40] = Rot_dZ
  [PVar-0x44..0x64] = Rotation2 matrix (9 singles)
    Row1(R10,R11,R12) at [0x44,0x48,0x4c]
    Row2(R20,R21,R22) at [0x50,0x54,0x58]
    Row0(R00,R01,R02) at [0x5c,0x60,0x64]
*/
