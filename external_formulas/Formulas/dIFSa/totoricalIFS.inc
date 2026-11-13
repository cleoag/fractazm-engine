// totoricalIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: totoricalIFS
// Hand-ported from ASM: 248 bytes, DEoption=21
// Toroidal (generalized) coordinates for DIFS
// Formula by Luca GN 2011

constexpr unsigned int FORMULA_TOTORICALIFS = 10313;

// ASM trace summary:
// dIFS transform (DEoption=21): no SSE2 preamble
// Coordinates: esi-0x78=x, esi-0x70=y, esi-0x68=z (dIFS convention)
//
// Step 1: Compute toroidal radius: rxy = sqrt(y^2 + z^2) (note: z at esi-0x68 loaded first)
//         Wait — ASM loads [esi-0x68] and [esi-0x70]:
//         [esi-0x68]=z: z^2, [esi-0x70]=y: y^2, sum, sqrt => rxy = sqrt(y^2 + z^2)
//         Then subtract R: d = rxy - R
//
// Step 2: Compute second radius: r2 = sqrt(d^2 + x^2) where x is [esi-0x78]
//
// Step 3: Compute angles:
//         angle1 = atan2(d, -x)  => atan2(d, -x) — note [esi-0x78] is negated
//         angle1 = |angle1| - subA1  (after optional multiplication by mulA1)
//         angle2 = |atan2(y, z)| (standard atan2 of y,z)
//
// Step 4: If FixATo1 == 0: apply foldinginfy to angles
//         else (FixATo1 != 0): scale angles by [edi+0x08] = FoldAngles constant
//
// Step 5: Multiply by mulA1/mulA2 and subtract subA1/subRadius
//
// Step 6: Apply foldinginfy to angle1 and angle2 (if nonzero fold param)
//
// Parameters (negative offsets from PVar):
//   [edi-0x10] = R (major radius, default 2.0)
//   [edi-0x18] = mulA1 (default 8)
//   [edi-0x20] = subA1 (default 0.0)
//   [edi-0x28] = mulA2 (default 4)
//   [edi-0x30] = subA2 (default 0.0) — note: .m3 has duplicate "subA1"
//   [edi-0x38] = mulRadius (default -1.0)
//   [edi-0x40] = subRadius (default -1.0)
//   [edi-0x44] = FixATo1 (integer, default 1)
//   [edi-0x4c] = FoldAngles (default 0.5)
// Constants:
//   [edi+0x00] = pi/2 = 1.5707963...
//   [edi+0x08] = 1/pi = 0.318309886...

template<>
cpu_double3 execute_formula<FORMULA_TOTORICALIFS>(cpu_double3 p, constant double* params, double &de) {
    const double R         = params[0];
    const double mulA1     = params[1];
    const double subA1     = params[2];
    const double mulA2     = params[3];
    const double subA2     = params[4];
    const double mulRadius = params[5];
    const double subRadius = params[6];
    const int   FixATo1   = int(params[7]);
    const double FoldAngles= params[8];

    // Constants
    const double c_halfpi  = 1.5707963267948966; // [PVar+0x00] = pi/2
    const double c_invpi   = 0.318309886183790671; // [PVar+0x08] = 1/pi

    double x = p.x;  // [esi-0x78]
    double y = p.y;  // [esi-0x70]
    double z = p.z;  // [esi-0x68]

    // Step 1: rxy = sqrt(z^2 + y^2) — ASM: [esi-0x68]^2 + [esi-0x70]^2
    double rxy = sqrt(z * z + y * y);

    // Subtract major radius R
    double d = rxy - R;

    // Step 2: r2 = sqrt(d^2 + x^2)
    double r2 = sqrt(d * d + x * x);

    // Step 3: Angles
    // angle1 = atan2(d, -x) — ASM: fld st(1)=d, fld [esi-0x78], fchs=-x, fpatan
    double angle1 = atan2(d, -x);
    // |angle1| then subtract [edi+0x00]=subA1_offset... no wait:
    // ASM: fsub [edi] which is [edi+0x00] = pi/2 constant
    // So: angle1 = |atan2(d, -x)| - pi/2... but wait, let me re-read:
    // DC 27 = fsub qword ptr [edi] => angle1 = |angle1| - [edi+0x00]
    // [edi+0x00] is the first constant = pi/2
    // But then it checks FixATo1...
    // Actually: angle1 = |atan2(d,-x)| (at 0x27, fabs at 0x2B after fsub... no:
    // 0x25: fchs => negate the atan2 result (wait, there's no fchs here)
    // Let me re-read ASM 0x20-0x37:
    // 0x20: fld st(1) = d (st0=d, st1=old_d_or_something)
    // 0x22: fld [esi-0x78] = x
    // 0x25: fchs => -x
    // 0x27: fpatan => atan2(st1=d, st0=-x) = atan2(d, -x), result replaces both
    // 0x29: fsub [edi+0x00] => angle1 = atan2(d,-x) - pi/2
    // Wait: DC 27 = fsub [edi] which is [edi+0x00]. So:
    // angle1_raw = atan2(d, -x)
    // Then: DC 27 = fsub qword ptr [edi] => subtract [edi+0x00] = subA1 param?
    // No, [edi+0x00] = first CONSTANT = pi/2 = 1.5707963

    // Hmm wait. Let me look at the offset. At the start, edi = PVar.
    // [edi+0x00] is the first constant from [CONSTANTS] section.
    // Constants: Double = 1.5707963... (pi/2), Double = 0.318309886... (1/pi)
    // But [edi] is also the PVar base. For negative offsets, those are params.
    // For positive offsets, those are constants.
    // So [edi+0x00] = PVar[0] = first constant in the binary = pi/2? No...
    // Actually I need to reconsider. Looking at other formulas, [edi] = PVar,
    // and [edi+0x00] is the first constant value = 1st entry in [CONSTANTS].

    // Actually wait, the .m3 [CONSTANTS] says:
    //   Double = 1.5707963...  (pi/2)
    //   Double = 0.318309886... (1/pi)
    // These go at [PVar+0x00] and [PVar+0x08]

    // But the actual usage: DC 27 = fsub [edi] at 0x29.
    // [edi] = [PVar+0x00] = pi/2
    // OK so: angle1 = atan2(d, -x) - pi/2... but then:
    // 0x2B: fabs => |angle1 - pi/2|
    // Wait, 0x2B is D9 E1 = fabs.

    // Actually the ASM at 0x27 is D9 F3 = fpatan.
    // Then 0x29: DC 27 = fsub [edi] = subtract first constant = pi/2... no wait.
    // edi was set via esi+0x30 at the start. Let me check if totoricalIFS has
    // different convention since it's DEoption=21.

    // For DEoption=21, looking at the m3: there's no .SSE2 flag. The params are:
    // R, mulA1, subA1, mulA2, subA2(labeled subA1 again), mulRadius, subRadius,
    // FixATo1, FoldAngles
    // With .3SingleAngles = 0 which doesn't appear in the options list... wait
    // the m3 doesn't have .3SingleAngles. Let me re-read:
    // .Double R = 2.0          [edi-0x10]
    // .Double mulA1 = 8        [edi-0x18]
    // .Double subA1 = 0.0      [edi-0x20]
    // .Double mulA2 = 4        [edi-0x28]
    // .Double subA1 = 0.0      [edi-0x30]  (duplicate name, 2nd sub)
    // .Double mulRadius = -1.0 [edi-0x38]
    // .Double subRadius = -1.0 [edi-0x40]
    // .Integer FixATo1 = 1     [edi-0x44] (4 bytes)
    // .Double FoldAngles = .5  [edi-0x4c]
    // Gap: the integer at -0x44 takes 4 bytes, so FoldAngles is at -0x44-4-8 = -0x50?
    // Actually integers are typically stored in 4 bytes but aligned to their slot.
    // Let me look at actual offsets used in ASM:
    //   [edi-0x10] = R
    //   [edi-0x18] = mulA1
    //   [edi-0x20] = subA1
    //   [edi-0x28] = mulA2
    //   [edi-0x30] = subA2
    //   [edi-0x38] = mulRadius
    //   [edi-0x40] = subRadius
    //   [edi-0x44] (cmp dword) = FixATo1
    //   [edi-0x4c] (fld qword) = FoldAngles (note: 0x44+4+4pad = 0x4C)
    //   Actually integer is at -0x44, takes 4 bytes, pad to 8 byte boundary:
    //   -0x44 to -0x40 is the integer slot, but we already have subRadius at -0x40
    //   Hmm. Looking at layout:
    //   -0x10: R (8 bytes) => -0x10 to -0x08
    //   -0x18: mulA1 (8 bytes) => -0x18 to -0x10
    //   -0x20: subA1 (8 bytes) => -0x20 to -0x18
    //   -0x28: mulA2 (8 bytes) => -0x28 to -0x20
    //   -0x30: subA2 (8 bytes) => -0x30 to -0x28
    //   -0x38: mulRadius (8 bytes) => -0x38 to -0x30
    //   -0x40: subRadius (8 bytes) => -0x40 to -0x38
    //   -0x44: FixATo1 (4 bytes integer) => -0x44 to -0x40
    //   Then FoldAngles (8 bytes) => would be at -0x4C
    //   ASM confirms: [edi-0x4c] is used as fld qword = FoldAngles. But wait,
    //   -0x44-8 = -0x4C. So FixATo1 occupies -0x44 to -0x40, and FoldAngles is at -0x4C.

    // Let me also check what [edi+0x00] and [edi+0x08] are:
    // Constants: 1.5707963 (pi/2) and 0.318309886 (1/pi)
    // [edi+0x00] = pi/2 = c_halfpi
    // [edi+0x08] = 1/pi = c_invpi

    // OK now let me trace the actual angle computation carefully.
    // The branch at 0x37 (cmp [edi-0x44], 0; jne 0xEA) splits behavior:

    // === FixATo1 == 0 path (0x41 onwards): ===
    // 0x41: fmul [edi-0x18] => angle2 * mulA1 ... wait, what's on the stack?
    // Let me trace from the beginning:
    // 0x00: fld z(esi-0x68), fmul st(0) => z^2
    // 0x05: fld y(esi-0x70), fmul st(0) => y^2
    // st: y^2, z^2
    // faddp => y^2+z^2
    // fsqrt => rxy = sqrt(y^2+z^2)
    // fsub [edi-0x10] => d = rxy - R
    // 0x12: fld st(0) => dup d
    // fmul st(0) => d^2
    // fld [esi-0x78] => x
    // fmul st(0) => x^2
    // faddp => d^2 + x^2
    // fsqrt => r2 = sqrt(d^2+x^2)
    // Stack: st0=r2, st1=d
    // 0x20: fld st(1) => d
    // 0x22: fld [esi-0x78] => x
    // 0x25: fchs => -x
    // 0x27: fpatan => atan2(d, -x) (st1/st0 convention)
    // Actually: fpatan computes atan2(st1, st0) and pops st0
    // st1=d, st0=-x => result = atan2(d, -x), stored in new st0
    // Stack after: st0=angle1, st1=r2, st2=d ... wait, d was st1 before fld st(1)
    // Before fld st(1): st0=r2, st1=d
    // fld st(1): st0=d, st1=r2, st2=d
    // fld [esi-0x78]: st0=x, st1=d, st2=r2, st3=d
    // fchs: st0=-x
    // fpatan: atan2(st1=d, st0=-x) => pops both, pushes result
    // st0=angle1, st1=r2, st2=d

    // 0x29: fsub [edi+0x00] => angle1 - pi/2... wait that's DC 27 = fsub [edi]
    // So [edi] = first constant at PVar+0x00 = pi/2
    // Actually, looking at the raw hex: DC 27 — this is fsub qword ptr [edi].
    // But edi = PVar pointer, and [PVar+0x00] = first constant = pi/2 = 1.5707963
    // Hmm wait: the m3 shows [CONSTANTS] has two doubles:
    //   Double = 1.5707963... (at PVar+0x00)
    //   Double = 0.318309886... (at PVar+0x08)

    // So: angle1 = atan2(d, -x) - pi/2
    // But actually fsub subtracts FROM st0: st0 = st0 - [edi] = angle1 - pi/2
    // Wait, DC 27 is fsub [edi]. Let me check: DC /4 = fsub m64
    // DC 27: modrm = 0x27 = mod=00, reg=100, r/m=111 = [edi]
    // reg=4 means fsub. So st0 = st0 - [edi] = angle1 - pi/2.
    // Hmm actually DC prefix: DC /4 = FSUB m64 => st0 -= m64

    // 0x2B: fabs => |angle1 - pi/2|
    // This transforms atan2 range to a distance from pi/2

    // 0x2D-0x33: Now compute angle2 = atan2(y, z)
    // fld [esi-0x70] = y
    // fld [esi-0x68] = z
    // fpatan => atan2(y, z)
    // fabs => |atan2(y, z)|
    // Stack: st0=|angle2|, st1=|angle1-pi/2|, st2=r2, st3=d

    // 0x37: cmp [edi-0x44], 0 => check FixATo1
    // If FixATo1 != 0, jump to 0xEA (the else path)

    // FixATo1 == 0 path (starting at 0x41):
    // 0x41: fmul [edi-0x18] => |angle2| * mulA1
    // 0x44: fsub [edi-0x20] => |angle2| * mulA1 - subA1
    // Stack: st0=angle2_scaled, st1=|angle1-pi/2|, st2=r2, st3=d

    // 0x48-0x8C: foldinginfy on angle2 (using FoldAngles at edi-0x4c)
    // Then store result to [esi-0x78] (x output)

    // Then: angle1 stuff at 0x8:
    // 0x8: fmul [edi-0x28] => |angle1-pi/2| * mulA2
    // 0x92: fsub [edi-0x30] => |angle1-pi/2| * mulA2 - subA2
    // 0x96-0xDA: foldinginfy on angle1
    // Store to [esi-0x70] (y output)

    // 0xDD: r2 * mulRadius
    // 0xE0: - subRadius => r2 * mulRadius - subRadius
    // 0xE3: fstp [esi-0x68] (z output)
    // 0xE6: fstp st(0) — discard d

    // FixATo1 != 0 path (0xEA):
    // 0xEA: fld [edi+0x08] = 1/pi
    // 0xED: fmul st(1), st(0) => angle2 *= 1/pi
    // 0xEF: fmul st(2), st(0) => angle1 *= 1/pi
    // 0xF1: fstp st(0) => discard 1/pi
    // 0xF3: jmp 0x41 => continue with the scaling

    // Summary:
    // FixATo1 != 0: angles are divided by pi before scaling
    // FixATo1 == 0: angles used as-is

    // Foldinginfy on each angle (same pattern as foldinginfyIFS):
    //   period = 4 * FoldAngles
    //   v = -(v - round(v/period) * period)
    //   result = v + |FoldAngles - v| - |v + FoldAngles|

    double angle1_raw = atan2(d, -x);
    double angle1_proc = fabs(angle1_raw - c_halfpi);

    double angle2_raw = atan2(y, z);
    double angle2_proc = fabs(angle2_raw);

    // If FixATo1, normalize angles by 1/pi
    if (FixATo1 != 0) {
        angle2_proc *= c_invpi;
        angle1_proc *= c_invpi;
    }

    // Scale angle2
    double a2_scaled = angle2_proc * mulA1 - subA1;

    // foldinginfy on a2_scaled
    if (FoldAngles != 0.0) {
        double period = 4.0 * FoldAngles;
        double reduced = a2_scaled - round(a2_scaled / period) * period;
        reduced = -reduced;
        a2_scaled = reduced + fabs(FoldAngles - reduced) - fabs(reduced + FoldAngles);
    }

    // Scale angle1
    double a1_scaled = angle1_proc * mulA2 - subA2;

    // foldinginfy on a1_scaled
    if (FoldAngles != 0.0) {
        double period = 4.0 * FoldAngles;
        double reduced = a1_scaled - round(a1_scaled / period) * period;
        reduced = -reduced;
        a1_scaled = reduced + fabs(FoldAngles - reduced) - fabs(reduced + FoldAngles);
    }

    // Radius output
    double r_out = r2 * mulRadius - subRadius;

    // Output: x=a2_scaled, y=a1_scaled, z=r_out
    // ASM: a2 -> [esi-0x78]=x, a1 -> [esi-0x70]=y, r -> [esi-0x68]=z
    return cpu_double3(a2_scaled, a1_scaled, r_out);
}

// Alternative direct function
cpu_double3 totoricalIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TOTORICALIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_totoricalIFS = {
    .id = FORMULA_TOTORICALIFS,
    .name = "totoricalIFS",
    .param_count = 9,
    .complexity = 248,
    .uses_transcendental = true
};

/*
Original M3 Description:
Torical (generalized) coords for DIFS. Useful as pretransform!
This coordinate system was defined by BradC in FractalForums.
Included an useful foldinginfy to angles, makes easy to wrap shapes on the torus.
Formula by Luca GN 2011

Mathematical Formula (DEoption=21, dIFS transform):
  Step 1: Compute toroidal coordinates
    rxy = sqrt(y^2 + z^2)
    d = rxy - R (distance from major radius circle)
    r2 = sqrt(d^2 + x^2) (distance from torus surface)

  Step 2: Compute angles
    angle1 = |atan2(d, -x) - pi/2| (poloidal angle, offset by pi/2)
    angle2 = |atan2(y, z)| (toroidal angle)

  Step 3: If FixATo1 != 0, normalize angles by 1/pi

  Step 4: Scale and offset angles
    a2 = angle2 * mulA1 - subA1
    a1 = angle1 * mulA2 - subA2

  Step 5: Apply foldinginfy to each angle (if FoldAngles != 0)

  Step 6: Compute radius output
    r_out = r2 * mulRadius - subRadius

  Output: x=a2, y=a1, z=r_out

Parameters:
  [0] R: double (default: 2.0) — major torus radius
  [1] mulA1: double (default: 8)
  [2] subA1: double (default: 0.0)
  [3] mulA2: double (default: 4)
  [4] subA2: double (default: 0.0)
  [5] mulRadius: double (default: -1.0)
  [6] subRadius: double (default: -1.0)
  [7] FixATo1: integer (default: 1) — normalize angles by 1/pi
  [8] FoldAngles: double (default: 0.5) — foldinginfy period control

Constants:
  [PVar+0x00] = pi/2 = 1.5707963...
  [PVar+0x08] = 1/pi = 0.318309886...
*/
