// Seashell.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Seashell
// Hand-ported from ASM: 395 bytes, DEoption=20
// dIFS shape — SSE2 IFS transform + x87 seashell surface DE
// Seashell surface using 3x3 rotation matrix, power functions, and sincos

constexpr unsigned int FORMULA_SEASHELL = 10109;

// ASM trace summary:
//   0x00-0x40: Standard SSE2 IFS preamble (Scale, X/Y/Z_add, Apply_scale_add)
//   0x40-0x4A: Store transformed coords to stack: [esp]=x, [esp+8]=y, [esp+0x10]=z
//   0x4B-0x96: 3x3 rotation: ROW2 first ([edi-0x70..78]), ROW0 second ([edi-0x58..60]),
//              ROW1 third ([edi-0x64..6C]). fstp: [esp]=ROW0=rx, [esp+8]=ROW1=ry, [esp+0x10]=ROW2=rz
//   0x9A-0xA4: initial_de = |rz - 0.5| - 0.5  (slab from ROW2=[esp+0x10])
//   0xA7-0xCD: power1 = -(|rz-1|^ZPower); fst overwrites [esp+0x10] with power1
//   0xD1-0xF6: angle = |power1|^SinZPower * Skew; fsincos -> [esp+0x20]=cos, [esp+0x18]=sin
//   0xFA-0x130: A = rx*sin - ry*cos; B = rx*cos + ry*sin + power1*Dist; dist = sqrt(A^2+B^2)
//   0x132-0x14A: if |Skew| > 1 (strictly): dist /= sqrt(|Skew|)
//   0x14A-0x154: dist += power1 * Thick  ([esp+0x10] = power1, NOT rz)
//   0x156-0x168: shape_de = max(initial_de, dist)  (CSG intersection: keeps LARGER value)
//   0x168-0x174: Scale correction; store to [esi-0x20]
//   0x174-0x18A: OTrap = |[edi-0x10] + iteration_count * [edi-0x4c]| -> stored at [esi+0x80]

template<>
cpu_double3 execute_formula<FORMULA_SEASHELL>(cpu_double3 p, constant double* params, double &de) {
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    // The shipped code uses the layout from before X_add became .2Doubles.
    // Preserve its overlapping loads: [PVar-0x44] is the upper word of X_add,
    // and [PVar-0x4c] spans Apply_scale_add and the lower word of X_add.
    // The declared OTrapOnIts at -0x54 is not read by this asset's code.
    uint64_t xAddBits;
    std::memcpy(&xAddBits, &params[4], sizeof(xAddBits));
    const int Apply_scale_add = static_cast<uint32_t>(xAddBits >> 32);
    const uint64_t trapBits = (xAddBits << 32) | static_cast<uint32_t>(static_cast<int32_t>(params[5]));
    double OTrapOnIts;
    std::memcpy(&OTrapOnIts, &trapBits, sizeof(OTrapOnIts));

    const double OTrapOffset = params[0];
    const double Scale = params[1];    // .2Doubles — 1 raw value
    const double Z_add = params[2];    // .2Doubles — 1 raw value
    const double Y_add = params[3];
    const double X_add = params[4];    // .2Doubles — 1 raw value
    // .3SingleAngles Rotate — 3 raw angles → buildRotMatrix3
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 7);
    const double Skew = params[16];       // [edi-0x80]
    const double Dist = params[17];       // [edi-0x88]
    const double Thick = params[18];      // [edi-0x90]
    const double SinZPower = params[19];  // [edi-0x98]
    const double ZPower = params[20];     // [edi-0xA0]
    // [CONSTANTS]: const0 = 0.5 at [edi+0x00]
    const double halfConst = 0.5;

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: 3x3 rotation matrix multiply
    // ASM computes three rows using matrix entries stored as singles in PVar.
    // The fstp store order at 0x8F-0x96 gives:
    //   [esp]      = ROW0 = x*m[0][0] + y*m[0][1] + z*m[0][2]  (using [edi-0x58..60])
    //   [esp+8]    = ROW1 = x*m[1][0] + y*m[1][1] + z*m[1][2]  (using [edi-0x64..6C])
    //   [esp+0x10] = ROW2 = x*m[2][0] + y*m[2][1] + z*m[2][2]  (using [edi-0x70..78])
    double rx = x * Rot.col0.x + y * Rot.col1.x + z * Rot.col2.x;  // ROW0 = [esp]
    double ry = x * Rot.col0.y + y * Rot.col1.y + z * Rot.col2.y;  // ROW1 = [esp+8]
    double rz = x * Rot.col0.z + y * Rot.col1.z + z * Rot.col2.z;  // ROW2 = [esp+0x10]

    // Step 3: Initial slab DE uses [esp+0x10] = ROW2 = rz
    // 0x9A: fld [esp+0x10]; fsub [edi]; fabs; fsub [edi]; fstp [esi-0x20]
    double initial_de = fabs(rz - halfConst) - halfConst;

    // Step 4: First power uses [esp+0x10] = ROW2 = rz
    // 0xA7-0xCB: fld [esp+0x10], fld1, fsubp -> (rz-1), fabs, fyl2x with ZPower -> pow -> fchs
    double power1 = -x87_log_power(fabs(rz - 1.0), ZPower);
    // 0xCD: fst [esp+0x10] — OVERWRITES [esp+0x10] with power1!
    // All subsequent reads of [esp+0x10] get power1, not rz.

    // Step 5: Second power and angle: pow(|power1|, SinZPower) * Skew -> fsincos
    // 0xD1-0xF0: fyl2x with SinZPower on |power1| -> pow -> * Skew -> fsincos
    // fsincos: st(0)=cos, st(1)=sin; fstp [esp+0x20]=cos, fstp [esp+0x18]=sin
    double angle = x87_log_power(fabs(power1), SinZPower) * Skew;
    double cos_a = cos(angle);
    double sin_a = sin(angle);

    // Step 6: Distance computation (tube around spiral curve)
    // Uses [esp]=ROW0=rx for sin/cos terms, [esp+8]=ROW1=ry for sin/cos terms,
    // and [esp+0x10]=power1 (overwritten!) for the Dist term.
    //
    //   0xFA:  fld [esp]        -> rx (ROW0)
    //   0xFD:  fmul [esp+0x18]  -> rx * sin
    //   0x101: fld [esp+8]      -> ry (ROW1)
    //   0x105: fmul [esp+0x20]  -> ry * cos
    //   0x109: fchs             -> -(ry*cos)
    //   0x10B: faddp            -> rx*sin - ry*cos = A
    //   0x10D: fmul st(0)       -> A^2
    //
    //   0x10F: fld [esp]        -> rx (ROW0)
    //   0x112: fmul [esp+0x20]  -> rx * cos
    //   0x116: fld [esp+8]      -> ry (ROW1)
    //   0x11A: fmul [esp+0x18]  -> ry * sin
    //   0x11E: faddp            -> rx*cos + ry*sin
    //   0x120: fld [esp+0x10]   -> power1 (NOT rz!)
    //   0x124: fmul [edi-0x88]  -> power1 * Dist
    //   0x12A: faddp            -> rx*cos + ry*sin + power1*Dist = B
    //   0x12C: fmul st(0)       -> B^2
    //   0x12E: faddp            -> A^2 + B^2
    //   0x130: fsqrt            -> dist = sqrt(A^2 + B^2)
    double A = rx * sin_a - ry * cos_a;
    double B = rx * cos_a + ry * sin_a + power1 * Dist;
    double dist = sqrt(A * A + B * B);

    // Step 7: Conditional scaling by |Skew|
    // 0x132: fld [edi-0x80] -> fabs -> fld1 -> fcomp st(1)
    // fcomp compares 1.0 vs |Skew|; jb = jump if 1.0 < |Skew| (strictly greater)
    // Division only when |Skew| > 1.0, NOT when |Skew| == 1.0
    double absSkew = fabs(Skew);
    if (absSkew > 1.0) {
        dist /= sqrt(absSkew);
    }

    // Step 8: Thick contribution uses [esp+0x10] = power1 (overwritten at 0xCD)
    // 0x14A: fld [esp+0x10] -> power1
    // 0x14E: fmul [edi-0x90] -> power1 * Thick
    // 0x154: faddp -> dist + power1 * Thick
    dist += power1 * Thick;

    // Step 9: shape_de = max(initial_de, dist)
    // 0x156: fld [esi-0x20]  -> initial_de
    // 0x159: fcom st(1)      -> compare initial_de vs dist
    // fnstsw/shr ah,1/jae:
    //   jae taken (initial_de >= dist) -> fstp st(1) -> keep initial_de
    //   jae not taken (initial_de < dist) -> fstp st(0) -> keep dist
    // Both paths keep the LARGER value = max(initial_de, dist)
    double shape_de = initial_de >= dist ? initial_de : dist;

    // Step 10: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    // Step 11: OTrap computation (stored at [esi+0x80])
    // 0x174: fld [edi-0x10]   -> OTrapOffset
    // 0x177: fild [esi-0x18]  -> iteration count
    // 0x17A: fmul [edi-0x4C]  -> iter * overlapping double (see above)
    // 0x17D: faddp st(1)     -> OTrapOffset + iter * OTrapOnIts
    // 0x17: fabs             -> |...|
    params[63] = fabs(OTrapOffset + params[ITER_COUNT_SLOT] * OTrapOnIts);

    return cpu_double3(state_x, state_y, state_z);
}

// Alternative direct function implementation
cpu_double3 Seashell_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_SEASHELL>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_Seashell = {
    .id = FORMULA_SEASHELL,
    .name = "Seashell",
    .param_count = 15,
    .complexity = 395,
    .uses_transcendental = true
};

/*
Original M3 Description:
dIFS shape that should look like a seashell.

Mathematical Formula (from ASM trace):
  DEoption=20 (dIFS shape with SSE2 IFS + 3x3 rotation + spiral tube DE):

  Step 1: IFS transform:
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: de *= Scale

  Step 2: Rotate (x,y,z) by 3x3 rotation matrix -> (rx=ROW0, ry=ROW1, rz=ROW2)

  Step 3: Initial slab DE = |rz - 0.5| - 0.5  (uses ROW2)

  Step 4: power1 = -(|rz - 1|^ZPower)  (uses ROW2, then overwrites rz slot)

  Step 5: angle = |power1|^SinZPower * Skew; compute sin(angle), cos(angle)

  Step 6: Tube distance (uses ROW0=rx, ROW1=ry, and power1 for Dist term):
    A = rx * sin(angle) - ry * cos(angle)
    B = rx * cos(angle) + ry * sin(angle) + power1 * Dist
    dist = sqrt(A^2 + B^2)

  Step 7: if |Skew| > 1: dist /= sqrt(|Skew|)  (strictly greater, not >=)

  Step 8: dist += power1 * Thick  (uses power1, not rz)

  Step 9: shape_de = max(initial_slab_de, dist)  (CSG intersection)

Parameters (raw M3 layout):
  [0] OTrapOffset: double (default: 0)
  [1] Scale: double (default: 2) — .2Doubles raw=1
  [2] Z_add: double (default: 0) — .2Doubles raw=1
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0) — .2Doubles raw=1
  [5] Apply_scale_add: int (default: 0)
  [6] OTrapOnIts: double (default: 0.25)
  [7..9] Rotate: 3SingleAngles (3 raw angles → buildRotMatrix3)
  [10] Skew: double (default: 12.0)
  [11] Dist: double (default: 0.7)
  [12] Thick: double (default: 0.15)
  [13] SinZPower: double (default: 0.35)
  [14] ZPower: double (default: 1)
  Constants: 0.5
*/
