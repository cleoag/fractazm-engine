// Shipped SSE2 tiling transform. CVTPD2DQ rounds to nearest-even (not truncation).
constexpr unsigned int FORMULA_TILINGIFS = 10242;

template<>
cpu_double3 execute_formula<FORMULA_TILINGIFS>(cpu_double3 p, constant double* params, double &de) {
    const int first = int(params[7]) & 1;
    const int third = 2 - 2 * first;
    double axes[] = {p.x, p.y, p.z};
    axes[third] -= params[0];
    for (int lane = 0; lane < 2; ++lane) {
        double cell = (axes[first + lane] - params[2 - lane]) / params[4 - lane];
        // CVTPD2DQ returns integer indefinite for NaN, infinity and overflow.
        const double nearest = std::nearbyint(cell);
        const int index = nearest >= -2147483648.0 && nearest <= 2147483647.0
                        ? static_cast<int>(nearest) : INT32_MIN;
        axes[first + lane] = (cell - double(index)) * params[4 - lane] + params[6 - lane];
    }
    return cpu_double3(axes[0], axes[1], axes[2]);
}
constant FormulaInfo formula_info_TilingIFS = {
    .id = FORMULA_TILINGIFS, .name = "TilingIFS", .param_count = 8,
    .complexity = 80, .uses_transcendental = false
};
