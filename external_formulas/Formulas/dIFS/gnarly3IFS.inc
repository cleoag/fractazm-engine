// Port of the shipped x86 heightfield formula; DEoption=20.
// Transform coordinates are committed only when Apply_scale_add is nonzero.
// A rejected bound preserves Dfree1; DEC/JG executes the orbit at least once.
// Known reference defect: Flip_XZ=1 tests [ESP+0x20] outside its 28-byte
// local frame. That caller-dependent bound is not portable or parity-covered.
// The C++ flipped-axis behavior remains an approximation, recorded separately.

constexpr unsigned int FORMULA_GNARLY3IFS = 10208;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void gnarly3IFSEvenOriginal_x87(double*, const double*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_GNARLY3IFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    if ((int(params[8]) & 1) == 0) {
        double state[] = {p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT],
                          params[MAX_FORMULA_PARAMS-1],params[DIFS_ROLD_SLOT],
                          0.,0.,0.,0.,1.5,.1};
        const int selectors[] = {int(params[6]),int(params[7])};
        gnarly3IFSEvenOriginal_x87(state,params,selectors);
        de=state[3];
        params[DIFS_VARYSCALE_SLOT]=state[4];
        params[MAX_FORMULA_PARAMS-1]=state[5];
        if (selectors[0]) params[DIFS_COMMIT_XYZ_SLOT]=1.;
        return cpu_double3(state[0],state[1],state[2]);
    }
#endif
    // Portable and odd-Flip fallback; odd Flip depends on the Pascal caller
    // stack in the original CODE and is not certified by point-oracle tests.

    const double Beta       = params[0];   // [edi - 0x10]  default: -3
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const double Amplitude  = params[5];   // [edi - 0x48]  default: 0.1
    const int   Apply_scale_add = int(params[6]); // [edi - 0x4c]
    const int   Iters      = int(params[7]);      // [edi - 0x50]  default: 16
    const int   Flip_XZ    = int(params[8]);      // [edi - 0x54]  default: 0
    const double Alpha      = params[9];   // [edi - 0x5c]  default: -2
    const double Step_Y     = params[10];  // [edi - 0x64]  default: 0.1
    const double Step_X     = params[11];  // [edi - 0x6c]  default: 0.1
    const double OTrapClrMul = params[12]; // [edi - 0x74]  default: 1

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }


    double xx, yy;
    double h_axis;  // axis used for heightfield subtraction at end

    if ((Flip_XZ & 1) != 0) {
        xx = z;
        yy = y;
        h_axis = x;
    } else {
        xx = x;
        yy = y;
        h_axis = z;
    }


    if ((Flip_XZ & 1) == 0 && !((fabs(Amplitude) + 0.1)*1.5 + params[DIFS_ROLD_SLOT] >= fabs(z))) {
        de = Apply_scale_add != 0 ? -z : -z / Scale;
        return Apply_scale_add != 0 ? cpu_double3(x,y,z) : p;
    }
    int iter_count = std::max(1, Iters); // body precedes DEC/JG
    for (int i = 0; i < iter_count; i++) {
        double sin_inner1 = sin(xx * Beta);
        double combined1 = sin(sin_inner1 + xx);
        double sign_sqrt_xx = xx * sqrt(fabs(xx));
        double yy_new = yy - Step_Y * sin(sign_sqrt_xx - Alpha * combined1);

        double sin_inner2 = sin(yy * Beta);
        double combined2 = sin(Alpha * (sin_inner2 + yy));
        double sign_sqrt_yy = yy * sqrt(fabs(yy));
        double xx_new = xx + Step_X * sin(sign_sqrt_yy + combined2);

        xx = yy_new;
        yy = xx_new;
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

    double otrap_val = shape_de_raw;
    if (OTrapClrMul <= 0.0) {
        otrap_val = sqrt(fabs(shape_de_raw + 2.0));
    }
    params[MAX_FORMULA_PARAMS - 1] = fabs(otrap_val * OTrapClrMul);

    double shape_de = shape_de_raw * Amplitude - h_axis;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
}

cpu_double3 gnarly3IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_GNARLY3IFS>(p, params, de);
}

constant FormulaInfo formula_info_gnarly3IFS = {
    .id = FORMULA_GNARLY3IFS,
    .name = "gnarly3IFS",
    .param_count = 13,
    .complexity = 312,
    .uses_transcendental = true
};

