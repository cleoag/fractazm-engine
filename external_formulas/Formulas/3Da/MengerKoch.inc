// Original operation order, comparison flags and intermediate Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MengerKochOriginal_x87(double*, const void*);
#endif
constexpr unsigned int FORMULA_MENGERKOCH = 10033;


template<>
cpu_double3 execute_formula<FORMULA_MENGERKOCH>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    alignas(16) unsigned char buffer[160]{};
    auto* var = buffer + 144; // SSE2 packed memory operands require 16-byte alignment.
    const auto real = [&](int offset, double v) { std::memcpy(var + offset, &v, 8); };
    const auto matrix = [&](int offset, int first) {
        for (int j=0;j<9;++j) { const float v=static_cast<float>(params[first+j]); std::memcpy(var+offset-4*j,&v,4); }
    };
    real(-8, .5);
    for (int j=0;j<4;++j) real(-16-8*j,params[j]);
    matrix(-44,4); matrix(-80,13);
    real(-120,params[22]); real(-128,params[23]);
    double state[] = {p.x,p.y,p.z,de};
    MengerKochOriginal_x87(state,var);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable arithmetic has not yet been certified against original x87 CODE.
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    const double Edge1 = params[22];
    const double Edge2 = params[23];

    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    cpu_double3 r1v = R1 * cpu_double3(ax, ay, az);
    double rx = r1v.x;
    double ry = r1v.y;
    double rz = r1v.z;

    double max_val = rx;
    double mid_val = ry;
    double min_val = rz;
    if (!(max_val >= mid_val)) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (!(max_val >= min_val)) { double t = max_val; max_val = min_val; min_val = t; }
    if (!(mid_val >= min_val)) { double t = mid_val; mid_val = min_val; min_val = t; }

    double edge_mid = abs(mid_val - Edge2);
    double edge_max = abs(Edge1 - max_val);

    cpu_double3 r2v = R2 * cpu_double3(edge_max, edge_mid, min_val);
    double r2x = r2v.x;  // Row0
    double r2y = r2v.y;  // Row1
    double r2z = r2v.z;  // Row2

    de = de * Scale;
    double xs = r2x * Scale;
    double ys = r2y * Scale;
    double zs = r2z * Scale;

    double k = Scale - 1.0;
    double x_out = xs - k * CScaleX;
    double y_out = ys - k * CScaleY;

    double kCZ = k * CScaleZ;
    double threshold = 0.5 * kCZ;
    double z_out;
    if (!(threshold > zs)) {
        z_out = zs - kCZ;
    } else {
        z_out = zs;
    }

    return cpu_double3(x_out, y_out, z_out);
#endif
}

constant FormulaInfo formula_info_MengerKoch = {
    .id = FORMULA_MENGERKOCH,
    .name = "MengerKoch",
    .param_count = 12,
    .complexity = 960,
    .uses_transcendental = true
};

