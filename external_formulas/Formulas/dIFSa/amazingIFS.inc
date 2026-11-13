// amazingIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: amazingIFS
// Hand-ported from ASM: 236 bytes, Capstone-verified
// DEoption: 21 (dIFS transform — Amazing Box adapted for dIFS)
// Based on Amazing Box formula (and Kali's tweaks)

constexpr unsigned int FORMULA_AMAZINGIFS = 10252;

template<>
cpu_double3 execute_formula<FORMULA_AMAZINGIFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters (from M3 [OPTIONS]):
    //   .Double Add X = 0           -> params[0]
    //   .Double Add Y = 0           -> params[1]
    //   .Double Add Z = 0           -> params[2]
    //   .3SingleAngles Rotation = 5 -> params[3..5] (3 Euler angles in degrees)
    //   .Double Fold X = .5         -> params[12]
    //   .Double Fold Y = .5         -> params[13]
    //   .Double Fold Z = 0          -> params[14]
    //   .Double 1/Min R (gt or eq 1) = 31 -> params[15]
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // .3SingleAngles: 3 Euler angles → buildRotMatrix3
    RotMatrix3 RM = rotMatrix3FromFloats(params, 3);
    const double Fold_X = params[12];
    const double Fold_Y = params[13];
    const double Fold_Z = params[14];
    const double Inv_MinR = params[15];

    // ASM trace:
    // 0000-004: Amazing Box folding on z, y, x axes:
    //   For each axis (z first, then y, then x):
    //     fld fold_param; fld axis_value
    //     dup axis; fsub fold -> axis - fold; fabs -> |axis - fold|
    //     fadd fold -> |axis - fold| + fold
    //     fxch; axis + fold; fabs -> |axis + fold|
    //     fsubrp -> |axis - fold| + fold - |axis + fold|
    //       Wait: DE E1 = fsubrp st(1): st(1) = st(0) - st(1), pop
    //       So st(1) was the top = |axis - fold| + fold
    //       st(0) was |axis + fold|
    //       Result: |axis + fold| - (|axis - fold| + fold)... no.
    //       fsubrp st(1) means st(1) = st(0) - st(1), pop.
    //       st(0) = |axis + fold|
    //       st(1) = |axis - fold| + fold
    //       Result in st(now 0) = |axis + fold| - (|axis - fold| + fold)
    //
    //     Hmm, that doesn't simplify nicely. Let me re-trace the z fold (0x0000):
    //     0000: fld [edi-0x4c] = Fold_Z
    //     0003: fld [esi-0x68] = z
    //     Stack: st0=z, st1=Fold_Z
    //     0006: fld st(0) = z
    //     0008: fsub st(2) = z - Fold_Z
    //     000A: fabs = |z - Fold_Z|
    //     000C: fadd st(1) = |z - Fold_Z| + z... wait, D8 C1 = fadd st(0), st(1)
    //     After push of z copy: st0=z_copy, st1=z, st2=Fold_Z
    //     fsub st(2): st0 = z_copy - Fold_Z
    //     fabs: st0 = |z - Fold_Z|
    //     D8 C1 = fadd st(0), st(1): st0 = |z - Fold_Z| + z
    //     000E: fxch st(1): st0 = z, st1 = |z - Fold_Z| + z
    //     0010: D8 C2 = fadd st(0), st(2): st0 = z + Fold_Z
    //     0012: fabs: st0 = |z + Fold_Z|
    //     0014: DE E1 = fsubrp st(1): st(1) = st(0) - st(1) = |z + Fold_Z| - (|z - Fold_Z| + z), pop
    //     Stack: st0 = |z + Fold_Z| - |z - Fold_Z| - z, st1 = Fold_Z
    //     0016: fstp [esi-0x68] -> store new z
    //     0019: fstp st(0) -> pop Fold_Z
    //
    //   So: new_z = |z + Fold_Z| - |z - Fold_Z| - z
    //   This is the "amazing box fold": clamp(z, -Fold_Z, Fold_Z) * 2 - z
    //   Because: |a+f| - |a-f| = 2*clamp(a, -f, f) for any a
    //   So: new_z = 2*clamp(z, -FoldZ, FoldZ) - z
    //
    //   Same pattern repeated for y (0x001B-0x0034) and x (0x0036-0x004)
    //
    // 0051-009C: Standard add + rotation:
    //   z' = folded_z + Add_Z; y' = folded_y + Add_Y; x' = folded_x + Add_X
    //   Apply 3x3 rotation -> rx, ry, rz
    //
    // 009C-00EB: Sphere inversion (from Amazing Box) + DE scaling:
    //   r2 = rx^2 + ry^2 + rz^2
    //   factor = 1/r2
    //
    //   if factor < 1.0: skip scaling entirely (jae 0xCE -> store and return)
    //     (fcomp 1.0 vs factor; if 1.0 < factor => factor > 1: jae skip)
    //     Wait: D8 D9 = fcomp st(1) compares st0 vs st1 = 1.0 vs factor
    //     If 1.0 < factor: C0=1, shr CF=1, jae (CF=0) doesn't jump
    //     If 1.0 >= factor: C0=0, jae jumps
    //     Actually, shr ah,1 shifts C0 into CF. fcomp: C0=1 when st0 < operand.
    //     st0=1.0, st1=factor: if 1.0 < factor (factor > 1): C0=1, CF=1, jae doesn't jump
    //     So: if factor > 1 (r2 < 1): proceed with scaling
    //
    //   if factor > Inv_MinR: clamp to Inv_MinR
    //     fcom [edi-0x64] = Inv_MinR; if factor > Inv_MinR: replace with Inv_MinR
    //     Actually: DC 57 9C = fcom [edi-0x64]. Hmm, 0x9C relative to edi.
    //     [edi + 0x9C]... that's a positive offset from edi which is PVar.
    //     Wait no: edi-0x64 and the opcode is DC 57 9C.
    //     DC 57 = fcom qword ptr [edi+disp8]. disp8 = 0x9C = -100 signed = -0x64.
    //     So it's [edi - 0x64] = 1/MinR parameter.
    //
    //   After clamping, apply scaling:
    //   0x00DA: fmul st(1), st(0) -> rx *= factor
    //   0x00DC: fmul st(2), st(0) -> ry *= factor
    //   0x00DE: fmul st(3), st(0) -> rz *= factor
    //   0x00E0: fld [esi+0x70] -> w
    //   0x00E3: fmul st(1) -> w * factor
    //   0x00E5: fabs -> |w * factor|
    //   0x00E7: fstp [esi+0x70] -> store w
    //   0x00EA: jmp 0xCE -> go to store
    //
    // 0x00CE: Store results:
    //   fstp st(0) -> pop factor
    //   fstp [esi-0x78] -> x
    //   fstp [esi-0x70] -> y
    //   fstp [esi-0x68] -> z
    //   ret

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Step 1: Amazing Box fold on each axis
    // ASM trace: [edi-0x4c]="Fold_X" (params[12]) applied to Z, [edi-0x54]="Fold_Y" to Y,
    //            [edi-0x5c]="Fold_Z" (params[14]) applied to X.
    // The M3 naming does not match axis application — Z and X fold params are swapped.
    z = abs(z + Fold_X) - abs(z - Fold_X) - z;   // [edi-0x4c] = params[12] → Z axis
    y = abs(y + Fold_Y) - abs(y - Fold_Y) - y;   // [edi-0x54] = params[13] → Y axis
    x = abs(x + Fold_Z) - abs(x - Fold_Z) - x;   // [edi-0x5c] = params[14] → X axis

    // Step 2: Add offsets
    z += Add_Z;
    y += Add_Y;
    x += Add_X;

    // Step 3: Apply 3x3 rotation matrix
    // ASM stores output: [esi-0x78]=x=B(offsets -0x24..-0x2c), [esi-0x70]=y=C(-0x30..-0x38),
    //   [esi-0x68]=z=A(-0x3c..-0x44). buildRotMatrix3 row ordering matches standard (rv.x=B, rv.y=C, rv.z=A).
    cpu_double3 rv = RM * cpu_double3(x, y, z);
    double rx = rv.x;   // B = M[0]·v = stored as output x ([esi-0x78])
    double ry = rv.y;   // C = M[1]·v = stored as output y ([esi-0x70])
    double rz = rv.z;   // A = M[2]·v = stored as output z ([esi-0x68])

    // Step 4: Sphere inversion (Amazing Box MinR replacement)
    double r2 = rx * rx + ry * ry + rz * rz;
    double factor = 1.0 / r2;

    if (factor > 1.0) {
        // r2 < 1: apply sphere inversion, clamped to Inv_MinR
        if (factor > Inv_MinR) {
            factor = Inv_MinR;
        }
        rx *= factor;
        ry *= factor;
        rz *= factor;
        de = abs(de * factor);
    }

    return cpu_double3(rx, ry, rz);
}

// Alternative direct function implementation
cpu_double3 amazingIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_AMAZINGIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_amazingIFS = {
    .id = FORMULA_AMAZINGIFS,
    .name = "amazingIFS",
    .param_count = 10,
    .complexity = 820,
    .uses_transcendental = false
};

/*
Original M3 Description:
Based on Amazing Box formula (and Kali's tweaks), adapted for dIFS.
Seven iters are enough to get very detailed images.
EDIT; DE fix + added a replacement for MinR (don't ask me why or how but it seems to work)
Luca GN 2012

Mathematical Formula:
  DEoption=21 (dIFS transform — Amazing Box for dIFS):
    Step 1: Box fold each axis:
      axis = |axis + fold| - |axis - fold| - axis
           = 2 * clamp(axis, -fold, fold) - axis
    Step 2: Add offsets + 3x3 rotation
    Step 3: Sphere inversion:
      r2 = x^2 + y^2 + z^2
      factor = 1/r2
      if factor > 1 (r2 < 1):
        factor = min(factor, Inv_MinR)
        p *= factor
        w = |w * factor|

Parameters:
  [0] Add_X: double (default: 0)
  [1] Add_Y: double (default: 0)
  [2] Add_Z: double (default: 0)
  [3..5] Rotation angles: 3 Euler angles in degrees (from .3SingleAngles, default: 5)
  [6] Fold_X: double (default: 0.5)
  [7] Fold_Y: double (default: 0.5)
  [8] Fold_Z: double (default: 0)
  [9] Inv_MinR: double (default: 31) — max sphere inversion factor

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles)
  [PVar - 0x4c] = Fold_Z (double)... wait, fold order is Z,Y,X in ASM:
  Actually:
  [PVar - 0x4c] = Fold_Z (double)  [0000: fld qword [edi-0x4c]]
  [PVar - 0x54] = Fold_Y (double)  [001B: fld qword [edi-0x54]]
  [PVar - 0x5c] = Fold_X (double)  [0036: fld qword [edi-0x5c]]
  [PVar - 0x64] = Inv_MinR (double) [00BB: fcom qword [edi-0x64]]

Note:
  - Box fold is the standard Amazing Box/Mandelbox fold operation
  - Sphere inversion only applies when r2 < 1 (inside unit sphere)
  - |w * factor| ensures DE stays positive
  - Inv_MinR clamps the inversion to prevent extreme distortion
  236 bytes.
*/
