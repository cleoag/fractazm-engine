// BulboxP-2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// BulboxP-2: shipped 553-byte x87 formula, default DEoption 0.
// Fold XYZ, measure its radius, then scale and translate before the inner
// negative-square transform. The boundary r == InnerR uses the full transform.
// Julia C is added before restoring the center offsets, with explicit Double
// stores. The Win64 helper preserves the original intermediate precision.

constexpr unsigned int FORMULA_BULBOXP2 = 10273;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BulboxP2Original_x87(double* xyzw, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA_BULBOXP2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    BulboxP2Original_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const auto fold=[&](E v) {
        return E(double(abs(v+E(params[1]))+(-abs(v-E(params[1]))-v)));
    };
    E x=fold(E(p.x)),y=fold(E(p.y)),z=fold(E(p.z));
    const E square=x*x+(y*y+z*z);
    const E savedSquare=double(square),radius=double(sqrt(square));
    x=double(x*params[0]-E(params[5]));
    y=double(y*params[0]-E(params[6]));
    z=double(z*params[0]-E(params[7]));
    if(int(params[9])==0 && radius>=E(1)) {
        x=double(x+E(params[10]));
        y=double(y+E(params[11]));
        z=double(z+E(params[12]));
    } else {
        // The comparison is InnerR >= radius; unordered takes interpolation.
        const bool interpolate=!(E(params[2])>=radius);
        E blend(0),complement(1);
        if(interpolate) {
            const E fraction=(radius-E(params[2]))/(E(1)-E(params[2]));
            blend=double(fraction);
            complement=double(E(1)-fraction); // FST of blend does not pop it.
        }
        const E rxy=double(sqrt((y*y+x*x)+abs(E(params[4]))));
        const E inverse=double(E(params[3])/(savedSquare*savedSquare));
        const E zr=z/rxy;
        const E angular=double(-(zr*zr)+E(1));
        const E nx=((x*x+(-(y*y)))*angular)*inverse;
        const E ny=(-(x*y)*angular)*inverse;
        E nz=-(z+z);
        if(interpolate) {
            nz=((nz*params[8])*rxy)*inverse;
            z=double((nz*complement+z*blend)+E(params[12]));
            y=double((ny*complement+y*blend)+E(params[11]));
            x=double((nx*complement+x*blend)+E(params[10]));
        } else {
            nz=((nz*rxy)*inverse)*params[8];
            z=double(nz+E(params[12]));
            y=double(ny+E(params[11]));
            x=double(nx+E(params[10]));
        }
    }
    // The original C-addition stores precede the three center restorations.
    return cpu_double3(double(x+E(params[5])),double(y+E(params[6])),double(z+E(params[7])));
#else
    double Scale     = params[0];    // [edi-0x10], default 2
    double Fold      = params[1];    // [edi-0x18], default 1
    double InnerR    = params[2];    // [edi-0x20], default 0.6
    double InnerScale= params[3];    // [edi-0x28], default -0.5
    double Unsharp   = params[4];    // [edi-0x30], default 0
    double xCen      = params[5];    // [edi-0x38], default 0
    double yCen      = params[6];    // [edi-0x40], default 0
    double zCen      = params[7];    // [edi-0x48], default 0
    double innerZMul = params[8];    // [edi-0x50], default 1
    int   disableBox= (int)params[9]; // [edi-0x54], default 0

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // --- Step 1: Tglad box folding ---
    // ASM 0x16-0x56: for each axis:
    //   result = |x+F| - |x-F| - x  (then add to original, effectively folds)
    // Actually: fld x, fadd F, fabs => |x+F|
    //           fld x, fsub F, fabs, fchs => -|x-F|
    //           fsub x => -|x-F| - x
    //           faddp => |x+F| - |x-F| - x
    x = fabs(x + Fold) - fabs(x - Fold) - x;
    y = fabs(y + Fold) - fabs(y - Fold) - y;
    z = fabs(z + Fold) - fabs(z - Fold) - z;

    // --- Step 2: r^2 and r ---
    // ASM 0x58-0x6D
    double r2 = x * x + y * y + z * z;
    double r  = sqrt(r2);

    // --- Step 3: Scale and subtract center ---
    // ASM 0x70-0x8E: x = x*Scale - xCen, etc.
    x = x * Scale - xCen;
    y = y * Scale - yCen;
    z = z * Scale - zCen;

    // --- Step 4: Branching on radius ---
    if (disableBox == 0) {
        // ASM exits early only when the point is already outside the boxy
        // core region, i.e. r >= 1.
        if (r >= 1.0) {
            // Original stores C addition before restoring the center.
            x = (x + params[10]) + xCen;
            y = (y + params[11]) + yCen;
            z = (z + params[12]) + zCen;
            return cpu_double3(x, y, z);
        }
    }

    // ASM 0xAB-0xB7:
    //   r < InnerR  -> full power path
    //   InnerR <= r < 1 -> interpolation path
    if (r > InnerR) {
        // --- Interpolation path (0xBF-0x16D) ---
        // k = (r - InnerR) / (1 - InnerR)
        double k = (r - InnerR) / (1.0 - InnerR);
        double one_minus_k = 1.0 - k;

        // Compute rxy, r4inv, a same as inner path
        double rxy = sqrt(x * x + y * y + fabs(Unsharp));
        double r4inv = InnerScale / (r2 * r2);
        double zr = z / rxy;
        double a = 1.0 - zr * zr;

        // W/N power -2 vector (Tvec)
        double Tvec_x = (x * x - y * y) * a * r4inv;
        double Tvec_y = -x * y * a * r4inv;
        double Tvec_z = -2.0 * z * innerZMul * rxy * r4inv;

        // Interpolation: result = Tvec * (1-k) + old * k + C
        // ASM stores z first (0x140-0x14D), then y (0x14-0x15C), then x (0x15E-0x16B)
        // Julia C is added below, before center restoration.
        z = Tvec_z * one_minus_k + z * k;
        y = Tvec_y * one_minus_k + y * k;
        x = Tvec_x * one_minus_k + x * k;
    } else {
        // --- Full power path (0x18E-0x206) ---
        double rxy = sqrt(x * x + y * y + fabs(Unsharp));
        double r4inv = InnerScale / (r2 * r2);
        double zr = z / rxy;
        double a = 1.0 - zr * zr;

        // ASM 0x1C4-0x1F6:
        double new_x = (x * x - y * y) * a * r4inv;
        double new_y = -x * y * a * r4inv;
        double new_z = -2.0 * z * rxy * r4inv * innerZMul;

        x = new_x;
        y = new_y;
        z = new_z;
    }

    // --- Step 7: Add center back ---
    // ASM 0x207-0x21A
    x = (x + params[10]) + xCen;
    y = (y + params[11]) + yCen;
    z = (z + params[12]) + zCen;

    return cpu_double3(x, y, z);
#endif
}

constant FormulaInfo formula_info_bulboxp2 = {
    .id = FORMULA_BULBOXP2,
    .name = "BulboxP-2",
    .param_count = 10,
    .complexity = 0,
    .uses_transcendental = false
};
