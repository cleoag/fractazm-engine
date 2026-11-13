// Original operation order, comparison flags and intermediate Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void foldingoctIFSOriginal_x87(double*, const void*);
#endif
constexpr unsigned int FORMULA_FOLDINGOCTIFS = 10248;

template<>
cpu_double3 execute_formula<FORMULA_FOLDINGOCTIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    alignas(16) unsigned char buffer[160]{};
    auto* var = buffer + 144; // SSE2 packed memory operands require 16-byte alignment.
    const auto real = [&](int offset, double v) { std::memcpy(var + offset, &v, 8); };
    const auto integer = [&](int offset, double v) {
        const int32_t i = static_cast<int32_t>(v); std::memcpy(var + offset, &i, 4);
    };
    const auto matrix = [&](int offset, int first) {
        for (int j=0;j<9;++j) { const float v=static_cast<float>(params[first+j]); std::memcpy(var+offset-4*j,&v,4); }
    };
    for (int j=0;j<3;++j) real(-16-8*j,params[j]);
    matrix(-36,3);
    for (int j=0;j<3;++j) integer(-72-4*j,params[12+j]);
    double state[] = {p.x,p.y,p.z};
    foldingoctIFSOriginal_x87(state,var);
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable arithmetic has not yet been certified against original x87 CODE.
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const int CtrlX = int(params[12]);   // AbsX/Do1stSwap 0...3
    const int CtrlY = int(params[13]);   // AbsY/Do2ndSwap 0...3
    const int CtrlZ = int(params[14]);   // AbsZ/Do3rdSwap 0...3


    double z_val = p.z + Add_Z;
    double y_val = p.y + Add_Y;
    double x_val = p.x + Add_X;

    cpu_double3 rv = R * cpu_double3(x_val, y_val, z_val);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    if ((CtrlX & 1) != 0) rx = abs(rx);
    if ((CtrlY & 1) != 0) ry = abs(ry);
    if ((CtrlZ & 1) != 0) rz = abs(rz);


    if ((CtrlX & 2) != 0) {
        if (!(rx > ry)) { double t = rx; rx = ry; ry = t; }
    }

    if ((CtrlY & 2) != 0) {
        if (!(rx > rz)) { double t = rx; rx = rz; rz = t; }
    }

    { double t = rx; rx = ry; ry = t; }

    if ((CtrlZ & 2) != 0) {
        if (!(rx > rz)) { double t = rx; rx = rz; rz = t; }
    }

    return cpu_double3(rx, ry, rz);
#endif
}

cpu_double3 foldingoctIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_FOLDINGOCTIFS>(p, params, de);
}

constant FormulaInfo formula_info_foldingoctIFS = {
    .id = FORMULA_FOLDINGOCTIFS,
    .name = "foldingoctIFS",
    .param_count = 9,
    .complexity = 640,
    .uses_transcendental = true
};

