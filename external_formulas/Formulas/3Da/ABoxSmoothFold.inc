// Smooth box/ball fold. Windows x64 preserves the original x87 power
// sequence, extended intermediates, unordered branches and Double stores.
// Portable fallback below is not covered by the Windows parity evidence.
constexpr unsigned int FORMULA_ABOXSMOOTHFOLD = 10181;

static double fast_int_pow(double base, unsigned int exp) {
    if (exp <= 1u) return base;
    double result = 1.0;
    double b = base;
    while (exp > 0u) {
        if (exp & 1u) result *= b;
        exp >>= 1;
        if (exp > 0u) b *= b;
    }
    return result;
}

static double smooth_box_fold(double v, double Fold, int sharpness, double fix) {
    double av = abs(v);
    double xp = fix * fast_int_pow(av, sharpness);
    double result = (av + (2.0 * Fold - av) * xp) / (xp + 1.0);
    return std::signbit(v) ? -result : result;
}

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxSmoothFoldOriginal_x87(double* state, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXSMOOTHFOLD>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double scale=params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT]<=0.0) {
        scale=params[0];
        params[BFIRSTIT_SLOT]+=1.0;
    }
    double state[]={p.x,p.y,p.z,de,scale};
    ABoxSmoothFoldOriginal_x87(state,params);
    de=state[3];
    params[DIFS_VARYSCALE_SLOT]=state[4];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const auto power = [](E base, uint32_t exponent) {
        // Original 0016 returns its input for both exponent 0 and 1.
        if (exponent <= 1) return base;
        E result(1);
        while (exponent) {
            if (exponent & 1) result *= base;
            exponent >>= 1;
            if (exponent) base *= base;
        }
        return result;
    };
    const auto fold = [&](double coordinate) {
        const E a = abs(E(coordinate));
        const E xp = power(a, uint32_t(int(params[5]))) * E(params[6]);
        const E result = (a + (E(params[3])+E(params[3])-a)*xp) / (xp+E(1));
        return std::signbit(coordinate) ? -result : result;
    };
    const E x = fold(p.x), y = fold(p.y), z = fold(p.z);
    double previous = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0) {
        previous = params[0];
        params[BFIRSTIT_SLOT] += 1;
    }
    const double scale = double(E(previous) + (abs(E(previous))-E(1))*E(params[4]));
    params[DIFS_VARYSCALE_SLOT] = scale;
    E radius(1);
    if (!(params[2] >= params[9])) {
        const E middle = (E(1)+E(params[2]))*E(0.5);
        const E halfWidth = (E(1)-E(params[2]))*E(0.5);
        const E delta = (x*x+y*y+z*z)-middle;
        const E normalized = abs(delta)/halfWidth;
        const uint32_t sharpness = uint32_t(int(params[7]));
        const E rp = power((sharpness&1) ? sqrt(normalized) : normalized,
                           (sharpness&1) ? sharpness : sharpness/2) * E(params[8]);
        E offset = ((normalized+rp)/(E(1)+rp))*halfWidth;
        if (!(delta >= E(0))) offset = -offset;
        radius = offset+middle;
    }
    const E multiplier = E(scale)/radius;
    de = double(E(de)*multiplier);
    return cpu_double3(double(x*multiplier+E(params[10])),
                       double(y*multiplier+E(params[11])),
                       double(z*multiplier+E(params[12])));
#else
    const double Scale = params[0];       // Scale
    const double minR2 = params[2];       // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];        // Fold
    const double Scale_vary = params[4];  // Scale_vary
    const int Sharpness_box = int(params[5]); // original integer, including zero
    const double Fix_box = params[6];     // Fix (BoxFold)
    const int Sharpness_ball = int(params[7]); // original integer, including zero
    const double Fix_ball = params[8];    // Fix (BallFold)
    const double const_0_99 = 0.99;

    const double Cx = params[10];
    const double Cy = params[11];
    const double Cz = params[12];

    double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

    double x = smooth_box_fold(p.x, Fold, Sharpness_box, Fix_box);
    double y = smooth_box_fold(p.y, Fold, Sharpness_box, Fix_box);
    double z = smooth_box_fold(p.z, Fold, Sharpness_box, Fix_box);

    double r_eff;  // effective radius for sphere fold

    if (minR2 >= const_0_99) {
        r_eff = 1.0;
    } else {
        double rr = x * x + y * y + z * z;
        double m_half = (1.0 + minR2) * 0.5;  // center of transition
        double n_half = (1.0 - minR2) * 0.5;  // half-width of transition

        double r1_raw = rr - m_half;
        double rs = (r1_raw < 0.0) ? -1.0 : 1.0;
        double r1_abs_norm = abs(r1_raw) / n_half;

        double rp;
        if (Sharpness_ball & 1) {
            rp = fast_int_pow(sqrt(r1_abs_norm), Sharpness_ball) * Fix_ball;
        } else {
            rp = fast_int_pow(r1_abs_norm, static_cast<unsigned int>(Sharpness_ball) >> 1) * Fix_ball;
        }

        double ratio = (r1_abs_norm + rp) / (1.0 + rp);
        r_eff = m_half + rs * ratio * n_half;
    }

    double m_final = VaryScale / r_eff;

    de = de * m_final;

    cpu_double3 result;
    result.z = z * m_final + Cz;
    result.y = y * m_final + Cy;
    result.x = x * m_final + Cx;
    return result;
#endif
}

cpu_double3 ABoxSmoothFold_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ABOXSMOOTHFOLD>(p, params, de);
}

constant FormulaInfo formula_info_ABoxSmoothFold = {
    .id = FORMULA_ABOXSMOOTHFOLD,
    .name = "ABoxSmoothFold",
    .param_count = 8,
    .complexity = 950,
    .uses_transcendental = true
};
