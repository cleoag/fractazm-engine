// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_ABOXVARYSCALE2 = 10056;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxVaryScale2Original_x87(double*, const double*, int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXVARYSCALE2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT]};
    int first=int(params[BFIRSTIT_SLOT]);
    ABoxVaryScale2Original_x87(state,params,&first);
    de=state[3];params[DIFS_VARYSCALE_SLOT]=state[4];params[BFIRSTIT_SLOT]=first;
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E=numeric::Extended80;
    E previous(params[DIFS_VARYSCALE_SLOT]);
    if(params[BFIRSTIT_SLOT]<=0) {previous=E(params[0]);params[BFIRSTIT_SLOT]+=1;}
    const double vary=static_cast<double>(previous+(abs(previous)-E(1))*E(params[4]));
    params[DIFS_VARYSCALE_SLOT]=vary;
    const E fold(params[3]),twice=fold+fold;
    auto folded=[&](E v) {return v>=fold ? -v+twice : (!(v>=-fold) ? -v-twice : v);};
    const E x=folded(E(p.x)),y=folded(E(p.y)),z=folded(E(p.z));
    auto power=[&](E v) {return numeric::extendedLogPower(abs(v),E(params[6]));};
    const E rr=numeric::extendedLogPower(abs(power(z)+power(y)+power(x)),E(params[5]));
    const E scale=!(rr>=E(params[2])) ? E(params[1]) : (rr<=E(1) ? E(vary)/rr : E(vary));
    de=static_cast<double>(E(de)*scale);
    return cpu_double3(static_cast<double>(x*scale+E(params[7])),
                       static_cast<double>(y*scale+E(params[8])),
                       static_cast<double>(z*scale+E(params[9])));
#else
    // Portable arithmetic remains uncertified.
    const double Scale = params[0];
    const double ScaleOverMinR2 = params[1]; // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];           // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];
    const double Scale_vary = params[4];
    const double R_power = params[5];
    const double XYZ_power = params[6];
    const double TwoFold = 2.0 * Fold;

    double VaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        VaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    VaryScale = VaryScale + Scale_vary * (abs(VaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;

    double x = p.x;
    if (x >= Fold) {
        x = TwoFold - x;
    } else if (x <= -Fold) {
        x = -TwoFold - x;
    }

    double y = p.y;
    if (y >= Fold) {
        y = TwoFold - y;
    } else if (y <= -Fold) {
        y = -TwoFold - y;
    }

    double z = p.z;
    if (z >= Fold) {
        z = TwoFold - z;
    } else if (z <= -Fold) {
        z = -TwoFold - z;
    }

    double rr_sum = x87_log_power(abs(x), XYZ_power)
                 + x87_log_power(abs(y), XYZ_power)
                 + x87_log_power(abs(z), XYZ_power);
    double rr = x87_log_power(rr_sum, R_power);

    double m;
    if (!(rr >= minR2)) {
        m = ScaleOverMinR2;  // uses initial Scale (from BOXSCALE precomputation)
    } else if (rr <= 1.0) {
        m = VaryScale / rr;
    } else {
        m = VaryScale;
    }

    de = de * m;

    const double J1 = params[7];
    const double J2 = params[8];
    const double J3 = params[9];

    cpu_double3 result;
    result.x = x * m + J1;
    result.y = y * m + J2;
    result.z = z * m + J3;
    return result;
#endif
}

cpu_double3 ABoxVaryScale2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ABOXVARYSCALE2>(p, params, de);
}

constant FormulaInfo formula_info_ABoxVaryScale2 = {
    .id = FORMULA_ABOXVARYSCALE2,
    .name = "ABoxVaryScale2",
    .param_count = 6,
    .complexity = 950,
    .uses_transcendental = true
};

