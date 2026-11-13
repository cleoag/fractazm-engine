// Preserve original x87 branches, arithmetic and Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void lidinoidIFSOriginal_x87(double*, const void*);
#endif
constexpr unsigned int FORMULA_LIDINOIDIFS = 5011;

template<>
cpu_double3 execute_formula<FORMULA_LIDINOIDIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    alignas(16) unsigned char buffer[160]{};
    auto* var = buffer + 128;
    const auto real = [&](int offset, double value) { std::memcpy(var + offset, &value, 8); };
    real(-16,params[0]); real(-32,params[1]); real(-24,params[1]);
    real(-48,params[2]); real(-40,params[2]); real(-56,params[3]); real(-64,params[4]);
    real(-72,params[5]);
    const int32_t apply=static_cast<int32_t>(params[6]);
    std::memcpy(var-76,&apply,4);
    for (int i=0;i<9;++i) {
        const float value=static_cast<float>(params[7+i]);
        std::memcpy(var-80-4*i,&value,4);
    }
    real(-120,params[16]); real(0,0.5); real(8,0.15);
    double state[] = {p.x,p.y,p.z,params[DIFS_VARYSCALE_SLOT],0.,0.};
    lidinoidIFSOriginal_x87(state,var);
    params[DIFS_VARYSCALE_SLOT]=state[3]; de=state[4];
    params[MAX_FORMULA_PARAMS-1]=state[5];
    if (apply != 0) params[DIFS_COMMIT_XYZ_SLOT]=1.;
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable numerical parity remains unverified.

    const double Size = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Density = params[5];
    const int Apply_scale_add = int(params[6]);
    const double TrapColor_Density = params[16];

    const double K_HALF = 0.5;
    const double K_OFFSET = 0.15;

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

    double rx = params[7]  * x + params[8]  * y + params[9]  * z;
    double ry = params[10] * x + params[11] * y + params[12] * z;
    double rz = params[13] * x + params[14] * y + params[15] * z;

    double D = Density;
    double sxD, cxD, syD, cyD, szD, czD;
    sxD = sincos(rx * D, cxD);
    syD = sincos(ry * D, cyD);
    szD = sincos(rz * D, czD);

    double sx, cx, sy, cy, sz, cz;
    sx = sincos(rx, cx);
    sy = sincos(ry, cy);
    sz = sincos(rz, cz);

    double term1 = sy * (cx * sxD);    // sin(ry) * cos(rx) * sin(rx*D)
    double term2 = cz * (sx * szD);    // cos(rz) * sin(rx) * sin(rz*D)
    double term3 = sz * (cy * syD);    // sin(rz) * cos(ry) * sin(ry*D)

    params[MAX_FORMULA_PARAMS - 1] =
        fabs((term1 - term2 - term3) * TrapColor_Density);

    double Part1 = term1 + term2 + term3;

    double Part2 = cxD * cyD + (cyD * czD + czD * cxD);

    double lidinoid_value = K_HALF * (Part1 - Part2) + K_OFFSET;

    double cube_dist = abs(rz);
    if (abs(ry) >= cube_dist) cube_dist = abs(ry);
    if (abs(rx) >= cube_dist) cube_dist = abs(rx);
    cube_dist -= Size;

    double shape_de = (cube_dist >= lidinoid_value ? cube_dist : lidinoid_value) / Density;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(state_x, state_y, state_z);
#endif
}

cpu_double3 lidinoidIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_LIDINOIDIFS>(p, params, de);
}

constant FormulaInfo formula_info_lidinoidIFS = {
    .id = FORMULA_LIDINOIDIFS,
    .name = "lidinoidIFS",
    .param_count = 17,
    .complexity = 980,
    .uses_transcendental = true
};
