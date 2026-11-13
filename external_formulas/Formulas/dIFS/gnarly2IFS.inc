// Port of the shipped x86 heightfield formula; DEoption=20.
// Transform coordinates are committed only when Apply_scale_add is nonzero.
// A rejected bound preserves Dfree1; DEC/JG executes the orbit at least once.
// Known reference defect: Flip_XZ=1 tests [ESP+0x20] outside its 28-byte
// local frame. That caller-dependent bound is not portable or parity-covered.
// The C++ flipped-axis behavior remains an approximation, recorded separately.

constexpr unsigned int FORMULA_GNARLY2IFS = 10089;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void gnarly2IFSEvenOriginal_x87(double*, const double*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_GNARLY2IFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    if ((int(params[8]) & 1) == 0) {
        double state[] = {p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT],
                          params[MAX_FORMULA_PARAMS-1],params[DIFS_ROLD_SLOT],
                          0.,0.,0.,0.,1.5,.1};
        const int selectors[] = {int(params[6]),int(params[7])};
        gnarly2IFSEvenOriginal_x87(state,params,selectors);
        de=state[3];
        params[DIFS_VARYSCALE_SLOT]=state[4];
        params[MAX_FORMULA_PARAMS-1]=state[5];
        if (selectors[0]) params[DIFS_COMMIT_XYZ_SLOT]=1.;
        return cpu_double3(state[0],state[1],state[2]);
    }
#endif
    // Portable and odd-Flip fallback; odd Flip depends on the Pascal caller
    // stack in the original CODE and is not certified by point-oracle tests.

    const double Beta       = params[0];   // [edi - 0x10]  default: -4
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const double Amplitude  = params[5];   // [edi - 0x48]
    const int   Apply_scale_add = int(params[6]); // [edi - 0x4c]
    const int   Iters      = int(params[7]);       // [edi - 0x50]
    const int   Flip_XZ    = int(params[8]);       // [edi - 0x54]
    const double Alpha      = params[9];   // [edi - 0x5c]
    const double Step       = params[10];  // [edi - 0x64]
    const double Gamma      = params[11];  // [edi - 0x6c]
    const double OTrapClrMul = params[12]; // [edi - 0x74]
    const double const_1_5  = 1.5;
    const double const_0_1  = 0.1;

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }



    double xx, yy;
    double ref_axis;  // the axis subtracted at the end (same as "height" axis)

    if ((Flip_XZ & 1) != 0) {
        xx = z;    // iterate in (z, y) space
        yy = y;
        ref_axis = x;
    } else {
        xx = x;    // iterate in (x, y) space
        yy = y;
        ref_axis = z;  // subtract z
    }

    if ((Flip_XZ & 1) == 0 && !((fabs(Amplitude) + 0.1)*1.5 + params[DIFS_ROLD_SLOT] >= fabs(z))) {
        de = Apply_scale_add != 0 ? -z : -z / Scale;
        return Apply_scale_add != 0 ? cpu_double3(x,y,z) : p;
    }
    int iter_count = std::max(1, Iters); // body precedes DEC/JG
    for (int i = 0; i < iter_count; i++) {
        double s1 = sin(Alpha * (xx + Beta * cos(xx)));
        double yy_new = yy + Step * sin(Gamma * (xx - yy) + s1);

        double s2 = sin(Alpha * (yy + Beta * cos(yy)));
        double xx_new = xx + Step * sin(Gamma * (xx + yy) + s2);

        xx = xx_new;
        yy = yy_new;
    }

    double gnarly_r = sqrt(xx * xx + yy * yy);

    double ref_other;
    if ((Flip_XZ & 1) != 0) {
        ref_other = z;
    } else {
        ref_other = x;
    }
    double ref_r = sqrt(y * y + ref_other * ref_other);

    double shape_de_raw = gnarly_r - ref_r;

    double otrap_val;
    if (OTrapClrMul > 0.0) {
        otrap_val = fabs(shape_de_raw * OTrapClrMul);
    } else {
        otrap_val = fabs(sqrt(fabs(shape_de_raw + 2.0)) * OTrapClrMul);
    }
    params[MAX_FORMULA_PARAMS - 1] = otrap_val;

    double shape_de = shape_de_raw * Amplitude - ref_axis;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
}

cpu_double3 gnarly2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_GNARLY2IFS>(p, params, de);
}

constant FormulaInfo formula_info_gnarly2IFS = {
    .id = FORMULA_GNARLY2IFS,
    .name = "gnarly2IFS",
    .param_count = 13,
    .complexity = 312,
    .uses_transcendental = true
};

