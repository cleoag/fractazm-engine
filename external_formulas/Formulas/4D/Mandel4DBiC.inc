// Original bicomplex iteration: independent (x,y) and (z,w) squares.
// C is added internally before the final Double stores.
constexpr unsigned int FORMULA_MANDEL4DBIC = 2755;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Mandel4DBiCOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MANDEL4DBIC>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    Mandel4DBiCOriginal_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#else
    const numeric::X87Scope extendedScope;
    const numeric::X87Real x=p.x, y=p.y, z=p.z, w=de;
    const numeric::X87Real xy=x*y, zw=z*w;
    de=double((zw+zw+params[4])+params[0]);
    return cpu_double3(double(x*x-y*y+params[1]),
                       double(xy+xy+params[2]),double(z*z-w*w+params[3]));
#endif
}

constant FormulaInfo formula_info_Mandel4DBiC = {
    .id = FORMULA_MANDEL4DBIC,
    .name = "Mandel4DBiC",
    .param_count = 1, // W_add; engine appends Cx, Cy, Cz, Cw.
    .complexity = 93,
    .uses_transcendental = false
};
