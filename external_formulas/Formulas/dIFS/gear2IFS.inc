// gear2IFS: original SSE2 transform and x87 shell/tooth evaluation.
constexpr unsigned int FORMULA_GEAR2IFS = 9008;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Gear2Distance_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula_dIFS<FORMULA_GEAR2IFS>(cpu_double3 p, double* params,
                                                double& varyScale, double& rout) {
    const double scale = params[1];
    const int apply = int(params[6]);
    const double x = p.x * scale + params[4];
    const double y = p.y * scale + params[3];
    const double z = p.z * scale + params[2];
    params[DIFS_COMMIT_XYZ_SLOT] = apply != 0 ? 1.0 : 0.0;
    if (apply != 0) varyScale *= scale;
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, 0.0};
    Gear2Distance_x87(state, params, apply);
    rout = state[3];
#else
    // Preserve the radius Double spill and unordered x87 branch directions.
    const double radius = static_cast<double>(sqrt(static_cast<long double>(x)*x +
                                                  static_cast<long double>(y)*y) - params[5]);
    long double angle = atan2(static_cast<long double>(y), static_cast<long double>(x)) * params[12];
    if (!(radius >= 0.0)) angle -= params[10];
    angle /= params[7];
    angle = abs((angle - rint(angle)) * params[7]);
    long double distance = -((angle / params[0] - 1.0L) * params[0] * params[13]);
    const long double shell1 = abs(static_cast<long double>(radius)) - params[8];
    if (shell1 >= distance) distance = shell1;
    const long double shell2 = abs(static_cast<long double>(radius)) - params[11];
    if (!(shell2 >= distance)) distance = shell2;
    const long double slab = abs(static_cast<long double>(z)) - params[9];
    if (slab >= distance) distance = slab;
    if (apply == 0) distance /= scale;
    rout = static_cast<double>(distance);
#endif
    return cpu_double3(x, y, z);
}

template<>
cpu_double3 execute_formula<FORMULA_GEAR2IFS>(cpu_double3 p, double* params, double& de) {
    double varyScale = 1.0;
    return execute_formula_dIFS<FORMULA_GEAR2IFS>(p, params, varyScale, de);
}

cpu_double3 gear2IFS_formula(cpu_double3 p, double* params, double& de) {
    return execute_formula<FORMULA_GEAR2IFS>(p, params, de);
}
constant FormulaInfo formula_info_gear2IFS = {
    .id = FORMULA_GEAR2IFS, .name = "gear2IFS", .param_count = 12,
    .complexity = 540, .uses_transcendental = true
};
