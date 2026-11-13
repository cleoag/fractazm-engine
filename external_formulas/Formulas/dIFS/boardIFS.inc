// boardIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: boardIFS
// Hand-ported from ASM: 480 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + 3SingleAngles + checkerboard DE)
// Checkerboard pattern with multiple coordinate mapping variations

constexpr unsigned int FORMULA_BOARDIFS = 10231;

template<>
cpu_double3 execute_formula<FORMULA_BOARDIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Bump = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Checker_scale = params[5];
    const int Apply_scale_add = int(params[6]);
    // params[7..15] = 3SingleAngles rotation matrix (9 floats at single precision)
    const int Variation = int(params[16]);
    const double OTrapClr1 = params[17];
    const double OTrapClr2 = params[18];
    const double OTrapOnIts = params[19];

    // Constants: [edi+0x00]=300
    const double const_scale = params[20]; // 300

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: 3SingleAngles rotation (9 floats from params[7..15])
    // Pre-computed 3x3 rotation matrix stored as single-precision floats
    // Adjusted edi by -0x2c, so matrix entries at edi-0x3c..-0x24 etc.
    double m00 = params[7],  m01 = params[8],  m02 = params[9];
    double m10 = params[10], m11 = params[11], m12 = params[12];
    double m20 = params[13], m21 = params[14], m22 = params[15];
    double rx = x * m00 + y * m01 + z * m02;
    double ry = x * m10 + y * m11 + z * m12;
    double rz = x * m20 + y * m21 + z * m22;
    x = rx; y = ry; z = rz;

    // Step 3: Variation-dependent coordinate mapping
    // ASM at 0xA2-0x15: different transforms based on Variation parameter
    double u = x, v = y; // working coords for sin(u)*sin(v)

    if (Variation == 1) {
        // Inverse map: u,v = x,y / (x^2 + y^2)
        double r2 = x * x + y * y;
        double inv_r2 = 1.0 / r2;
        u = x * inv_r2;
        v = y * inv_r2;
    } else if (Variation == 2) {
        // Cayley/Mobius transform
        // ASM: x^2 + (y+1)^2 → inv, then (x^2+y^2-1)*inv → new_v, -2*x*inv → new_u (reversed)
        double denom = x * x + (y + 1.0) * (y + 1.0);
        double inv_denom = 1.0 / denom;
        double r2 = x * x + y * y;
        double new_v = (r2 - 1.0) * inv_denom;
        double new_u = -2.0 * x * inv_denom;
        // ASM adds [edi-0x40] = fadd qword ptr... this is the Xadd param area
        // Actually looking more carefully: the result is stored back
        v = new_v;
        u = new_u;
    } else if (Variation == 3) {
        // Logarithmic: factor = 1/log2(x^2+y^2), u *= factor, v *= factor
        double r2 = x * x + y * y;
        double factor = 1.0 / log2(r2);
        u = x * factor;
        v = y * factor;
    } else if (Variation == 4) {
        // Polar: r = sqrt(x^2 + y^2), theta = atan2(y, x)
        double r2 = x * x + y * y;
        double r = sqrt(r2);
        double theta = atan2(y, x);
        u = r;
        v = theta;
    }
    // Variation == 0: standard, u = x, v = y

    // Step 4: Checkerboard pattern: sin(u * Checker_scale) * sin(v * Checker_scale)
    double pattern = sin(u * Checker_scale) * sin(v * Checker_scale);

    // Step 5: Color trap assignment based on sign of pattern
    // ASM 0x176-0x18A: ftst, if pattern < 0 → OTrapClr1 else OTrapClr2
    // Plus iteration-dependent color: fild [esi-0x18] * OTrapOnIts + base
    // (orbit trap color stored to [esi+0x80])
    params[MAX_FORMULA_PARAMS - 1] =
        params[ITER_COUNT_SLOT] * OTrapOnIts +
        (!(pattern >= 0.0) ? OTrapClr1 : OTrapClr2);


    // Step 6: Shape DE computation
    // ASM 0x19B-0x1D0:
    // 0x19B: fmul [edi] = multiply pattern by const (300)
    // 0x19D: fdiv [edi-0x48] = divide by Checker_scale
    double shape_de_val = pattern * const_scale / Checker_scale;

    // 0x1A1: fld [esp+0x10] = z
    // 0x1A5: fchs → -z (negate), stored back to [esp+0x10]
    // 0x1A7: fst [esp+0x10] = save -z (FST, no pop)
    // 0x1AB: fadd [edi-0x10] = -z + Bump → call this z_de
    // Stack now: [z_de, shape_de_val]
    // 0x1AE: fcom st(1) → compare z_de vs shape_de_val
    // shr ah,1; jae 0x1BB → if z_de >= shape_de_val: keep shape_de_val
    //   0x1B5: fstp st(1) = pop shape_de_val, keep z_de? NO:
    //   jae: jump if CF=0 = z_de >= shape_de_val
    //   0x1B7: DD D9 = fstp st(1) → pop st(1)=shape_de_val, result = z_de
    //   not jae: 0x1B3: DD D8 = fstp st(0) → pop st(0)=z_de, result = shape_de_val
    // So result = min(z_de, shape_de_val) = min(-z+Bump, shape_de_val)
    double z_de = -z + Bump;
    double result = min(z_de, shape_de_val);

    // 0x1BD: fld [esp+0x10] = -z (saved earlier)
    // 0x1C1: fcom st(1) → compare -z vs result
    // shr ah,1; jae 0x1CE → if -z >= result: keep result (fstp st(1))
    //   0x1CE: DD D9 = fstp st(1) → keep -z... WAIT:
    //   jae: jump if CF=0 = -z >= result
    //   0x1CE: DD D8 = fstp st(0) → pop -z, keep result? Let me re-check:
    //   0x1C8: 73 04 = jae 0x1CE → at 0x1CA: DD D8 = fstp st(0) (discard -z, keep result)
    //   no wait, jae means "jump if above or equal" (CF=0), target is 0x1CE.
    //   0x1C8: jae 0x1CE → if -z >= result: jump
    //   At 0x1CE: DD D9 = fstp st(1) → pop st(1)=result, keep st(0)=-z
    //   Not jae (fall through) 0x1CA: DD D8 = fstp st(0) → pop -z, keep result
    // So:
    //   if -z >= result: shape = -z (keep -z when larger)
    //   if -z < result: shape = result (keep result when larger)
    // = max(-z, result) = max(-z, min(-z+Bump, shape_de_val))
    shape_de_val = max(-z, result);

    // Step 7: Scale correction
    if (Apply_scale_add == 0) {
        shape_de_val /= Scale;
    }

    de = shape_de_val;
    return cpu_double3(state_x, state_y, state_z);
}

// Alternative direct function implementation
cpu_double3 boardIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_BOARDIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_boardIFS = {
    .id = FORMULA_BOARDIFS,
    .name = "boardIFS",
    .param_count = 21,
    .complexity = 480,
    .uses_transcendental = true
};

/*
Original M3 Description:
Quick attempt to realize a checkerboard. (Function used; sin(x)*sin(y))
Variations available: 0=Standard, 1=Inverse map, 2=Cayley map, 3=Logarithmic, 4=Polar
Updated for new release; now supports colors.
Luca GN 2012
*/
