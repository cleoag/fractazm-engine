// Preserve original SSE2 writeback, x87 comparisons and Double temporaries.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void WeirdCubeIFSOriginal_x87(double*, const void*);
#endif
constexpr unsigned int FORMULA_WEIRDCUBEIFS = 10310;


template<>
cpu_double3 execute_formula<FORMULA_WEIRDCUBEIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
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
    for (int i=0;i<9;++i) {
        const float value=static_cast<float>(params[7+i]);
        std::memcpy(var-80-4*i,&value,4);
    }
    for (int i=0;i<6;++i) real(8*i,params[16+i]);
    double state[] = {p.x,p.y,p.z,params[DIFS_VARYSCALE_SLOT],0.};
    WeirdCubeIFSOriginal_x87(state,var);
    params[DIFS_VARYSCALE_SLOT]=state[3]; de=state[4];
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable numerical parity with x87 remains unverified.

    const double MaxRadius   = params[0];
    const double Scale       = params[1];    // .2Doubles — 1 raw value
    const double Zadd        = params[2];    // .2Doubles — 1 raw value
    const double Yadd        = params[3];
    const double Xadd        = params[4];
    const double ShapeModder = params[5];
    const int   ApplyScale  = int(params[6]);

    const double c25   = 25.0;
    const double c50   = 50.0;
    const double c_coeff1 = 0.520833333;  // ~= 1/1.92
    const double c_neg4   = -4.0;
    const double c_067    = 0.67;

    double x = p.x * Scale + Xadd;
    double y = p.y * Scale + Yadd;
    double z = p.z * Scale + Zadd;

    const cpu_double3 committed(x,y,z);
    if (ApplyScale != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    RotMatrix3 Rot = rotMatrix3FromFloats(params, 7);
    double rx = x * Rot.col0.x + y * Rot.col1.x + z * Rot.col2.x;
    double ry = x * Rot.col0.y + y * Rot.col1.y + z * Rot.col2.y;
    double rz = x * Rot.col0.z + y * Rot.col1.z + z * Rot.col2.z;
    x = rx; y = ry; z = rz;

    double x2 = x * x;  // x^2
    double y2 = y * y;  // y^2
    double z2 = z * z;  // z^2

    double xy = x * y;   // x*y
    double yz = y * z;   // y*z
    double xz = x * z;   // x*z

    double xy2 = xy * xy;  // (xy)^2 = x^2*y^2
    double yz2 = yz * yz;  // (yz)^2 = y^2*z^2
    double xz2 = xz * xz;  // (xz)^2 = x^2*z^2


    double pos_accum = c50 * (xy2 + yz2 + xz2);
    double neg_accum = 0.0;

    double term1 = x * x2 * (y + z) * c25;
    if (term1 < 0.0) neg_accum -= term1; else pos_accum += term1;

    double term2 = y * y2 * (x + z) * c25;
    if (term2 < 0.0) neg_accum -= term2; else pos_accum += term2;

    double term3 = z * z2 * (x + y) * c25;
    if (term3 < 0.0) neg_accum -= term3; else pos_accum += term3;

    double term4 = x * yz * ShapeModder;
    if (term4 < 0.0) neg_accum -= term4; else pos_accum += term4;




    double coeff_mix = c_coeff1 * ShapeModder;

    double term5 = (x2 * coeff_mix + 1.0) * yz * c_neg4;
    if (term5 < 0.0) neg_accum -= term5; else pos_accum += term5;

    double term6 = (y2 * coeff_mix + 1.0) * xz * c_neg4;
    if (term6 < 0.0) neg_accum -= term6; else pos_accum += term6;

    double term7 = (z2 * coeff_mix + 1.0) * xy * c_neg4;
    if (term7 < 0.0) neg_accum -= term7; else pos_accum += term7;

    double shape_de = sqrt(sqrt(pos_accum)) - sqrt(sqrt(neg_accum));
    shape_de *= c_067;

    double r = sqrt(x * x + y * y + z * z) - MaxRadius;

    if (r >= shape_de) {
        shape_de = r;
    }

    if (ApplyScale == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return ApplyScale != 0 ? committed : p;
#endif
}

cpu_double3 WeirdCubeIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_WEIRDCUBEIFS>(p, params, de);
}

constant FormulaInfo formula_info_WeirdCubeIFS = {
    .id = FORMULA_WEIRDCUBEIFS,
    .name = "WeirdCubeIFS",
    .param_count = 10,
    .complexity = 674,
    .uses_transcendental = false
};
