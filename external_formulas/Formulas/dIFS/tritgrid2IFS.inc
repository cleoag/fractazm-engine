// tritgrid2IFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: tritgrid2IFS
// Hand-ported from ASM: 417 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS + 3SingleAngles + triangular tube grid)
// Triangular grid of tubes discovered by accident

constexpr unsigned int FORMULA_TRITGRID2IFS = 10227;

template<>
cpu_double3 execute_formula<FORMULA_TRITGRID2IFS>(cpu_double3 p, constant double* params, double &de) {
    // Raw param layout: [0] RadiusMod, [1] Scale (.2Doubles raw=1), [2] Z_add (.2Doubles raw=1),
    // [3] Y_add, [4] X_add, [5] Grid_styling, [6] Apply_scale_add,
    // [7..9] 3 angles (.3SingleAngles raw=3) → buildRotMatrix3
    const double RadiusMod       = params[0];
    const double Scale           = params[1];    // .2Doubles — 1 raw value
    const double Z_add           = params[2];    // .2Doubles — 1 raw value
    const double Y_add           = params[3];
    const double X_add           = params[4];
    const double Grid_styling    = params[5];
    const int Apply_scale_add   = int(params[6]);

    // Keep the asset's decimal constants, including its truncated sqrt(3)/2.
    const double c_half = params[16];
    const double c_s32 = params[17];
    const double c_one = params[18];

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    const cpu_double3 committed = Apply_scale_add ? cpu_double3(x, y, z) : p;

    // Step 2: 3SingleAngles rotation (3 raw angles → buildRotMatrix3)
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 7);
    double rx = x * Rot.col0.x + y * Rot.col1.x + z * Rot.col2.x;
    double ry = x * Rot.col0.y + y * Rot.col1.y + z * Rot.col2.y;
    double rz = x * Rot.col0.z + y * Rot.col1.z + z * Rot.col2.z;
    x = rx; y = ry; z = rz;

    // Step 3: Triangular grid cell computation
    // ASM 0xA0-0xA3: fsubr [edi-0x48] → Grid_styling - x
    // Then fist/fild/fsubp pattern = fractional part extraction

    // Direction 1: Grid_styling - x → fist → fild → subtract → abs
    double v1 = Grid_styling - x;
    int iv1 = x87_fistp_int32(v1);
    // fld st0, fadd [edi], fistp [esp+0x20] → iv1_adj = lrint(v1 + 0.5)
    int iv1_adj = x87_fistp_int32(v1 + c_half);
    double frac1 = fabs(v1 - double(iv1));

    // Direction 2: x * [edi] + y * [edi+8] = x*0.5 + y*0.866025
    // Then fld st1, fadd st1 → x*0.5 + y*0.866025 doubled? No:
    // ASM 0xBB-0xDB: fld x, mul [edi], fld y, mul [edi+8], then some complex ops
    // st1 = x*c_half, st0 = y*c_s32
    // fld st1 → push copy of x*c_half; fadd st1 → (x*c_half) + (y*c_s32) = v2
    double v2 = x * c_half + y * c_s32;
    int iv2 = x87_fistp_int32(v2);
    int iv2_adj = x87_fistp_int32(v2 + c_half);
    double frac2 = fabs(v2 - double(iv2));

    // Direction 3: FSUBP after FXCH gives y*c_s32 - x*c_half.
    // Then same fist/fild/abs pattern
    double v3 = y * c_s32 - x * c_half;
    int iv3 = x87_fistp_int32(v3);
    int iv3_adj = x87_fistp_int32(v3 + c_half);
    double frac3 = fabs(v3 - double(iv3));

    // Step 4: Modulo-3 coloring check
    // ASM 0xF8-0x15: for each direction, (adj+0x1000) % 3 == 0 → replace frac with c_one
    // This creates the triangular pattern by zeroing out every 3rd cell

    // x87 stack order at the modulo stage is [frac3, frac2, frac1].
    // The three divisibility checks replace those stack slots in the order:
    //   iv3_adj -> frac1
    //   iv1_adj -> frac2
    //   iv2_adj -> frac3
    if (((static_cast<uint32_t>(iv3_adj) + 0x1000u) % 3u) == 0) {
        frac1 = c_one;
    }
    if (((static_cast<uint32_t>(iv1_adj) + 0x1000u) % 3u) == 0) {
        frac2 = c_one;
    }
    if (((static_cast<uint32_t>(iv2_adj) + 0x1000u) % 3u) == 0) {
        frac3 = c_one;
    }

    // FCOM/JB selects the lower distance, retaining the first for unordered.
    double d_min = !(frac3 >= frac2) ? frac3 : frac2;
    d_min = !(d_min >= frac1) ? d_min : frac1;

    // Step 6: Tube DE = sqrt(d_min^2 + z^2) - RadiusMod
    double shape_de = sqrt(d_min * d_min + z * z) - RadiusMod;

    // Step 7: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return committed;
}

cpu_double3 tritgrid2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TRITGRID2IFS>(p, params, de);
}

constant FormulaInfo formula_info_tritgrid2IFS = {
    .id = FORMULA_TRITGRID2IFS,
    .name = "tritgrid2IFS",
    .param_count = 10,
    .complexity = 417,
    .uses_transcendental = false
};

/*
Original M3 Description:
Iso-surface made of a triangular grid of tubes.
This formula was discovered by accident fiddling with tritgrid,
the pattern is pretty so I left it as a bonus formula.
Luca GN 2012
*/
