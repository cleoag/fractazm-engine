// Preserve original SSE2 writeback, x87 comparisons and Double temporaries.
constexpr unsigned int FORMULA_KNOTS2IFS = 5059;

template<>
cpu_double3 execute_formula<FORMULA_KNOTS2IFS>(cpu_double3 p, constant double* params, double &de) {
    FormulaWorkBudget work("Knots2IFS");
#if defined(_MSC_VER) && defined(_M_X64) && !defined(MB3D_FORCE_PORTABLE_FORMULA_LOOPS)
    alignas(16) unsigned char buffer[176]{};
    auto* var = buffer + 128;
    const auto real = [&](int offset, double value) { std::memcpy(var + offset, &value, 8); };
    const auto integer = [&](int offset, double value) {
        const int32_t flag = static_cast<int32_t>(value);
        std::memcpy(var + offset, &flag, 4);
    };
    real(-16,params[0]); real(-32,params[1]); real(-24,params[1]);
    real(-48,params[2]); real(-40,params[2]); real(-56,params[3]); real(-64,params[4]);
    real(-72,params[5]); integer(-76,params[6]);
    integer(-80,params[7]); integer(-84,params[8]); integer(-88,params[9]);
    real(-96,params[10]); integer(-100,params[11]);
    for (int i=0;i<4;++i) real(8*i,params[12+i]);
    double state[] = {p.x,p.y,p.z,params[DIFS_VARYSCALE_SLOT],0.};
    work.checkNativeStatus(Knots2IFSOriginal_x87(state,var,work.nativeCounter()));
    params[DIFS_VARYSCALE_SLOT]=state[3]; de=state[4];
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable numerical parity with x87 remains unverified.

    const double Radius_1 = params[0];
    const double Scale = params[1];   // .2Doubles — 1 raw value
    const double Z_add = params[2];   // .2Doubles — 1 raw value
    const double Y_add = params[3];   // [edi-0x38]
    const double X_add = params[4];   // [edi-0x40]
    const double Radius_2 = params[5];   // [edi-0x48]
    const int Apply_scale_add = int(params[6]);  // [edi-0x4c]
    const int Order = int(params[7]);            // [edi-0x50]
    const int Mobius_X = int(params[8]);         // [edi-0x54]
    const int Mobius_Y = int(params[9]);         // [edi-0x58]
    const double Radius_3 = params[10];           // [edi-0x60]
    const int Change_this_to_close = int(params[11]); // [edi-0x64]
    const double two_pi = params[13];   // [edi+0x08]
    const double huge_min = params[14]; // [edi+0x10]

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    double theta = atan2(y, x);
    double r_xy = sqrt(x * x + y * y);

    double min_dist = huge_min;

    double theta_shifted = theta;
    int count = Change_this_to_close;

    for (;;) {
        work.beforeStep();
        theta_shifted += two_pi;

        double t = theta_shifted / double(Order);

        double dx = r_xy + Radius_3 * cos(double(Mobius_X) * t) - Radius_1;

        double dz_val = z + Radius_3 * sin(double(Mobius_Y) * t) - Radius_1;

        double dist = sqrt(dx * dx + dz_val * dz_val);

        if (!(dist >= min_dist)) min_dist = dist;

        // DEC/JG runs once for nonpositive counts, including INT_MIN.
        if (count <= 1) break;
        --count;
    }

    double shape_de = min_dist - Radius_2;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(state_x, state_y, state_z);
#endif
}

cpu_double3 Knots2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_KNOTS2IFS>(p, params, de);
}

constant FormulaInfo formula_info_Knots2IFS = {
    .id = FORMULA_KNOTS2IFS,
    .name = "Knots2IFS",
    .param_count = 12,
    .complexity = 500,
    .uses_transcendental = true
};
