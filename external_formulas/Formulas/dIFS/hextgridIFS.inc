// Original hexagonal tube grid; local rotation does not alter orbit coordinates.
constexpr unsigned int FORMULA_HEXTGRIDIFS = 10232;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HexTubeGrid_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula_dIFS<FORMULA_HEXTGRIDIFS>(cpu_double3 p, double* params,
                                                    double& varyScale, double& rout) {
    const double scale = params[1];
    const int apply = int(params[6]);
    cpu_double3 q(p.x * scale + params[4], p.y * scale + params[3],
                  p.z * scale + params[2]);
    params[DIFS_COMMIT_XYZ_SLOT] = apply != 0 ? 1.0 : 0.0;
    if (apply != 0) varyScale *= scale;
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {q.x, q.y, q.z, 0.0};
    HexTubeGrid_x87(state, params, apply);
    rout = state[3];
#else
    double v[3];
    for (int row = 0; row < 3; ++row) {
        const double* m = params + 7 + row * 3;
        v[row] = double((static_cast<long double>(q.x) * m[0]
                      + static_cast<long double>(q.y) * m[1])
                      + static_cast<long double>(q.z) * m[2]);
    }
    const long double x=v[0], y=v[1], z=v[2];
    double positive=double(abs(z*z*params[16]+params[0])), negative=0.0;
    auto accumulate = [&](long double term) {
        if (term >= 0.0L) positive = double(term + positive);
        else negative = double(negative - term);
    };
    accumulate(cos(y * params[18]));
    accumulate(cos(x * params[20] - y * params[19]));
    accumulate(cos(x * params[20] + y * params[19]));
    accumulate(cos(x * params[21]) * params[5]);
    accumulate(cos(y * params[17] - x * params[20]) * params[5]);
    accumulate(cos(x * params[20] + y * params[17]) * params[5]);
    long double distance = sqrt(static_cast<long double>(positive))
                         - sqrt(static_cast<long double>(negative));
    if (!apply) distance /= scale;
    rout = double(distance);
#endif
    return q;
}

template<>
cpu_double3 execute_formula<FORMULA_HEXTGRIDIFS>(cpu_double3 p, double* params, double& de) {
    double varyScale = 1.0;
    return execute_formula_dIFS<FORMULA_HEXTGRIDIFS>(p, params, varyScale, de);
}
cpu_double3 hextgridIFS_formula(cpu_double3 p, double* params, double& de) {
    return execute_formula<FORMULA_HEXTGRIDIFS>(p, params, de);
}
constant FormulaInfo formula_info_hextgridIFS = {
    .id = FORMULA_HEXTGRIDIFS, .name = "hextgridIFS", .param_count = 10,
    .complexity = 483, .uses_transcendental = true
};
