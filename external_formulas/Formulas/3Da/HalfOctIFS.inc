// HalfOctIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: HalfOctIFS
// Hand-ported from ASM: 225 bytes, DEoption=2
// Half-octahedron IFS with two rotations and partial fold

constexpr unsigned int FORMULA_HALFOCTIFS = 10026;

// ASM trace summary:
//   0x0D-0x11: Load z, y, x -> ST0=x, ST1=y, ST2=z (no abs!)
//   0x13-0x4A: Apply Rotation1 matrix (offsets [esi-0x2c..0x4c])
//              buildRotMatrix3 assigns M[0] to lowest-magnitude offsets:
//              RB = x*[-0x2c]+y*[-0x30]+z*[-0x34] -> M[0] -> r1v.x, lands at ST0
//              RC = x*[-0x38]+y*[-0x3c]+z*[-0x40] -> M[1] -> r1v.y, lands at ST1
//              RA = x*[-0x44]+y*[-0x48]+z*[-0x4c] -> M[2] -> r1v.z, lands at ST2
//              After R1: ST0=r1v.x(=RB), ST1=r1v.y(=RC), ST2=r1v.z(=RA)
//   0x4C-0x54: Fold1: fcom st(1); shr ah,1; jae 0x56; fxch st(1)
//              CF=1 when ST0<ST1; jae skips if ST0>=ST1; swap if ST0<ST1
//              -> ensures ST0=max(RB,RC), ST1=min
//   0x56: fchs -> ST0=-max
//   0x58-0x60: Fold2: fcom st(1); shr ah,1; jb 0x62; fxch st(1)
//              jb skips if -max<min (sum>0); swap if -max>=min (sum<=0)
//   0x62: fchs -> negate ST0
//   0x64-0x6C: Fold3: fcom st(2); shr ah,1; jb 0x6E; fxch st(2)
//              jb skips if ST0<RA; swap ST0<->RA if ST0>=RA
//   0x6E: fchs -> negate ST0
//   0x70: fxch st(1) -> final reorder; result: ST0=RC(=fa), ST1=-min(fold,RA)(=fb), ST2=max(=fc)
//   0x72-0xA9: Apply Rotation2; input vector order is (fb, fa, fc):
//              qB=fb*[-0x50]+fa*[-0x54]+fc*[-0x58] -> M[0] -> r2v.x -> stored to x
//              qC=fb*[-0x5c]+fa*[-0x60]+fc*[-0x64] -> M[1] -> r2v.y -> stored to y
//              qA=fb*[-0x68]+fa*[-0x6c]+fc*[-0x70] -> M[2] -> r2v.z -> stored to z
//   0xAB-0xD9: de*=Scale; result = R2*(fb,fa,fc)*Scale - (Scale-1)*CScale
//   Store: fstp [edx]=y, fstp [ebx]=x, fstp [ecx]=z

template<>
cpu_double3 execute_formula<FORMULA_HALFOCTIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // [PVar-0x10] default: 2
    const double CScaleX = params[1];   // [PVar-0x18] default: 0
    const double CScaleY = params[2];   // [PVar-0x20] default: 0
    const double CScaleZ = params[3];   // [PVar-0x28] default: 1

    // Rotation1 from 3 Euler angles (degrees)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    // Rotation2 from 3 Euler angles (degrees)
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // Step 1: Apply Rotation1 (no abs — signed input coordinates)
    // After R1: r1v.x=RB lands at ST0, r1v.y=RC at ST1, r1v.z=RA at ST2
    cpu_double3 r1v = R1 * p;
    double sB = r1v.x;  // ST0 after R1 (row with offsets -0x2c,-0x30,-0x34)
    double sC = r1v.y;  // ST1 after R1 (row with offsets -0x38,-0x3c,-0x40)
    double sA = r1v.z;  // ST2 after R1 (row with offsets -0x44,-0x48,-0x4c)

    // Step 2: Half-octahedral folds (stack: ST0=sB, ST1=sC, ST2=sA)

    // Fold1 (0x4C-0x54): fcom st(1); shr ah,1; jae 0x56; fxch st(1)
    // CF=1 means sB<sC; jae skips when CF=0 (sB>=sC); swap when sB<sC
    // Result: ST0=max(sB,sC), ST1=min(sB,sC)
    if (sB < sC) { double t = sB; sB = sC; sC = t; }

    // Fold2 (0x56-0x62): edge fold
    // 0x56: fchs -> ST0=-sB (negate max)
    // 0x58: fcom st(1): compare -sB vs sC
    // 0x5E: jb skips if CF=1 (-sB<sC, i.e. sB+sC>0); swap when -sB>=sC (sB+sC<=0)
    // 0x60: fxch st(1)
    // 0x62: fchs -> negate ST0
    double neg_sB = -sB;
    if (neg_sB >= sC) {
        double t = neg_sB; neg_sB = sC; sC = t;
    }
    sB = -neg_sB;  // 0x62 fchs; stack restored to (sB, sC, sA) form

    // Fold3 (0x64-0x70): compare current ST0=sB against ST2=sA, then negate ST0 and swap with ST1.
    // Important: after Fold1, ST1 is the minimum of the original (RB,RC) pair. It is NOT always the original sC.
    // 0x64: fcom st(2): compare ST0=sB vs ST2=sA
    // 0x6A: jb skips if sB<sA; 0x6C: fxch st(2) swaps when sB>=sA
    // 0x6E: fchs -> negate ST0
    // 0x70: fxch st(1) -> final reorder
    // Final stack:
    //   if sB >= sA: (min(RB,RC), -sA, sB)
    //   else:        (min(RB,RC), -sB, sA)
    double fa, fb, fc;
    if (sB >= sA) {
        // fxch st(2): ST=(sA,sC,sB) -> fchs: ST=(-sA,sC,sB) -> fxch st(1): ST=(sC,-sA,sB)
        // Here sC already carries min(RB,RC) after Fold1/Fold2.
        fa = sC;
        fb = -sA;
        fc = sB;
    } else {
        // no swap: ST=(sB,sC,sA) -> fchs: ST=(-sB,sC,sA) -> fxch st(1): ST=(sC,-sB,sA)
        // Here sC already carries min(RB,RC) after Fold1/Fold2.
        fa = sC;
        fb = -sB;
        fc = sA;
    }

    // Step 3: Apply Rotation2
    // The asm feeds Rotation2 with (fb, fa, fc). The resulting qB/qC/qA land in
    // x/y/z respectively, matching the standard row-major RotMatrix3 multiply.
    cpu_double3 r2v = R2 * cpu_double3(fb, fa, fc);

    // Step 4: Scale + DE + linear offset
    // 0xAB-0xB3: de *= Scale
    // 0xB6-0xBA: q_vec *= Scale (via DC CB/CA/C9)
    // 0xBC-0xBE: k = Scale - 1 (fld1; fsubp st(1))
    // 0xC4-0xD1: result -= k * CScale (fmul+fsubp for each axis)
    // 0xD3-0xD7: store y, x, z
    de *= Scale;
    double k = Scale - 1.0;

    return r2v * Scale - k * cpu_double3(CScaleX, CScaleY, CScaleZ);
}

// Alternative direct function implementation
cpu_double3 HalfOctIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HALFOCTIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_HalfOctIFS = {
    .id = FORMULA_HALFOCTIFS,
    .name = "HalfOctIFS",
    .param_count = 10,
    .complexity = 800,
    .uses_transcendental = true
};

/*
Original M3 Description:
A 3D half-octahedron attempt; same as normal one but the third fold is dropped.
Nice shapes can be obtained with this variation.

Luca GN 2011

Mathematical Formula:
  1. Apply Rotation1 to (x, y, z) - signed coordinates, no abs
  2. Half-octahedral folds on rotated vector (ST0=RB=r1v.x, ST1=RC=r1v.y, ST2=RA=r1v.z):
     a. Sort ST0,ST1: ensure max is at ST0
     b. Edge fold: if -(max) >= min (sum<=0) swap and negate
     c. Sort ST0 vs RA: if ST0>=RA swap, then negate ST0, fxch ST1
     Result: (fa=RC, fb=-min(folded,RA), fc=max(folded,RA))
  3. Apply Rotation2 with input (fb, fa, fc)
  4. Scale: de *= Scale; result = R2_out * Scale - (Scale-1) * CScale

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 0)
  [2] CScale_Y: double (default: 0)
  [3] CScale_Z: double (default: 1)
  [4..6] Rotation1 angles (degrees) - .3SingleAngles
  [7..9] Rotation2 angles (degrees) - .3SingleAngles

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScale_X
  [PVar-0x20] = CScale_Y
  [PVar-0x28] = CScale_Z
  [PVar-0x2C..0x4C] = Rotation1 matrix (9 singles)
  [PVar-0x50..0x70] = Rotation2 matrix (9 singles)
*/
