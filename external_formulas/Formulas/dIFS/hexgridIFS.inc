// hexgridIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: hexgridIFS
// Hand-ported from ASM: 475 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS + 3SingleAngles + hexagonal grid with z-thickness)
// Modified hextgrid showing solid shapes instead of toruses

constexpr unsigned int FORMULA_HEXGRIDIFS = 10233;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HexGridSolidDistance_x87(double*, const double*, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_HEXGRIDIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Spacing = params[0];         // [edi-0x10]
    const double Scale = params[1];           // .2Doubles
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Grid_styling = params[5];    // [edi-0x48]
    const int Apply_scale_add = int(params[6]);
    // params[7..15] = 3SingleAngles rotation matrix (9 single floats)
    // params[16..17] = SingleAngle Rho → sin/cos pair
    const double Rho_sin = params[16];
    const double Rho_cos = params[17];
    const double Final_add = params[18];      // [edi-0x88]
    const double zThick = params[19];

    // [CONSTANTS] embedded as compile-time values:
    const double c_half = params[20];            // [edi+0x00]
    const double c_invs3 = params[21];           // [edi+0x08]
    const double c_ns32 = params[22];            // [edi+0x10]
    const double c_nhalf = params[23];           // [edi+0x18]
    const double c_s34 = params[24];             // [edi+0x20]

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    const cpu_double3 transformed(x,y,z);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={x,y,z,0.,0.};
    HexGridSolidDistance_x87(state,params,Apply_scale_add);
    de=state[4];
    return Apply_scale_add != 0 ? transformed : p;
#else
    // Portable shape arithmetic remains uncertified.
    // Step 2: 3SingleAngles rotation
    double m00 = params[7],  m01 = params[8],  m02 = params[9];
    double m10 = params[10], m11 = params[11], m12 = params[12];
    double m20 = params[13], m21 = params[14], m22 = params[15];
    double rx = x * m00 + y * m01 + z * m02;
    double ry = x * m10 + y * m11 + z * m12;
    double rz = x * m20 + y * m21 + z * m22;
    x = rx; y = ry; z = rz;

    // Step 3: Hex grid folding (same as hextgridIFS but with abs(z) thickness)
    // ASM 0xA0: x = |x - Grid_styling|
    double ax = fabs(x - Grid_styling);

    // Fold x into hex cell
    // ax/Grid_styling * 0.5 - 0.5 → round → *2+1 → *Grid_styling → negate → add ax → abs
    double fx = x87_frndint(ax / Grid_styling * c_half - c_half);
    fx = fabs(ax - (fx * 2.0 + 1.0) * Grid_styling);

    // hy = Grid_styling * c_invs3 (hex height)
    double hy = Grid_styling * c_invs3;
    // Fold y
    double ay = fabs(y - hy);
    double fy = x87_frndint(ay / hy * c_half - c_half);
    fy = fabs(ay - (fy * 2.0 + 1.0) * hy);

    // Step 4: Two hex lattice iterations (Knighty-style)
    // Iteration 1
    // ASM 0x106-0x13E: both fx and fy get fabs applied
    // d1 + (-|d1|) = d1 - |d1| = 2*min(d1, 0)
    double d1 = fx * c_ns32 + fy * c_nhalf + c_s34;
    double s1 = d1 - fabs(d1); // = 2*min(d1, 0)

    // Update fx, fy using s1
    double nfx = fabs(fx - s1 * c_ns32);   // fabs applied
    double nfy = fabs(fy - s1 * c_nhalf);  // fabs applied

    // Iteration 2
    // ASM 0x142-0x17A: nfx does NOT get fabs (nop nop then fstp without fabs),
    // but nfy DOES get fabs (D9 E1 = fabs at 0x178)
    double d2 = nfx * c_ns32 + nfy * c_nhalf + c_s34;
    double s2 = d2 - fabs(d2); // 2*min(d2, 0)
    nfx = nfx - s2 * c_ns32;             // NO fabs for x in iteration 2
    nfy = fabs(nfy - s2 * c_nhalf);      // fabs applied for y

    // Step 5: Final DE = max(combined_hex_dist - Spacing, |z| - zThick)
    // ASM 0x17E-0x19E: nfx * Rho_sin + nfy * Rho_cos + Final_add
    // These come from SingleAngle Rho params
    double hex_dist = fabs(nfx * Rho_sin + nfy * Rho_cos + Final_add);

    // |z| - zThick
    double z_dist = fabs(z) - zThick;

    // max(hex_dist, z_dist) → then subtract Spacing
    // ASM: fcom; jbe pattern selects max
    double shape_de = max(hex_dist, z_dist) - Spacing;

    // Step 6: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? transformed : p;
#endif
}

// Alternative direct function implementation
cpu_double3 hexgridIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HEXGRIDIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_hexgridIFS = {
    .id = FORMULA_HEXGRIDIFS,
    .name = "hexgridIFS",
    .param_count = 21,
    .complexity = 475,
    .uses_transcendental = false
};

/*
Original M3 Description:
Modified version of hextgrid, to show solid shapes instead of toruses.
CutByUnrotZ allows you to get different effects if you use it together
with non-zero PlaneRot X,Y.
Luca GN 2012
*/
