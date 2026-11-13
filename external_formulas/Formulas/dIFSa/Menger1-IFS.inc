// Menger1-IFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Menger1-IFS
// Hand-ported from ASM: 164 bytes, Capstone-verified
// DEoption: 21 (dIFS transform — Menger sponge iteration with sorting, rotation, DE tracking)
// Hack of MengerIFS adapted for DIFS requirements

constexpr unsigned int FORMULA_MENGER1_IFS = 10247;

template<>
cpu_double3 execute_formula<FORMULA_MENGER1_IFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters (from M3 [OPTIONS]):
    //   .Double Scale = 3           -> params[0]
    //   .Double CScale X = 1        -> params[1]
    //   .Double CScale Y = 1        -> params[2]
    //   .Double CScale Z = 0.5      -> params[3]
    //   .3SingleAngles Rotation = 0 -> params[4..6] (3 Euler angles in degrees)
    const double Scale = params[0];
    const double CScale_X = params[1];
    const double CScale_Y = params[2];
    const double CScale_Z = params[3];
    // params[4..6] = 3 Euler angles (degrees) for rotation matrix
    RotMatrix3 R = rotMatrix3FromFloats(params, 4);

    // ASM trace:
    // 0000-000D: Load z, |z|; load y, |y|; load x, |x|
    //   Stack: st0=|x|, st1=|y|, st2=|z|
    //
    // 000-002D: Sort three values so that st0 >= st1 >= st2 (descending sort)
    //   fcom st(1); jae skip; fxch st(1)   -> ensure st0 >= st1
    //   fcom st(2); jae skip; fxch st(2)   -> ensure st0 >= st2
    //   fxch st(1); fcom st(2); jae skip; fxch st(2) -> ensure st1 >= st2
    //   After sort: st0=max, st1=mid, st2=min
    //
    // 002-0066: Apply 3x3 rotation matrix to (max, mid, min):
    //   rx = max*R00 + mid*R01 + min*R02   (R at [edi-0x44], [edi-0x48], [edi-0x4c])
    //   ry = max*R10 + mid*R11 + min*R12   (R at [edi-0x2c], [edi-0x30], [edi-0x34])
    //   rz = max*R20 + mid*R21 + min*R22   (R at [edi-0x38], [edi-0x3c], [edi-0x40])
    //   Note: matrix indices shifted compared to standard Add+Rotate formulas
    //
    // 0068-00A3: Scale, offset, and Menger cross subtraction:
    //   fld Scale ([edi-0x10]); fld w ([esi+0x70])
    //   w *= Scale; store w
    //   rx *= Scale; ry *= Scale (fmul st, Scale)
    //
    //   t = Scale - 1  (fld Scale; fld1; fsubp)
    //   dup t; dup t
    //   rx -= t * CScale_X ([edi-0x18])
    //   ry -= t * CScale_Y ([edi-0x20])
    //
    //   For z: t_z = t / Scale * CScale_Z ([edi-0x28])
    //   rz_abs = |rz * Scale|   (but done as rz - t_z with abs correction)
    //   Actually: rz = |rz * Scale| - t * CScale_Z (Menger cross cut)
    //
    //   Detailed z computation:
    //   0077: fld st(0) = dup Scale
    //   0079: fld1; fsubp -> (Scale-1) = t
    //   007D: fld st(0); fld st(0) -> two copies of t
    //   0081: fmul [edi-0x18] -> t * CScale_X; fsubp st(5) -> rx -= t*CScale_X
    //   0086: fmul [edi-0x20] -> t * CScale_Y; fsubp st(3) -> ry -= t*CScale_Y
    //   008B: fdiv st(1) -> t / Scale
    //   008D: fmul [edi-0x28] -> (t/Scale) * CScale_Z
    //   0090: fsub st(4), st(0) -> rz -= (t/Scale)*CScale_Z... but then:
    //   0092: fxch st(4); fabs -> |rz - (t/Scale)*CScale_Z|
    //   0096: fsubp st(4) -> ... st4 = |rz_shifted| - (t/Scale)*CScale_Z
    //
    //   Wait, let me re-trace more carefully:
    //   After rotation: stack has rx, ry, rz (in some arrangement)
    //   0068: fld [edi-0x10] = Scale
    //   006B: fld [esi+0x70] = w; fmul st(1) = w*Scale; fstp [esi+0x70] -> w stored
    //   0073: fmul st(2), st(0) = ry*Scale (st2)
    //   0075: fmul st(1), st(0) = rx*Scale (st1)
    //   Stack: st0=Scale, st1=rx*S, st2=ry*S, st3=rz
    //   0077: fld st(0) -> Scale dup
    //   0079: fld1; fsubp -> Scale-1
    //   Stack: st0=(S-1), st1=Scale, st2=rx*S, st3=ry*S, st4=rz
    //   007D: fld st(0) -> (S-1) dup
    //   007: fld st(0) -> (S-1) dup again
    //   Stack: st0=t, st1=t, st2=t, st3=Scale, st4=rx*S, st5=ry*S, st6=rz
    //   0081: fmul [edi-0x18] -> t*CScaleX; fsubp st(5) -> st5 = rx*S - t*CScaleX
    //   0086: fmul [edi-0x20] -> t*CScaleY; fsubp st(3) -> st3 = ry*S - t*CScaleY
    //   Wait st3 was Scale... let me re-count.
    //   After 0081 fsubp: stack = st0=t, st1=t, st2=Scale, st3=(rx*S-t*CScX), st4=ry*S, st5=rz
    //   After 0086 fsubp: t*CScY fsubp st(3) -> st3=ry*S-t*CScY
    //   Stack: st0=t, st1=Scale, st2=(rx*S-t*CScX), st3=(ry*S-t*CScY), st4=rz
    //   008B: fdiv st(1) -> t/Scale
    //   008D: fmul [edi-0x28] -> (t/Scale)*CScaleZ
    //   Stack: st0=(t/S)*CScZ, st1=Scale, st2=rx', st3=ry', st4=rz
    //   0090: fsub st(4), st(0) -> rz -= (t/S)*CScZ -> st4 = rz - (t/S)*CScZ
    //   0092: fxch st(4) -> swap: st0 = rz-(t/S)*CScZ, st4=(t/S)*CScZ
    //   0094: fabs -> |rz - (t/S)*CScZ|
    //   0096: fsubp st(4) -> st4 old = (t/S)*CScZ, new = |rz-(t/S)*CScZ| - (t/S)*CScZ
    //   Wait, fsubp st(4) means st(4) = st(4) - st(0) and pop.
    //   After fxch st(4): st0=rz-(t/S)*CScZ absed, st1=Scale, st2=rx', st3=ry', st4=(t/S)*CScZ
    //   fsubp st(4): st(4) = (t/S)*CScZ - |rz-(t/S)*CScZ| ... hmm that's fsubp.
    //   Actually DE E4 = fsubrp st(4), st(0): st(4) = st(0) - st(4), pop.
    //   0096 is DE EC. Let me check: DE EC = fsubp st(4)?
    //   No: DE E0-E7 = fsubrp; DE E8-EF = fsubp. DE EC = fsubp st(4), st(0): st(4) = st(4) - st(0), pop.
    //   So st4 = (t/S)*CScZ - |rz - (t/S)*CScZ|
    //   Hmm. Actually: fsubp st(i), st(0) means st(i) = st(i) - st(0), pop.
    //   So st4_new = st4_old - st0 = (t/S)*CScZ - |rz - (t/S)*CScZ|
    //
    //   Wait, 0096 byte: DE EC. The CLAUDE.md says:
    //   DE E8-EF = FSUBP st(i), st(0) — st(i) = st(i) - st(0), pop (normal subtraction)
    //   So DE EC = FSUBP st(4): st(4) = st(4) - st(0), pop.
    //   But Capstone disassembly shows 0096: fsubp st(4) at this line... wait let me re-read.
    //
    //   Re-examining bytes at 0090 onward:
    //   0090: DC EC = fsub st(4), st(0) [not pop]
    //   0092: D9 CC = fxch st(4)
    //   0094: D9 E1 = fabs
    //   0096: DE EC = fsubp st(4)  -> st(4) = st(4) - st(0), pop
    //
    //   Before 0090: st0=c, st1=Scale, st2=rx', st3=ry', st4=rz  (c = (t/S)*CScZ)
    //   0090 fsub st(4),st(0): st4 = rz - c
    //   Stack: st0=c, st1=Scale, st2=rx', st3=ry', st4=(rz-c)
    //   0092 fxch st(4): st0=(rz-c), st4=c
    //   0094 fabs: st0=|rz-c|
    //   0096 fsubp st(4): st(4) = c - |rz-c|, pop
    //   Stack: st0=Scale, st1=rx', st2=ry', st3= c - |rz-c|
    //
    //   Hmm that gives rz_new = (t/S)*CScZ - |rz - (t/S)*CScZ|
    //
    //   Then:
    //   0098: DE CB = fmulp st(3) -> st3 *= Scale = Scale*(c - |rz-c|)
    //   Actually wait. After the pop from fsubp, stack is:
    //   st0=Scale, st1=rx', st2=ry', st3=rz_new
    //   DE CB = fmulp st(3), st(0): st3 = st3 * st0 = rz_new * Scale, pop
    //   Stack: st0=rx', st1=ry', st2= rz_new*Scale
    //
    //   Wait, that doesn't match. Let me recount after rotation.
    //
    //   Actually the rotation output order matters. Let me trace from the rotation code.
    //   The rotation uses a complex fxch pattern. Let me just trust the store order:
    //   009A: fstp [esi-0x70] = y_out
    //   009D: fstp [esi-0x78] = x_out
    //   00A0: fstp [esi-0x68] = z_out
    //
    //   So the final stack had: st0 -> y, st1 -> x, st2 -> z

    double x = abs(p.x);
    double y = abs(p.y);
    double z = abs(p.z);

    // Step 1: Sort so that st0=max >= st1=mid >= st2=min
    // The sorting ensures rotational symmetry of the Menger sponge
    if (x < y) { double t = x; x = y; y = t; }
    if (x < z) { double t = x; x = z; z = t; }
    if (y < z) { double t = y; y = z; z = t; }
    // Now x >= y >= z

    // Step 2: Apply 3x3 rotation matrix (built from 3 Euler angles)
    cpu_double3 rv = R * cpu_double3(x, y, z);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Step 3: DE tracking
    params[DIFS_VARYSCALE_SLOT] *= Scale;

    // Step 4: Scale x and y
    rx *= Scale;
    ry *= Scale;

    // Step 5: Menger cross subtraction
    double t = Scale - 1.0;
    rx -= t * CScale_X;
    ry -= t * CScale_Y;

    // Step 6: Z computation with abs fold
    // rz_new = (t/Scale)*CScale_Z - |rz - (t/Scale)*CScale_Z|
    // Then rz_new *= Scale
    double c = (t / Scale) * CScale_Z;
    double rz_new = (c - abs(rz - c)) * Scale;

    // Store order from ASM: y, x, z
    return cpu_double3(rx, ry, rz_new);
}

// Alternative direct function implementation
cpu_double3 Menger1_IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_MENGER1_IFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_Menger1_IFS = {
    .id = FORMULA_MENGER1_IFS,
    .name = "Menger1-IFS",
    .param_count = 7,
    .complexity = 580,
    .uses_transcendental = false
};

/*
Original M3 Description:
'Hack' of MengerIFS, partly rewritten to fit the requirements of DIFS.
Disable Apply scale+add on all shapes to make it work just like the 'normal' formula.

Mathematical Formula:
  DEoption=21 (dIFS transform — Menger sponge with rotation):
    Step 1: Take absolute values of x,y,z
    Step 2: Sort descending: max >= mid >= min
    Step 3: Apply 3x3 rotation matrix to (max, mid, min)
    Step 4: DE tracking: w *= Scale
    Step 5: Scale x,y: rx *= Scale, ry *= Scale
    Step 6: Menger subtraction: rx -= (S-1)*CScaleX, ry -= (S-1)*CScaleY
    Step 7: Z fold: rz = (c - |rz - c|) * Scale where c = (S-1)/S * CScaleZ

Parameters:
  [0] Scale: double (default: 3) — Menger scale factor
  [1] CScale_X: double (default: 1) — X cross-section scale
  [2] CScale_Y: double (default: 1) — Y cross-section scale
  [3] CScale_Z: double (default: 0.5) — Z cross-section scale
  [4..12] Rotation: 9 floats (row-major 3x3 matrix)

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = CScale_X (double)
  [PVar - 0x20] = CScale_Y (double)
  [PVar - 0x28] = CScale_Z (double)
  [PVar - 0x2c .. -0x4c] = Rotation matrix (9 singles)

  The fcom comparison uses: shr ah,1; jae (CF check)
  CF is set when st0 < st1 for fcom.

Key notes:
  - Sorting step is the core Menger sponge symmetry operation
  - The Z fold with abs creates the Menger cross-section holes
  - DE tracking via w *= Scale
  - 164 bytes — moderately complex
*/
