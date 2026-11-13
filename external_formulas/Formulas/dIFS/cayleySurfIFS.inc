// Cayley cubic surface, translated from the shipped 616-byte SSE2/x87 code.
constexpr unsigned int FORMULA_CAYLEYSURFIFS = 10307;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CayleySurface_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula<FORMULA_CAYLEYSURFIFS>(cpu_double3 p, constant double* params, double &de) {
    const double scale = params[1];
    const int apply = int(params[6]);
    cpu_double3 transformed(p.x * scale + params[4],
                            p.y * scale + params[3],
                            p.z * scale + params[2]);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {transformed.x, transformed.y, transformed.z, 0.0};
    CayleySurface_x87(state, params, apply);
    de = state[3];
#else
    const numeric::X87Scope extendedScope;
    // Matrix entries have already been expanded from .3SingleAngles by the loader.
    double v[3];
    for (int row = 0; row < 3; ++row) {
        const double* m = params + 7 + row * 3;
        v[row] = double((static_cast<numeric::X87Real>(transformed.x) * m[0]
                      + static_cast<numeric::X87Real>(transformed.y) * m[1])
                      + static_cast<numeric::X87Real>(transformed.z) * m[2]);
    }
    double positive = 0.0, negative = 0.0;
    auto accumulate = [](numeric::X87Real term, double& pos, double& neg) {
        // FTST's unordered branch is the same as its negative branch.
        if (term >= 0) pos = double(term + pos);
        else neg = double(neg - term);
    };
    const int cubic[6][2] = {{0,1},{0,2},{1,0},{1,2},{2,1},{2,0}};
    for (const auto& axes : cubic)
        accumulate((static_cast<numeric::X87Real>(v[axes[0]]) * v[axes[0]]) * v[axes[1]], positive, negative);
    positive = double(numeric::x87Abs(static_cast<numeric::X87Real>(positive) * params[5]));
    negative = double(numeric::x87Abs(static_cast<numeric::X87Real>(negative) * params[5]));
    const int quadratic[3][2] = {{0,1},{0,2},{1,2}};
    for (const auto& axes : quadratic) {
        numeric::X87Real term = static_cast<numeric::X87Real>(v[axes[0]]) * v[axes[1]];
        accumulate(term + term, negative, positive);
    }
    numeric::X87Real surface = (numeric::x87Sqrt(numeric::x87Sqrt(static_cast<numeric::X87Real>(positive)))
                         - numeric::x87Sqrt(numeric::x87Sqrt(static_cast<numeric::X87Real>(negative)))) * params[16];
    numeric::X87Real sphere = numeric::x87Sqrt(static_cast<numeric::X87Real>(v[0]) * v[0]
                        + (static_cast<numeric::X87Real>(v[1]) * v[1]
                         + static_cast<numeric::X87Real>(v[2]) * v[2])) - params[0];
    numeric::X87Real result = sphere >= surface ? sphere : surface;
    if (!apply) result /= scale;
    de = double(result);
#endif
    // Apply=0 transforms only the local shape query, leaving the orbit untouched.
    if (!apply) return p;
    params[DIFS_VARYSCALE_SLOT] *= scale;
    return transformed;
}

cpu_double3 cayleySurfIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_CAYLEYSURFIFS>(p, params, de);
}

constant FormulaInfo formula_info_cayleySurfIFS = {
    .id = FORMULA_CAYLEYSURFIFS,
    .name = "cayleySurfIFS",
    .param_count = 17,
    .complexity = 616,
    .uses_transcendental = false
};
