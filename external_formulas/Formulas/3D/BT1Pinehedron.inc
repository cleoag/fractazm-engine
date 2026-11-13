// BT1Pinehedron: literal control flow from the shipped 493-byte x86 program.
// PolyIterAfter and Sides/Sides2 are declared in metadata but never read by code.
constexpr unsigned int FORMULA_BT1PINEHEDRON = 10267;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BT1PinehedronT1_x87(double* state);
#endif

template<>
cpu_double3 execute_formula<FORMULA_BT1PINEHEDRON>(cpu_double3 p, constant double* params, double &de) {
    const double scale = params[0], offset = params[1];
    const double s23 = params[13], s13 = params[14], s12 = params[15];
    // MOV AL / DEC AL: zero means 256 iterations, other integers use low byte.
    const unsigned countByte = static_cast<unsigned>(static_cast<int>(params[8])) & 255u;
    const unsigned count = countByte ? countByte : 256u;
    const unsigned iteration = static_cast<unsigned>(static_cast<int>(params[ITER_COUNT_SLOT])) & 255u;
    auto lowByte = [&](int index) { return static_cast<unsigned>(static_cast<int>(params[index])) & 255u; };
    auto signedByte = [](unsigned value) { return value < 128u ? int(value) : int(value)-256; };
    double x = p.x, y = p.y, z = p.z;
    for (unsigned i = 0; i < count; ++i) {
#if defined(_MSC_VER) && defined(_M_X64)
        double state[] = {x,y,z,s23,s13,s12,scale,offset};
        BT1PinehedronT1_x87(state);
        x = state[0]; y = state[1]; z = state[2];
#elif defined(MB3D_SOFTWARE_EXT80)
        external_pine_t1(x,y,z,s23,s13,s12,scale,offset);
#else
        // Preserve the explicit Double stores and x87 grouping where the host
        // has extended long double. Other arithmetic targets need verification.
        const double tx = ((long double)x*s23 - (long double)z*s13)*s12;
        const double nz = std::abs((long double)x*s13 + (long double)z*s23);
        const double nx = std::abs((long double)tx - (long double)y*s12);
        const double ny = std::abs((long double)tx + (long double)y*s12);
        const double tx2 = ((long double)nx + ny)*s12;
        y = ((long double)ny - nx)*s12;
        x = (long double)tx2*s23 + (long double)nz*s13;
        z = -(long double)tx2*s13 + (long double)nz*s23;
        x = (long double)x*scale - offset;
        y *= scale; z *= scale;
#endif
    }
    // 00B8..00C6: byte equality followed by signed JG.
    if (iteration == lowByte(6) || signedByte(iteration) >= signedByte(lowByte(7))) {
        const double c = cos(params[5]), s = sin(params[5]);
        const double nz = z*s + y*c;
        y = y*s - z*c;
        z = nz;
    }
    // 00F5..00FB: byte equality only; no PolyIterAfter condition.
    if (iteration == lowByte(9)) {
        long double ratio = (long double)y*y / ((long double)z*z);
        if (ratio >= 1) ratio = 1 / ratio;
        const long double f1 = 1 / std::sqrt(1 + ratio);
        const long double fy = y*f1, fz = z*f1;
        const double fy2 = fy*fy;
        ratio = (fz*fz + fy2) / ((long double)x*x);
        if (ratio >= 1) ratio = 1 / ratio;
        const double f2 = 1 / std::sqrt(1 + ratio);
        x *= f2; y = double(fy)*f2; z = double(fz)*f2;
    }
    const double xx = x*x, yy = y*y, zz = z*z;
    const double t = 2*x / sqrt(yy+zz);
    return cpu_double3(xx-yy-zz + params[17]*params[2],
                       2*t*z*y + params[18]*params[4],
                       t*(yy-zz) + params[19]*params[3]);
}

constant FormulaInfo formula_info_BT1Pinehedron = {
    .id = FORMULA_BT1PINEHEDRON,
    .name = "BT1Pinehedron",
    .param_count = 17,
    .complexity = 493,
    .uses_transcendental = true
};
