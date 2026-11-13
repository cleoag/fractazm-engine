// Apollo3D-IFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Apollo3D-IFS
// Hand-ported from ASM: 304 bytes, DEoption=21
// 3D Apollonian fractal with polyfolding and volcano fixer
// Formula by Luca GN 2012, DE fixed by Jesse

constexpr unsigned int FORMULA_APOLLO3DIFS = 10316;

// ASM trace summary:
// dIFS transform (DEoption=21): direct x87 on dIFS coords
// Coordinates: esi-0x78=x, esi-0x70=y, esi-0x68=z
//
// This extends Apollo2D to 3D using:
// 1. Scale input XY by [edi-0x50] = Global XY-scale
// 2. Polyfold in XY plane using PolyOrder
// 3. Apply 3x3 rotation matrix (with AddX2 offset)
// 4. 3D Apollonian inversion: s2=x^2+y^2, r2=s2+z^2, Mobius in XY + z coupling
// 5. Volcano fixer: DE correction using sqrt(s2^2 + r2^2 + volcano_param)
//
// Parameters:
//   [edi-0x10] = Add X 1 (default 0)
//   [edi-0x18] = Add X 2 (default 0)
//   [edi-0x20] = Mul Z (default 0.612) -- note: named differently from .m3
//   [edi-0x24..edi-0x44] = Rotation matrix (9 floats from .3SingleAngles)
//   [edi-0x48] = PolyFold Order (integer, default 3)
//   [edi-0x50] = Global XY-scale (double, default 0.081)
//   [edi-0x58] = Final add X (double, default 0.5718)
//   [edi-0x60] = Volcano fixer (double, default 0.291)
//
// Constants:
//   [edi+0x00] = 0.159154943 (1/(2*pi))
//   [edi+0x08] = 6.283185307 (2*pi)
//   [edi+0x10] = 1.57079632675 (pi/2)
//   [edi+0x18] = 0.017453293 (pi/180, deg to rad)

template<>
cpu_double3 execute_formula<FORMULA_APOLLO3DIFS>(cpu_double3 p, constant double* params, double &de) {
    const double AddX1     = params[0];
    const double AddX2     = params[1];
    const double MulZ      = params[2];
    // params[3..5] = 3 Euler angles (degrees) for rotation matrix
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const int   PolyOrder = int(params[12]);
    const double GScale    = params[13];
    const double FinalAddX = params[14];
    const double Volcano   = params[15];

    const double c_inv2pi  = 0.159154943;
    const double c_2pi     = 6.283185307;

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // --- Step 1: Scale and Polyfold ---
    // ASM 0x03-0x51: Similar to Apollo2D but with GScale applied differently
    // 0x03: fld [esi-0x70] = y
    // 0x06: fmul [edi-0x50] = y * GScale
    // 0x09: fld [esi-0x78] = x
    // 0x0C: fmul [edi-0x50] = x * GScale
    // 0x0: fadd [edi-0x10] = x*GScale + AddX1
    // Polyfold same as Apollo2D:

    double sx = x * GScale + AddX1;
    double sy = y * GScale;

    double angle = -atan2(sy, sx);
    double angle_norm = angle * c_inv2pi;

    // ASM: fimul [edi-0x48] => multiply by integer PolyOrder
    double sector_f = round(angle_norm * double(PolyOrder));
    int sector_i = int(sector_f);

    // ASM: fidiv [edi-0x48] => divide by PolyOrder
    double sector_angle = (sector_f / double(PolyOrder)) * c_2pi;

    double cs = cos(sector_angle);
    double sn = sin(sector_angle);

    // Rotate point by sector angle
    double rot1 = sy * cs + sx * sn;
    if ((sector_i & 1) == 0) {
        rot1 = -rot1;
    }
    double rot2 = sx * cs - sy * sn;

    // ASM: fstp [esi-0x70] stores rot2*something to y slot
    // Store polyfolded result to y slot, negated to x slot

    // --- Step 2: Rotation ---
    // Load z from [esi-0x68], add AddX2 to rot2
    // Apply 3x3 rotation matrix

    double a = rot2 + AddX2;
    double b = rot1;
    double c_val = z;

    // Apply 3x3 rotation matrix (built from 3 Euler angles)
    cpu_double3 rv = R * cpu_double3(a, b, c_val);
    double ox = rv.x;
    double oy = rv.y;
    double oz = rv.z;

    // Post-rotation: ox -> y slot, -oy -> x slot, oz -> z slot (negated)
    // Then negate and scale
    double xp = -oy;   // stored to [esi-0x78]
    double yp = ox;     // stored to [esi-0x70]
    double zp = oz;     // stored to [esi-0x68]

    // --- Step 3: 3D Apollonian inversion ---
    // ASM 0xAC-0xFF:
    // xp_sq = xp^2; fst [esp+0x10]
    // s2 = xp^2 + yp^2; fst [esp]  (= xp^2+yp^2)
    // Then: add these together... let me trace:
    // 0xAC: fld [esi-0x78] = xp
    // 0xAF: fst [esp+0x10] (save xp)
    // 0xB3: fmul st(0) = xp^2; fst [esp] (save xp^2)
    // 0xB8: fld [esi-0x70] = yp
    // 0xBB: fmul st(0) = yp^2
    // 0xBD: fld st(0) = dup yp^2
    // 0xBF: fadd [esp] = yp^2 + xp^2 = s2_xy
    // 0xC2: fstp [esp] (overwrite with s2_xy)
    // 0xC5: faddp => xp^2 + yp^2... wait, what's on the stack?
    // After 0xBF: st0 = s2_xy, st1 = yp^2, st2 = xp^2
    // fstp [esp] pops s2_xy to memory
    // st0 = yp^2, st1 = xp^2
    // 0xC5: DE C1 = faddp st(1) => xp^2 + yp^2... but that's same as s2_xy
    // Hmm. But we already saved s2_xy. So this is just consuming.

    // Actually let me re-read:
    // 0xB3: fmul st(0) = xp^2
    // 0xB5: fst [esp] saves xp^2
    // 0xB8: fld [esi-0x70] = yp
    // 0xBB: fmul st(0) = yp^2
    // Stack: st0=yp^2, st1=xp^2
    // 0xBD: fld st(0) = dup yp^2 => st0=yp^2, st1=yp^2, st2=xp^2
    // 0xBF: fadd [esp] = yp^2 + [esp] = yp^2 + xp^2 = s2_xy
    //   But this is DC 04 24 = fadd qword ptr [esp]
    //   Actually [esp] has xp^2 (saved at 0xB5 as fst)
    //   st0 = yp^2 + xp^2 = s2_xy
    // 0xC2: fstp [esp] = save s2_xy, pop
    //   Stack: st0=yp^2, st1=xp^2
    // 0xC5: DE C1 = faddp st(1) => st1 += st0 = xp^2 + yp^2, pop
    //   st0 = s2_xy (again, but computed differently? No, same value)
    //   This is redundant but probably the compiler artifact.
    // 0xC7: fld1 => st0=1, st1=s2_xy
    // 0xC9: DE E9 = fsubp => st0 = s2_xy - 1 = A
    //   (FSUBP st(1): st(1) = st(1) - st(0) = s2_xy - 1)

    // 0xCB: fld [esp] = s2_xy
    // 0xCE: fld1 => 1
    // 0xD0: faddp => s2_xy + 1
    // 0xD2: fld [esi-0x70] = yp
    // 0xD5: fadd st(0) => 2*yp
    // 0xD7: faddp => s2_xy + 1 + 2*yp = B
    // Stack: st0=B, st1=A
    // 0xD9: fst [esp+8] (save B)
    // 0xDD: DE F9 = fdivp st(1) => st(1) = st(1)/st(0) = A/B, pop
    //   Wait: DE F8+i: DE F9 = FDIVP st(1): st(1) = st(1)/st(0), pop
    //   So st0 = A / B
    // 0xDF: fadd [edi-0x58] => A/B + FinalAddX
    // 0xE2: fst [esp+0x20] (save this = new_x_like)
    // 0xE6: fdiv [edi-0x50] => / GScale (undo the initial scaling)
    // 0xE9: fstp [esi-0x78] => store to x output

    // 0xEC: fld [esp+0x10] = xp (saved earlier)
    // 0xF0: fadd st(0) => 2*xp
    // 0xF2: fchs => -2*xp
    // 0xF4: fdiv [esp+8] => -2*xp / B
    // 0xF8: fst [esp+0x28] (save new_y_like)
    // 0xFC: fdiv [edi-0x50] => / GScale
    // 0xFF: fstp [esi-0x70] => store to y output

    double s2_xy = xp * xp + yp * yp;
    double A_val = s2_xy - 1.0;
    double B_val = s2_xy + 1.0 + 2.0 * yp;

    double new_x_raw = A_val / B_val + FinalAddX;
    double new_y_raw = -2.0 * xp / B_val;

    double out_x = new_x_raw / GScale;
    double out_y = new_y_raw / GScale;

    // --- Step 4: Volcano fixer and DE ---
    // 0x103-0x12:
    // Compute DE correction factor:
    // fld [esp+0x20] = new_x_raw; fmul st(0) = new_x_raw^2
    // fld [esp+0x28] = new_y_raw; fmul st(0) = new_y_raw^2
    // faddp => new_x_raw^2 + new_y_raw^2
    // fadd [edi-0x60] = + Volcano
    // fsqrt => de_factor = sqrt(new_x^2 + new_y^2 + Volcano)
    //
    // fld [edi-0x20] = MulZ
    // fdivrp => MulZ / de_factor
    // This is the DE correction.
    // fld st(0); fabs => |MulZ/de_factor|
    // fmul [esi+0x70] => de *= |MulZ/de_factor|
    // fstp [esi+0x70]
    // Then: fmul [esi-0x68] => z * (MulZ/de_factor)
    // fstp [esi-0x68]

    double de_factor = sqrt(new_x_raw * new_x_raw + new_y_raw * new_y_raw + Volcano);
    double scale_factor = MulZ / de_factor;

    de *= fabs(scale_factor);
    double out_z = zp * scale_factor;

    return cpu_double3(out_x, out_y, out_z);
}

// Alternative direct function
cpu_double3 Apollo3DIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_APOLLO3DIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_Apollo3DIFS = {
    .id = FORMULA_APOLLO3DIFS,
    .name = "Apollo3D-IFS",
    .param_count = 10,
    .complexity = 304,
    .uses_transcendental = true
};

/*
Original M3 Description:
Attempt to extend the Apollonian fractal to 3D; the shapes will generate "volcanos"
when they get closer to critical points! Be careful with the coefficients, because
only a subset of parameters work with this variant. Also volcanoes can be noisy...
Keep scale=1 on all shapes with "apply scale" enabled. Also to make the volcanoes
look better use a Z add on the 1st shape with "apply scale" enabled (like 0.03)
and adjust zmul (higher decreases the noise).
DE fixed (thanks Jesse) + Formula modified ("volcano" formula and cleanup)
Luca GN 2012

Mathematical Formula (DEoption=21, dIFS transform):
  Step 1: Scale inputs by GScale, polyfold in XY plane
  Step 2: Apply 3x3 rotation matrix with AddX2 offset
  Step 3: 3D Apollonian inversion (Mobius transform):
    s2 = x'^2 + y'^2
    B = s2 + 1 + 2*y'
    new_x = (s2-1)/B + FinalAddX
    new_y = -2*x'/B
    Undo GScale: divide by GScale
  Step 4: Volcano fixer DE:
    de_factor = sqrt(new_x^2 + new_y^2 + Volcano)
    scale = MulZ / de_factor
    z_out = z' * scale
    de *= |scale|

Parameters:
  [0] Add X 1: double (default: 0)
  [1] Add X 2: double (default: 0)
  [2] Mul Z: double (default: 0.612)
  [3..5] Rotation angles (degrees, from .3SingleAngles)
  [6] PolyFold Order: integer (default: 3)
  [7] Global XY-scale: double (default: 0.081)
  [8] Final add X: double (default: 0.5718)
  [9] Volcano fixer: double (default: 0.291)

Constants:
  [PVar+0x00] = 0.159154943 (1/(2*pi))
  [PVar+0x08] = 6.283185307 (2*pi)
  [PVar+0x10] = 1.57079632675 (pi/2)
  [PVar+0x18] = 0.017453293 (pi/180)
*/
