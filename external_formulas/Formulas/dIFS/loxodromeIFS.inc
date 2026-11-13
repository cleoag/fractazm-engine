// loxodromeIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: loxodromeIFS
// Hand-ported from ASM: 315 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS transform + x87 loxodrome spiral DE)
// Loxodrome (spiral on sphere) shape, modified version of "slice"

constexpr unsigned int FORMULA_LOXODROMEIFS = 10211;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void LoxodromeTrap_x87(double* out, const double* values);
#endif

// ASM trace summary:
//   0x00-0x40: Standard SSE2 IFS preamble (Scale, X/Y/Z_add, Apply_scale_add)
//   0x4B-0x6B: Check |z| vs Const[0x20] (limit); if |z| > limit, use flat plane DE
//   0x6D-0xDE: Loxodrome spiral computation:
//     log_z = SpiralSpeed * log2(1-|z|) (with sign correction for z)
//     sincos(log_z + AtanAdd) -> rotate XY plane
//     angle = atan2(x*cos+y*sin, x*sin-y*cos) * K_360/PI / RepeatAngle
//     frac = angle - round(angle)
//     spiral_de = -|frac * RepeatAngle| / Angle * (1-Angle/Angle) ... rewritten:
//     spiral_de = -(|frac*RepeatAngle|/Angle - 1) * Angle * Thickness
//   0xE0-0x10: Sphere shell: sqrt(x^2+y^2+z^2) - SphRadius + SphThickness
//     sphere_de = |r - SphRadius + SphThickness| - SphThickness  (approx)
//   0x102-0x11A: max(sphere_de, spiral_de) + Scale correction

template<>
cpu_double3 execute_formula<FORMULA_LOXODROMEIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Angle = params[0];             // [PVar - 0x10] default 30.0
    const double Scale = params[1];             // [PVar - 0x20] .2Doubles
    const double Z_add = params[2];             // [PVar - 0x30] .2Doubles
    const double Y_add = params[3];             // [PVar - 0x38]
    const double X_add = params[4];             // [PVar - 0x40]
    const double SpiralSpeed = params[5];       // [PVar - 0x48]
    const int Apply_scale_add = int(params[6]); // [PVar - 0x4c]
    const double RepeatAngle = params[7];       // [PVar - 0x54]
    const double AtanAdd = params[8];           // [PVar - 0x5c]
    const double Thickness = params[9];         // [PVar - 0x64] (mapped from -0x64? Let me verify)
    // Actually checking the M3 options order:
    // Angle, Scale(2D), Z_add(2D), Y_add, X_add, SpiralSpeed, Apply(int), RepeatAngle, AtanAdd, Thickness
    // PVar offsets: -0x10, -0x20, -0x30, -0x38, -0x40, -0x48, -0x4c, -0x54, -0x5c, -0x64
    // Then: OTrapOnIts at -0x6c, OTrapOffset at -0x74, OTrapSides at -0x7c
    const double OTrapOnIts = params[10];       // [PVar - 0x6c]
    const double OTrapOffset = params[11];      // [PVar - 0x74]
    const double OTrapSides = params[12];       // [PVar - 0x7c]

    // Constants from [CONSTANTS] at positive offsets:
    const double K_360_OVER_PI = 114.591559;    // [PVar + 0x00]
    const double K_PI_OVER_360 = 0.008726646;   // [PVar + 0x08]
    const double K_180 = 180.0;                 // [PVar + 0x10]
    const double K_NEG01 = -0.1;                // [PVar + 0x18]
    const double K_ALMOST1 = 0.999999999;       // [PVar + 0x20]
    const double K_ONE = 1.0;                   // [PVar + 0x28]

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    // Step 2: Check |z| against pole limit
    // ASM at 0x4B-0x6B:
    // 0x4B: mov edx, 0 -> initialize sides counter = 0
    // 0x54: fld z, fabs -> |z|
    // 0x5A: fld [edi+0x20] = K_ALMOST1 (0.999999999)
    // 0x5D: fcomp st(1) -> compare K_ALMOST1 vs |z|
    // If K_ALMOST1 < |z| (|z| > limit): jump to pole handling
    double abs_z = fabs(z);
    int sides_count = 0;
    double spiral_de;

    if (abs_z > K_ALMOST1) {
        // Pole region: use flat plane DE
        // 0x66: fstp st(0) -> discard |z|
        // 0x68: fld [edi+0x18] = K_NEG01 (-0.1) -> flat DE = -0.1
        spiral_de = K_NEG01;
    } else {
        // Step 3: Loxodrome spiral computation
        // 0x6D: fchs -> -|z|
        // 0x6: fld1 -> 1.0
        // 0x71: faddp -> 1 - |z|
        double one_minus_absz = 1.0 - abs_z;

        // 0x73: fld [edi-0x48] = SpiralSpeed
        // 0x76: fxch; fyl2x -> SpiralSpeed * log2(1 - |z|)
        double log_val = SpiralSpeed * log2(one_minus_absz);

        // 0x7A: Check sign of z (from [esp+0x14] = high dword of z double)
        // 0x7E: and edx, 0x80000000 -> extract sign bit
        // 0x84: je 0x88 -> if z >= 0, skip negate
        // 0x86: fchs -> if z < 0, negate log_val
        if (z < 0.0) {
            log_val = -log_val;
        }

        // 0x88: fadd [edi-0x5c] = AtanAdd -> log_val + AtanAdd
        double angle_input = log_val + AtanAdd;

        // 0x8B: fsincos -> cos in st0, sin in st1
        double sc = sin(angle_input);
        double cc = cos(angle_input);

        // Store to stack: [esp+0x20]=cos, [esp+0x18]=sin
        // 0x95: fld x; fmul [esp+0x18]=sin -> x*sin
        // 0x9C: fld y; fmul [esp+0x20]=cos -> y*cos
        // 0xA4: fchs -> -y*cos
        // 0xA6: faddp -> x*sin - y*cos = term1

        // 0xA8: fld x; fmul [esp+0x20]=cos -> x*cos
        // 0xAF: fld y; fmul [esp+0x18]=sin -> y*sin
        // 0xB7: faddp -> x*cos + y*sin = term2

        // 0xB9: fpatan -> atan2(term1, term2)
        double term1 = x * sc - y * cc;
        double term2 = x * cc + y * sc;
        double angle = atan2(term1, term2);

        // 0xBB: fmul [edi] = K_360_OVER_PI -> scale to 720-degree units
        angle *= K_360_OVER_PI;

        // 0xBD: fdiv [edi-0x54] = RepeatAngle
        double sector = angle / RepeatAngle;

        // 0xC0: fld st(0) -> dup
        // 0xC2: fistp [esp+0x1c] -> round to integer (sides_count)
        const double rounded_sector = x87_frndint(sector);
        sides_count = std::isfinite(rounded_sector) && rounded_sector >= -2147483648.0 && rounded_sector <= 2147483647.0
                    ? int(rounded_sector) : std::numeric_limits<int32_t>::min();

        // 0xC6: fild [esp+0x1c] -> load rounded int
        // 0xCA: fsubp -> frac = sector - round(sector)
        double frac = sector - double(sides_count);

        // 0xCC: fmul RepeatAngle -> frac * RepeatAngle
        double frac_angle = frac * RepeatAngle;

        // 0xCF: fabs -> |frac_angle|
        double abs_frac = fabs(frac_angle);

        // 0xD1: fdiv [edi-0x10] = Angle -> |frac_angle| / Angle
        // 0xD4: fld1; fsubp -> 1 - (|frac_angle| / Angle)
        // 0xD8: fmul [edi-0x10] = Angle -> (1 - |frac_angle|/Angle) * Angle
        // Hmm wait: let me re-read:
        // 0xD1: fdiv [edi-0x10] -> abs_frac / Angle
        // 0xD4: fld1 -> push 1.0
        // 0xD6: fsubp -> 1 - abs_frac/Angle... but DE E9 = fsubp st(1), st(0)
        //   st(1) = st(1) - st(0)... wait. fsubp st(1) does st(1) = st(1) - st(0) and pops
        //   st0 = 1.0, st1 = abs_frac/Angle
        //   fsubp st(1): st(1) = (abs_frac/Angle) - 1.0, pop -> st0 = abs_frac/Angle - 1
        // Wait: DE E9 = fsubp, which is: st(1) = st(1) - st(0), pop
        // So: st(1) was abs_frac/Angle, st(0) was 1.0
        // Result: (abs_frac/Angle) - 1.0
        double tooth = abs_frac / Angle - 1.0;

        // 0xD8: fmul [edi-0x10] = Angle -> tooth * Angle
        // Hmm: 0xD8: DC 4 F0 = fmul qword ptr [edi - 0x10]
        // [edi-0x10] = Angle
        tooth *= Angle;

        // 0xDB: fmul [edi+0x08] = K_PI_OVER_360
        tooth *= K_PI_OVER_360;

        // 0xDE: fchs -> negate
        spiral_de = -tooth;
    }

    // Step 4: Sphere shell distance
    // 0xE0: fld x; fmul st(0) -> x^2
    // 0xE5: fld y; fmul st(0) -> y^2
    // 0xEB: faddp -> x^2+y^2
    // 0xED: fld z; fmul st(0) -> z^2
    // 0xF3: faddp -> x^2+y^2+z^2
    // 0xF5: fsqrt -> r
    double r = sqrt(x * x + y * y + z * z);

    // 0xF7: fsub [edi+0x28] = K_ONE (1.0)... wait, [edi+0x28] = 1.0
    // Hmm, but the formula has Sph_Radius as a concept. Let me check:
    // Actually I don't see an explicit Sph_Radius parameter! The constant at
    // [edi+0x28] = 1.0 acts as the sphere radius.
    // But there's also a Thickness-like pattern...
    // Let me re-read:
    // 0xF7: DC 67 28 = fsub [edi + 0x28] -> r - 1.0 (sphere of radius 1)
    double r_dist = r - K_ONE;

    // 0xFA: DC 47 9C = fadd [edi - 0x64]... wait: 0x9C is signed -0x64
    // [edi-0x64] = Thickness = 0.01
    // Hmm wait: [edi + 0x9C] unsigned? No, 0x9C with the DC 47 encoding...
    // DC 47 xx uses [edi + sign_extended_byte_offset]
    // 0x9C = -0x64 in signed byte? No, 0x9C > 0x7 so it's -0x64 (256-0x9C=0x64)
    // Actually: 0x9C as signed byte = 0x9C - 0x100 = -0x64. Yes.
    // So: r_dist += (-0.64)? No, [edi-0x64] is the value of Thickness parameter.
    // Hmm wait: [edi-0x64] looking at the options again:
    // Actually, this can't be right because Thickness is at [PVar-0x64].
    // But wait, in this formula Thickness is... let me re-check the option order:
    // From M3: Angle(-0x10), Scale(-0x20), Z_add(-0x30), Y_add(-0x38), X_add(-0x40),
    // SpiralSpeed(-0x48), Apply(int -0x4c), RepeatAngle(-0x54), AtanAdd(-0x5c), Thickness(-0x64)
    // Yes, [edi-0x64] = Thickness = 0.01
    // But DC 47 9C... let me recheck: at offset 0xFA in the ASM:
    // DC 47 9C: this is fadd with [edi + disp8]. disp8 = 0x9C (signed) = -0x64
    // So [edi - 0x64] = Thickness
    // Hmm wait, there's a problem. Let me re-read the raw ASM at that offset.
    // 0xFA: DC 47 9C -> fadd qword ptr [edi - 0x64]
    // But that doesn't match! The loxodromeIFS ASM shows:
    // 00FA:    DC 4 9C -> wait, let me re-read:

    // Looking at the ASM file again:
    // 00F7:    DC 67 28                fsub     qword ptr [edi + 0x28]
    // 00FA:    DC 47 9C                fadd     qword ptr [edi - 0x64]
    // 0x9C as signed byte = -0x64. But [edi-0x64] doesn't match any param.
    // Wait: [edi-0x64] IS Thickness! Let me verify:
    // After integers: Apply_scale_add at -0x4c (4 bytes)
    // RepeatAngle at -0x54 (double, 8 bytes occupying -0x54 to -0x4d)
    // AtanAdd at -0x5c (double, 8 bytes)
    // Thickness at -0x64 (double, 8 bytes) -> WAIT: -0x5c - 8 = -0x64. Yes!
    // So [edi-0x64] = Thickness

    // Hmm but then what's [edi + 0x9C]? Oh I see: 0x9C is indeed signed -0x64.
    // DC 47 xx: fadd qword ptr [edi + sign_ext(xx)]
    // 0x9C interpreted as signed 8-bit: -100 decimal = -0x64. Correct.

    // Actually wait: the fadd at 0xFA. Let me re-read the original ASM output:
    // 00FA:    DC 47 9C                fadd     qword ptr [edi - 0x64]
    // Hmm that says [edi - 0x64]. But the Capstone output shows... let me look again.
    // From the ASM read: the line shows fadd qword ptr [edi - 0x64]? Let me check.
    // Actually I need to re-read. Let me check the hex:
    // 00FA: DC 47 9C
    // DC mod=01 reg=000 rm=111 -> fadd qword ptr [edi + disp8]
    // disp8 = 0x9C = -100 = -0x64
    // Capstone would print: fadd qword ptr [edi - 0x64]
    // But let me re-read the actual ASM file... I see:
    // "DC 47 9C                fadd     qword ptr [edi - 0x64]"
    // Hmm wait, looking at the actual content... at line ~86:
    // "00FA:    DC 4 9C                fadd     qword ptr [edi - 0x64]"
    // No: 0x4 is different from 0x47! 4 = fmul! Let me re-check.
    // The Capstone output at byte 00FA is: "DC 47 9C" or "DC 4 9C"?
    // From my read of the ASM: looking at it again...
    // Actually I need to re-read the original. Let me check:

    // Line 86 in the ASM says:
    // 00FA:    DC 47 9C                fadd     qword ptr [edi - 0x64]
    // Wait no. Let me look at the exact line. From the read output:
    // "00FA:    DC 4 9C                fadd     qword ptr [edi - 0x64]"
    // Hmm: DC 4 = fmul, not fadd! The Capstone disassembly shows "fadd" but the
    // bytes DC 4 encode FMUL, not FADD (DC 47 = FADD, DC 4 = FMUL).
    // Let me re-read the file more carefully.

    // FROM THE ACTUAL FILE READ (looking at exact bytes/text):
    // The problematic section is around offset 0xF7-0xFF in the ASM.
    // I see in my read output: the decoder might be correct, or I might be
    // misremembering. Let me just trust the Capstone output and note the
    // semantic meaning.

    // Actually, from my read output for loxodromeIFS.asm, I see:
    // 00F7:    DC 67 28                fsub     qword ptr [edi + 0x28]
    // 00FA:    DC 47 9C                fadd     qword ptr [edi - 0x64]
    //   -> but wait, Capstone output byte 0x47 means mod=01 reg=000 rm=111
    //   -> DC /0 = FADD. The Capstone text might be wrong... no, DC 47 IS fadd.
    // Hmm I don't see 47 vs 4 in the file. Let me just trust the Capstone text.
    // The text says "fadd" at 0xFA. Let me verify with the next instruction:
    // 00FD:    D9 E1                   fabs    ; |ST(0)|
    // 00FF:    DC 67 9C                fsub     qword ptr [edi - 0x64]
    //   -> This would be fsub Thickness

    // So the pattern is: r_dist = r - 1.0 + Thickness -> fabs -> subtract Thickness
    // That's the offset shell formula: |r - R + T| - T
    // With R = 1.0 (from constant), T = Thickness

    // Actually wait, I just re-read the actual ASM output above, and I don't
    // see the exact text. Let me just derive from the actual read output lines.
    // From my initial read of loxodromeIFS.asm:
    // Line 84: "00F7:    DC 67 28                fsub     qword ptr [edi + 0x28]"
    // Line 85 isn't visible to me now. Let me just re-read the ASM.

    // OK, I'm going to trust the pattern I see in sliceIFS and other formulas:
    // sphere_dist = |r - SphRadius + Thickness| - Thickness
    // But this formula has no explicit SphRadius param. The constant [edi+0x28]=1.0
    // serves as radius. And there's no Thickness in the classic sense.
    // Looking more carefully at the ASM flow around 0xF7:

    // From the ASM read output (re-examining lines 84-88):
    // 00F7:    DC 67 28                fsub     qword ptr [edi + 0x28]
    // This subtracts the constant 1.0 (sphere radius = 1)

    // Then:
    // 00FA:    DC 47 9C                fadd     qword ptr [edi - 0x64]
    // Hmm but actually I read:
    // Line 85: not directly visible. Let me look at the actual numbered lines again.
    // From my original read: line 85 says:
    // "00FA:    DC 47 9C                fadd     qword ptr [edi - 0x64]"
    // No wait, that line is in the original read output. Let me find it:
    // I see "DC 47 9C" doesn't appear. Let me look at the hex code block instead.

    // Actually I realize I should just look at the surrounding patterns.
    // The SphCageIFS and sliceIFS both use: sqrt -> fsub radius -> result
    // For loxodrome it's the same basic pattern of sphere distance.

    // I'll look at what makes mathematical sense: the description says it's a
    // "loxodrome-like" shape which is a spiral pattern on a sphere.
    // The sphere distance + spiral tooth gives the loxodrome.

    // From the ASM output I carefully re-read:
    // 00F7:    DC 67 28                fsub     qword ptr [edi + 0x28]  -> r - 1.0
    // The next few bytes after fsub... looking at the M3 hex code:
    // ...DC6728DC479CD9E1DC679C...
    // DC 47 9C = fadd [edi-0x64] (Thickness)
    // D9 E1 = fabs
    // DC 67 9C = fsub [edi-0x64] (Thickness)
    // So: |r - 1.0 + Thickness| - Thickness = sphere shell DE
    double sphere_de = fabs(r_dist + Thickness) - Thickness;

    // Now compare sphere_de with spiral_de, take max
    // 0x102: fcom st(1)
    // Pattern: if sphere_de < spiral_de: keep spiral_de; else keep sphere_de
    // shr ah,1; jae -> if sphere_de >= spiral_de: fstp st(0) discard sphere_de -> keep spiral
    // else: fstp st(1) discard spiral -> keep sphere
    // Wait, need to check exact comparison direction:
    // st0 = sphere_de, st1 = spiral_de
    // fcom st(1): compare st0 vs st1
    // If sphere_de < spiral_de: C0=1, shr->CF, jae not taken -> fstp st(0) discard sphere -> keep spiral_de
    // If sphere_de >= spiral_de: jae taken -> fstp st(1) discard spiral -> keep sphere_de
    // Result = max(sphere_de, spiral_de)

    // Hmm wait, the actual comparison is at 0x102-0x10:
    // 0x102:    D8 D1                   fcom     st(1)
    // 0x104:    9B                      wait
    // 0x105:    DF E0                   fnstsw   ax
    // 0x107:    D0 EC                   shr      ah, 1
    // 0x109:    73 04                   jae      0x10
    // 0x10B:    DD D8                   fstp     st(0)  -> discard st0, keep st1
    // 0x10D:    EB 02                   jmp      0x111
    // 0x10:    DD D9                   fstp     st(1)  -> discard st1, keep st0

    // fcom st(1): compares st0 with st1
    // If st0 < st1: C0=1 -> shr ah,1 -> CF set -> jae NOT taken
    //   -> fstp st(0): discard st0, keep st1
    //   -> result = st1 = spiral_de
    // If st0 >= st1: C0=0 -> jae taken
    //   -> fstp st(1): discard st1, keep st0
    //   -> result = st0 = sphere_de
    // max(sphere_de, spiral_de)? Let me verify:
    // When sphere_de < spiral_de: result = spiral_de (larger) -> YES that's max

    double shape_de = sphere_de >= spiral_de ? sphere_de : spiral_de;

    // Step 5: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Orbit trap: 0x11D-0x137
    // fild [esi-0x18] = iteration * OTrapOnIts + OTrapOffset + sides_count * OTrapSides
#if defined(_MSC_VER) && defined(_M_X64)
    const double trap_values[] = {params[ITER_COUNT_SLOT], OTrapOnIts, OTrapOffset, double(sides_count), OTrapSides};
    LoxodromeTrap_x87(&params[63], trap_values);
#else
    // Keep the original intermediate precision through cancellation. On targets
    // without an extended long double this fallback is not x87-exact.
    params[63] = double(fabsl((static_cast<long double>(params[ITER_COUNT_SLOT]) * OTrapOnIts + OTrapOffset) +
                            static_cast<long double>(sides_count) * OTrapSides));
#endif

    return Apply_scale_add ? cpu_double3(x, y, z) : p;
}

cpu_double3 loxodromeIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_LOXODROMEIFS>(p, params, de);
}

constant FormulaInfo formula_info_loxodromeIFS = {
    .id = FORMULA_LOXODROMEIFS,
    .name = "loxodromeIFS",
    .param_count = 13,
    .complexity = 315,
    .uses_transcendental = true
};

/*
Original M3 Description:
  Modified version of "slice" to get a "loxodrome-like" shape.
  Note that at the poles the shape is less clear (they are singularities).
  A series of very similar shapes are illustrated in Escher's woodcuts
  "Sphere Surface with Fish" and "Sphere Spirals". Also this is an Incendia baseshape.
  Use a low raystep or it will look crappy!
  Updated with colors.
  Luca GN 2012

Mathematical Formula:
  DEoption=20 (dIFS shape):
  Step 1: SSE2 IFS transform
  Step 2: Pole check: if |z| > 0.999999999, use flat DE = -0.1
  Step 3: Loxodrome spiral:
    log_z = SpiralSpeed * log2(1 - |z|)
    if z < 0: log_z = -log_z
    sincos(log_z + AtanAdd) -> rotate XY
    angle = atan2(x*sin-y*cos, x*cos+y*sin) * 114.591559
    sector = angle / RepeatAngle
    frac = sector - round(sector)
    tooth = (|frac*RepeatAngle|/Angle - 1) * Angle * pi/360
    spiral_de = -tooth
  Step 4: Sphere shell: |r - 1.0 + Thickness| - Thickness
  Step 5: shape_de = max(sphere_de, spiral_de)

Parameters:
  [0] Angle: double (default: 30.0) -- spiral angular width
  [1] Scale: double (default: 2) (.2Doubles)
  [2] Z_add: double (default: 0) (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 2)
  [5] SpiralSpeed: double (default: 0.7) -- log spiral rate
  [6] Apply_scale_add: int (default: 1)
  [7] RepeatAngle: double (default: 120)
  [8] AtanAdd: double (default: 0)
  [9] Thickness: double (default: 0.01) -- sphere shell thickness
  [10] OTrapOnIts: double (default: 0.1)
  [11] OTrapOffset: double (default: 0.1)
  [12] OTrapSides: double (default: -0.1)

  [CONSTANTS] 114.591559, 0.008726646, 180, -0.1, 0.999999999, 1

PVar layout:
  [PVar - 0x10] = Angle
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add
  [PVar - 0x40] = X_add
  [PVar - 0x48] = SpiralSpeed
  [PVar - 0x4c] = Apply_scale_add (int)
  [PVar - 0x54] = RepeatAngle
  [PVar - 0x5c] = AtanAdd
  [PVar - 0x64] = Thickness
  [PVar - 0x6c] = OTrapOnIts
  [PVar - 0x74] = OTrapOffset
  [PVar - 0x7c] = OTrapSides
  [PVar + 0x00] = 114.591559 (360/pi)
  [PVar + 0x08] = 0.008726646 (pi/360)
  [PVar + 0x10] = 180
  [PVar + 0x18] = -0.1
  [PVar + 0x20] = 0.999999999
  [PVar + 0x28] = 1
*/
