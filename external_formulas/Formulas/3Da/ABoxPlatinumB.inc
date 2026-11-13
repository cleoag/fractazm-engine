// ABoxPlatinumB: shipped 574-byte approximated rotated box, DEoption 2.
// Expanded parameters: Scale, Scale/MinR^2, MinR^2, Fold, nine Single
// matrix entries, center XYZ, inverse radius, four sin/cos pairs, mask, CXYZ.
constexpr unsigned int FORMULA_ABOXPLATINUMB = 10050;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxPlatinumBOriginal_x87(double* state, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXPLATINUMB>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, de};
    ABoxPlatinumBOriginal_x87(state, params);
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const E fold(params[3]), px(p.x), py(p.y), pz(p.z);
    auto box = [&](E v) { return abs(v+fold) - (abs(v-fold)+v); };
    // 0025/003A/005B and 00A2/00B7/00E4 are Double scratch stores.
    // The folded component and each inverse rotation stay extended.
    const E ax(static_cast<double>(px*E(params[20]) - pz*E(params[19])));
    const E az(static_cast<double>(px*E(params[19]) + pz*E(params[20])));
    const E ay(static_cast<double>(ax*E(params[17]) + py*E(params[18])));
    E x = (box(ax*E(params[18])-py*E(params[17]))*E(params[18])
           - ay*E(params[17]))*E(params[20]) - az*E(params[19]);
    const E by(static_cast<double>(py*E(params[24]) - pz*E(params[23])));
    const E bz(static_cast<double>(py*E(params[23]) + pz*E(params[24])));
    const E bx(static_cast<double>(by*E(params[21]) + px*E(params[22])));
    E y = (box(by*E(params[22])-px*E(params[21]))*E(params[22])
           - bx*E(params[21]))*E(params[24]) - bz*E(params[23]);
    E z = box(pz);
    const int mask = int(params[25]);
    if (mask & 4) z = abs(z);
    if (mask & 2) y = abs(y);
    if (mask & 1) x = abs(x);
    z += E(params[15]); y += E(params[14]); x += E(params[13]);
    const E rr = ((z*z + y*y) + x*x)*E(params[16]);
    const E scale(params[0]);
    const E m = !(rr >= E(params[2])) ? E(params[1])
        : (E(1) >= rr ? scale/rr : scale);
    de = static_cast<double>(E(de)*m);
    z = z*m - E(params[15])*scale;
    y = y*m - E(params[14])*scale;
    x = x*m - E(params[13])*scale;
    // 01E4..022F loads the matrix as Single and stores only final XYZ.
    E matrix[9];
    for (int i=0; i<9; ++i) matrix[i] = E(static_cast<float>(params[4+i]));
    return cpu_double3(
        static_cast<double>((x*matrix[0]+y*matrix[1])+z*matrix[2]+E(params[26])),
        static_cast<double>((x*matrix[3]+y*matrix[4])+z*matrix[5]+E(params[27])),
        static_cast<double>((x*matrix[6]+y*matrix[7])+z*matrix[8]+E(params[28])));
#else
    const double fold = params[3];
    auto box = [fold](double v) { return fabs(v + fold) - (fabs(v - fold) + v); };
    // The second angle pair is applied first, as in the original code.
    const double ax = p.x*params[20] - p.z*params[19];
    const double az = p.x*params[19] + p.z*params[20];
    const double ay = ax*params[17] + p.y*params[18];
    double x = (box(ax*params[18] - p.y*params[17])*params[18] - ay*params[17])*params[20] - az*params[19];
    const double by = p.y*params[24] - p.z*params[23];
    const double bz = p.y*params[23] + p.z*params[24];
    const double bx = by*params[21] + p.x*params[22];
    double y = (box(by*params[22] - p.x*params[21])*params[22] - bx*params[21])*params[24] - bz*params[23];
    double z = box(p.z);
    const int mask = (int)params[25];
    if (mask & 4) z = fabs(z);
    if (mask & 2) y = fabs(y);
    if (mask & 1) x = fabs(x);
    z += params[15]; y += params[14]; x += params[13];
    const double rr = ((z*z + y*y) + x*x)*params[16];
    const double m = !(rr >= params[2]) ? params[1] : rr > 1. ? params[0] : params[0]/rr;
    de *= m;
    z = z*m - params[15]*params[0];
    y = y*m - params[14]*params[0];
    x = x*m - params[13]*params[0];
    return cpu_double3((x*params[4] + y*params[5]) + z*params[6] + params[26],
                       (x*params[7] + y*params[8]) + z*params[9] + params[27],
                       (x*params[10] + y*params[11]) + z*params[12] + params[28]);
#endif
}
constant FormulaInfo formula_info_ABoxPlatinumB = {
    .id = FORMULA_ABOXPLATINUMB, .name = "ABoxPlatinumB", .param_count = 15,
    .complexity = 574, .uses_transcendental = false
};
