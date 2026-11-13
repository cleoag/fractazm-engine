// YinYangIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: YinYangIFS
// Hand-ported from ASM: 355 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS transform + x87 Yin-Yang torus/sphere DE)
// Tao symbol: two toruses and spheres with optional 3SingleAngles rotation

constexpr unsigned int FORMULA_YINYANGIFS = 10218;

// ASM trace (x87 section, after SSE2 preamble stores coords to stack):
//
// The formula uses .3SingleAngles "YinYangRot" which pre-computes a 3x3 rotation
// matrix stored as 9 floats (single-precision) at negative offsets from edi.
// After "add edi, -0x34" at 0x4C, the rotation matrix entries are at:
//   [edi-0x3c]..[edi-0x24] = row0 (float32 each)
//   [edi-0x24]..[edi-0x0c] = row1 (note: shared with row0 end)
// Actually the 9 matrix entries M[i] (single!) are at:
//   M0=[edi-0x3c], M1=[edi-0x40], M2=[edi-0x44]   (fmul dword)
//   M3=[edi-0x24], M4=[edi-0x28], M5=[edi-0x2c]
//   M6=[edi-0x30], M7=[edi-0x34], M8=[edi-0x38]
//
// 0x4C: add edi, -0x34  (shift base for rotation matrix access)
// 0x4-0x55: fld z, fld y, fld x  -> stack: x, y, z
//
// Rotation: rx = x*M0 + y*M1 + z*M2
//           ry = x*M3 + y*M4 + z*M5
//           rz = x*M6 + y*M7 + z*M8
// (standard 3x3 * vec3 via x87 FPU, 0x5A-0x91)
//
// 0x93-0x9D: store rotated x,y,z back to stack, restore edi
//
// Shape DE computation:
// Torus 1: sqrt(sqrt(rx^2 + ry^2) - R1)^2 + rz^2) - R2
//   0xA2: rx^2
//   0xA7: + ry^2
//   0xAF: sqrt -> r_xy
//   0xB1: - R1 (twice, i.e. -2*R1)... wait no:
//   0xB1: fsub R1 [edi-0x10]
//   0xB4: fsub R1 [edi-0x10]  -> r_xy - 2*R1
//   Actually looking more carefully: this subtracts R1 twice from r_xy.
//   But that gives (r_xy - 2*R1). Let me re-read...
//   0xA2: fld [esp], D8 C8 = fmul st0,st0 -> x^2
//   0xA7: fld [esp+8], D8 C8 -> y^2, faddp -> x^2+y^2
//   0xAF: fsqrt -> r_xy = sqrt(x^2+y^2)
//   0xB1: DC 67 F0 = fsub qword [edi-0x10] -> r_xy - Radius1
//   0xB4: DC 67 F0 = fsub qword [edi-0x10] -> (r_xy - Radius1) - Radius1 = r_xy - 2*Radius1
//   Hmm, that's unusual. But let me re-check the hex: DC 67 F0 DC 67 F0 yes, same instruction twice.
//   So ring1 = r_xy - 2*Radius1.
//   0xB7: D8 C8 = fmul st0 -> ring1^2
//   0xB9: fld [esp+0x10] -> z
//   0xBD: D8 C8 -> z^2
//   0xBF: faddp -> ring1^2 + z^2
//   0xC1: fsqrt -> sqrt(ring1^2 + z^2)
//   0xC3: DC 67 B8 = fsub [edi-0x48] -> subtract Radius2 = "Radius Spheres"
//   -> torus1_de = sqrt((r_xy - 2*R1)^2 + z^2) - RadSph
//
// Sphere displaced in x:
//   0xC6: fld [esp] = rx
//   0xC9: D9 E1 = fabs -> |rx|
//   0xCB: DD 47 F0 = fld [edi-0x10] = R1
//   0xCE: DC 0 = fmul [edi] = R1 * const[0] = R1 * (-1.0) = -R1
//   0xD0: faddp -> |rx| + (-R1) = |rx| - R1
//   0xD2: D8 C8 = square -> (|rx| - R1)^2
//   0xD4: fld [esp+8] -> ry
//   0xD8: D8 C8 = square -> ry^2
//   0xDA: faddp -> (|rx| - R1)^2 + ry^2
//   0xDC: fld [esp+0x10] -> rz
//   0xE0: D8 C8 = square -> rz^2
//   0xE2: faddp -> (|rx| - R1)^2 + ry^2 + rz^2
//   0xE4: fsqrt -> r_sphere
//   0xE6: DC 67 AC = fsub [edi-0x54] -> sphere_de = r_sphere - BottleThickness
//   Wait, [edi-0x54] maps to "Bottle thickness" in KleinBotTIFS... no, this is YinYangIFS.
//   The params: R1=[edi-0x10], R2=[edi-0x48], [edi-0x54]="Radius Spheres" at offset -0x54?
//   Actually: Radius1=[edi-0x10], Scale at [edi-0x20], Z_add at [edi-0x30],
//   Y_add=[edi-0x38], X_add=[edi-0x40], R2=[edi-0x48],
//   Apply_scale_add=[edi-0x4c], RadSph=[edi-0x54].
//   Wait but [edi-0x48] is loaded at 0xC3 (Radius2 / "Radius Spheres" from M3 = 0.2),
//   and [edi-0x54] at 0xE6 would be... let me re-check M3:
//   .Double Radius 1 = .5          -> [PVar-0x10]
//   .2Doubles Scale = 1            -> [PVar-0x20] (16 bytes)
//   .2Doubles Z add = 0            -> [PVar-0x30] (16 bytes)
//   .Double Y add = 0              -> [PVar-0x38]
//   .Double X add = 0              -> [PVar-0x40]
//   .Double Radius 2 = 0.1         -> [PVar-0x48]
//   .Integer Apply scale+add = 1   -> [PVar-0x4c]
//   .Double Radius Spheres = 0.2   -> [PVar-0x54]
//   .3SingleAngles YinYangRot = 0  -> (rotation matrix, offsets vary)
//   [CONSTANTS] Double = -1.0      -> [PVar+0x00] = [edi]
//
//   So [edi-0x48] = Radius2 = 0.1 and [edi-0xB8] = RadiusSpheres but wait,
//   Actually "Radius Spheres" = 0.2 is at [PVar-0x54].
//   But at 0xC3 the ASM does fsub [edi-0x48] = Radius2 for the torus tube.
//   Actually looking again: after "add edi, -0x34" and then "add edi, 0x34" at 0x9E,
//   edi is restored. So [edi-0x10] = Radius1, [edi-0x48] = Radius2,
//   [edi-0x54] = RadiusSpheres. But wait, [edi-0xB8]? No, the preamble already
//   has edi pointing at PVar. After restoration:
//   [edi-0x48] = Radius2 = 0.1
//   [edi-0xB8]... no, [edi-0x48] from M3 is "Radius 2".
//   But fsub at 0xC3 is DC 67 B8 = fsub [edi-0x48]? Let me decode: DC = fop qword,
//   67 would be... DC 67 B8: ModRM = 0x67, which is [edi+disp8], disp8 = 0xB8 = -0x48 signed.
//   Yes, [edi-0x48] = Radius2.
//   Then 0xE6: DC 67 AC: disp8 = 0xAC = -0x54 signed. [edi-0x54] = RadiusSpheres.
//
// So: torus1_de = sqrt((sqrt(rx^2+ry^2) - 2*R1)^2 + rz^2) - R2
//     sphere_de = sqrt((|rx|-R1)^2 + ry^2 + rz^2) - RadSph
//
// 0xE9: fcom st(1)  -> compare sphere_de vs torus1_de
// 0xEB-0xEE: fnstsw ax, shr ah,1 -> test C0
// 0xF0: jae 0xF6 -> if sphere_de >= torus1_de, jump
// 0xF2: fstp st(1) -> keep sphere_de (it's smaller)
// 0xF4: jmp 0xF8
// 0xF6: fstp st(0) -> keep torus1_de (it's smaller)
// -> min(sphere_de, torus1_de)
//
// Part 2: Yin-Yang split based on y sign
// 0xF8: fld [esp] = rx
// 0xFB: fld [esp+8] = ry
// 0xFF: ftst -> compare ry with 0
// 0x101-0x104: fnstsw ax, shr ah,1
// 0x106: jae 0x112 -> if ry >= 0, jump to 0x112
//   0x108: fld [edi-0x10] = R1
//   0x10B: mov edx, 1
//   0x110: jmp 0x11C
// 0x112: fld [edi-0x10] = R1
//   0x115: fchs -> -R1
//   0x117: mov edx, -1
// 0x11C: fstp st(1) -> replace ry with R1 or -R1
//   -> if ry < 0: rx stays, st0 becomes R1, edx=1
//   -> if ry >= 0: rx stays, st0 becomes -R1, edx=-1
//   After fstp st(1): stack was (R1_or_neg, ry, rx, min_de)
//   fstp st(1) pops st0 into st1, so: stack = (R1_or_neg, rx, min_de)
//   Wait: before 0x11C stack is:  st0=R1 or -R1, st1=ry, st2=rx, st3=min_de
//   fstp st(1): store st0 to st1 and pop -> st0=R1_or_neg(was ry), st1=rx, st2=min_de
//   No: fstp st(1) means "store st0 into st1 location, then pop". After pop,
//   old st1 had ry, now it gets R1_or_neg, then pop removes st0.
//   Result: st0 = R1_or_neg (written to old st1 location, now top after pop), st1=rx, st2=min_de
//   Hmm actually: fstp st(1) stores st0 -> st1, then pops. Old st2 becomes new st1.
//   So: st0 = old_st1 replaced with st0 value? No.
//   fstp st(i): st(i) = st(0), then pop. After pop, indices shift down.
//   Before: st0=val, st1=ry, st2=rx, st3=min_de
//   fstp st(1): st1 = val, pop -> new st0=val(old st1 loc), st1=rx, st2=min_de
//   Actually after pop, old st2 becomes new st1, old st3 becomes new st2.
//   The written value is in old st1 position which becomes new st0.
//   So: st0 = val (R1 or -R1), st1 = rx, st2 = min_de. Yes.
//
// 0x11E: faddp st(1) -> rx + val -> if ry<0: rx+R1, if ry>=0: rx-R1
//   stack: (rx +/- R1), min_de
// 0x120: fstp [esp] -> store adjusted rx, stack: min_de
//
// Torus 2 (second half of Yin-Yang):
// 0x123: fld [esp] = adjusted_rx
// 0x126: D8 C8 = square
// 0x128: fld [esp+8] = ry
// 0x12C: D8 C8 = square
// 0x12E: faddp -> adjusted_rx^2 + ry^2
// 0x130: fsqrt -> r_xy2
// 0x132: DC 67 F0 = fsub [edi-0x10] = R1 -> r_xy2 - R1
// 0x135: D8 C8 = square -> (r_xy2 - R1)^2
// 0x137: fld [esp+0x10] = rz
// 0x13B: D8 C8 = square
// 0x13D: faddp -> (r_xy2 - R1)^2 + rz^2
// 0x13: fsqrt
// 0x141: DC 67 B8 = fsub [edi-0x48] = R2 -> torus2_de
// -> torus2_de = sqrt((sqrt(adj_rx^2 + ry^2) - R1)^2 + rz^2) - R2
//
// 0x144: fcom st(1) -> compare torus2_de with current min
// 0x146-0x149: fnstsw, shr ah,1
// 0x14B: jae 0x151
// 0x14D: fstp st(1) -> keep torus2_de (smaller)
// 0x14: jmp 0x153
// 0x151: fstp st(0) -> keep current min
//
// Scale correction and store:
// 0x153: cmp [edi-0x4c], 0
// 0x157: jne 0x15C
// 0x159: fdiv [edi-0x20] = Scale
// 0x15C: fstp [esi-0x20] -> store DE
// 0x15: add esp, 0x32
// 0x162: ret

template<>
cpu_double3 execute_formula<FORMULA_YINYANGIFS>(cpu_double3 p, constant double* params, double &de) {
    // IFS parameters
    const double Radius1 = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Radius2 = params[5];
    const int Apply_scale_add = int(params[6]);
    const double RadiusSpheres = params[7];
    // params[8..16] = 3SingleAngles rotation matrix (9 floats)
    // Layout: M0,M1,M2 (row0), M3,M4,M5 (row1), M6,M7,M8 (row2)
    const double M0 = params[8];
    const double M1 = params[9];
    const double M2 = params[10];
    const double M3 = params[11];
    const double M4 = params[12];
    const double M5 = params[13];
    const double M6 = params[14];
    const double M7 = params[15];
    const double M8 = params[16];
    // [CONSTANTS] Double = -1.0 -> params[17]
    const double neg_one = params[17];

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Apply 3x3 rotation matrix
    // ASM: rx = x*M0 + y*M1 + z*M2
    //      ry = x*M3 + y*M4 + z*M5
    //      rz = x*M6 + y*M7 + z*M8
    double rx = x * M0 + y * M1 + z * M2;
    double ry = x * M3 + y * M4 + z * M5;
    double rz = x * M6 + y * M7 + z * M8;

    // Step 3: Torus 1 (full Yin-Yang body)
    // ASM: sqrt(rx^2 + ry^2) -> r_xy, then (r_xy - 2*R1)
    // Note: ASM subtracts R1 twice: fsub [edi-0x10] twice in succession
    double r_xy = sqrt(rx * rx + ry * ry);
    double ring1 = r_xy - 2.0 * Radius1;
    double torus1_de = sqrt(ring1 * ring1 + rz * rz) - Radius2;

    // Step 4: Sphere centered at (+/-R1, 0, 0)
    // ASM: |rx| + R1*(-1) = |rx| - R1
    double sphere_de = sqrt((abs(rx) + Radius1 * neg_one) * (abs(rx) + Radius1 * neg_one) + ry * ry + rz * rz) - RadiusSpheres;

    // Step 5: min(torus1_de, sphere_de)
    // ASM: fcom + shr ah,1 + jae -> keeps smaller value
    double current_de = min(torus1_de, sphere_de);

    // Step 6: Yin-Yang offset based on y sign
    // ASM: if ry < 0: adjusted_rx = rx + R1, edx = 1
    //      if ry >= 0: adjusted_rx = rx - R1, edx = -1
    double adjusted_rx;
    if (ry < 0.0) {
        adjusted_rx = rx + Radius1;
    } else {
        adjusted_rx = rx - Radius1;
    }

    // Step 7: Torus 2 (half-torus for Yin-Yang)
    // ASM: sqrt(adj_rx^2 + ry^2) - R1, then sqrt(ring^2 + rz^2) - R2
    double r_xy2 = sqrt(adjusted_rx * adjusted_rx + ry * ry);
    double ring2 = r_xy2 - Radius1;
    double torus2_de = sqrt(ring2 * ring2 + rz * rz) - Radius2;

    // Step 8: min(current_de, torus2_de)
    double shape_de = min(current_de, torus2_de);

    // Step 9: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(x, y, z);
}

// Alternative direct function implementation
cpu_double3 YinYangIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_YINYANGIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_YinYangIFS = {
    .id = FORMULA_YINYANGIFS,
    .name = "YinYangIFS",
    .param_count = 18,
    .complexity = 520,
    .uses_transcendental = false
};

/*
Original M3 Description:
The symbol of Tao, made with toruses and spheres. Added too rotations.

Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape -- SSE2 IFS + x87 Yin-Yang DE):
    - Standard SSE2 IFS preamble
    - 3x3 rotation matrix applied to (x,y,z)
    - Torus 1: full ring at radius 2*R1 from origin, tube radius R2
    - Sphere: at |rx|=R1, radius RadiusSpheres (Yin-Yang dots)
    - Torus 2: half-ring offset by R1 based on sign of ry
    - DE = min(torus1, sphere, torus2)

Parameters:
  [0] Radius 1: double (default: 0.5) -- major radius component
  [1] Scale: double (default: 1) -- IFS scale factor (.2Doubles)
  [2] Z_add: double (default: 0) -- Z offset (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Radius 2: double (default: 0.1) -- torus tube radius
  [6] Apply_scale_add: int (default: 1)
  [7] Radius Spheres: double (default: 0.2) -- dot sphere radius
  [8..16] YinYangRot: 3SingleAngles (9 floats, rotation matrix)
  [17] Constant: -1.0

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = Radius1 (double)
  [PVar - 0x20] = Scale (.2Doubles, 16 bytes)
  [PVar - 0x30] = Z_add (.2Doubles, 16 bytes)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Radius2 (double)
  [PVar - 0x4c] = Apply_scale_add (integer)
  [PVar - 0x54] = RadiusSpheres (double)
  (.3SingleAngles rotation matrix at deeper offsets)
  [PVar + 0x00] = -1.0 (constant)

Code structure:
  0x00-0x3: Standard SSE2 IFS preamble
  0x40-0x4A: Store coords to stack
  0x4C-0x9D: 3x3 rotation matrix multiply
  0xA2-0xC3: Torus 1 DE: sqrt((sqrt(x^2+y^2)-2R1)^2 + z^2) - R2
  0xC6-0xE6: Sphere DE: sqrt((|x|-R1)^2 + y^2 + z^2) - RadSph
  0xE9-0xF6: min(torus1, sphere)
  0xF8-0x120: Yin-Yang y-sign split, adjust rx
  0x123-0x141: Torus 2 DE
  0x144-0x151: min with torus2
  0x153-0x162: Scale correction + store + return
*/
