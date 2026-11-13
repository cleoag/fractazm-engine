// tilingbrick2IFS: add/rotate, nearest-even Y cell, then X fold.
// Preserve original Single matrix expansion, Double stores and invalid FISTP.
constexpr unsigned int FORMULA_TILINGBRICK2IFS = 10245;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void TilingBrick2_x87(double* state, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA_TILINGBRICK2IFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z};
    TilingBrick2_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    const numeric::X87Scope extendedScope;
    const numeric::X87Real x = (numeric::X87Real)p.x + params[0];
    const numeric::X87Real y = (numeric::X87Real)p.y + params[1];
    const numeric::X87Real z = (numeric::X87Real)p.z + params[2];
    const double rx = double(x*params[3]+y*params[4]+z*params[5]);
    const double ry = double(x*params[6]+y*params[7]+z*params[8]);
    const double rz = double(x*params[9]+y*params[10]+z*params[11]);
    const numeric::X87Real rounded = numeric::x87Round((numeric::X87Real)ry/params[13]);
    const int iy = rounded >= -numeric::X87Real(2147483648.) && rounded <= numeric::X87Real(2147483647.)
                 ? int(rounded) : INT32_MIN;
    const numeric::X87Real fy = (numeric::X87Real)ry - (numeric::X87Real)iy*params[13];
    const numeric::X87Real shifted = (numeric::X87Real)rx + (numeric::X87Real)params[14]*iy;
    const numeric::X87Real fx = shifted - numeric::x87Round(shifted/params[12])*params[12];
    return cpu_double3(double(fx),double(fy),rz);
#endif
}

constant FormulaInfo formula_info_tilingbrick2IFS = {
    .id = FORMULA_TILINGBRICK2IFS, .name = "tilingbrick2IFS", .param_count = 9,
    .complexity = 147, .uses_transcendental = false
};
