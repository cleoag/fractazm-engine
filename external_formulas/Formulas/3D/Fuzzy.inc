// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_FUZZY = 9885;


template<> struct ExternalFormulaReadsRout<FORMULA_FUZZY> { static constexpr bool value = true; };
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void FuzzyOriginal_x87(double*, const double*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_FUZZY>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,params[FORMULA_ROUT_SLOT]};
    const int iteration=int(params[ITER_COUNT_SLOT]);
    FuzzyOriginal_x87(state,params,&iteration);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    const E n=(E(1)+E(int32_t(params[ITER_COUNT_SLOT])))*params[0];
    const E radius=params[FORMULA_ROUT_SLOT], v=E(p.z)/sqrt(radius);
    const E phi=extendedAtan2(v,sqrt(E(1)-v*v))*n;
    const E theta=extendedAtan2(E(p.y),E(p.x))*n;
    const auto ph=extendedSinCos(phi), th=extendedSinCos(theta);
    auto rangeFailure = [](E a) {
        return (a.signExponent()&0x7fff) != 0x7fff && abs(a) >= E(0x1p63);
    };
    if (rangeFailure(phi) || rangeFailure(theta)) {
        // Missing FSINCOS pushes shift the radius operand. XYZ's remaining
        // stack values are observed independently by the three final stores.
        const E base=rangeFailure(phi) ? theta : ph.cos;
        const E rp=extendedLogPower(base,E(params[0])-1);
        return cpu_double3(double(extendedNaN()),double(extendedNaN()),
                           double(E(params[4])-radius*rp));
    }
    const E rp=extendedLogPower(radius,E(params[0])-1), radial=ph.cos*rp;
    return cpu_double3(double(th.cos*radial+params[2]),
                       double((th.sin*radial)*params[1]+params[3]),
                       double(E(params[4])-ph.sin*rp));
#else
    // Portable arithmetic remains uncertified.
    const double Power        = params[0];  // [edi-0x10] (default: 8)
    const double Y_multiplier = params[1];  // [edi-0x18] (default: -1)
    const double iteration    = params[ITER_COUNT_SLOT];  // [esi+0x40] iteration count
    const double Cx           = params[2];  // from engine context
    const double Cy           = params[3];  // from engine context
    const double Cz           = params[4];  // from engine context

    double x = p.x, y = p.y, z = p.z;

    double N = (1.0 + iteration) * Power;

    double r_sq = params[FORMULA_ROUT_SLOT];
    double r = sqrt(r_sq);

    double cos_elev = z / r;
    double sin_elev = sqrt(1.0 - cos_elev * cos_elev);
    double phi = atan2(cos_elev, sin_elev);

    double theta = atan2(y, x);

    double phi_N = phi * N;
    double theta_N = theta * N;

    double R = pow(r_sq, Power - 1.0);

    double cos_phi_N = cos(phi_N);
    double sin_phi_N = sin(phi_N);
    double cos_theta_N = cos(theta_N);
    double sin_theta_N = sin(theta_N);

    double x_out = R * cos_phi_N * cos_theta_N + Cx;
    double y_out = R * cos_phi_N * sin_theta_N * Y_multiplier + Cy;
    double z_out = Cz - R * sin_phi_N;

    return cpu_double3(x_out, y_out, z_out);
#endif
}

constant FormulaInfo formula_info_Fuzzy = {
    .id = FORMULA_FUZZY,
    .name = "Fuzzy",
    .param_count = 6,
    .complexity = 129,
    .uses_transcendental = true
};

