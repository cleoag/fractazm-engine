// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA__FOLDING2 = 6002;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Folding2Original_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__FOLDING2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,0.};
    Folding2Original_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable arithmetic remains uncertified.
    const double fold=params[0], twice=params[1];
    return cpu_double3(p.x+abs(p.x-twice)-abs(p.x-fold)-fold,
                       p.y+abs(p.y-twice)-abs(p.y-fold)-fold,
                       p.z+abs(p.z-twice)-abs(p.z-fold)-fold);
#endif
}
constant FormulaInfo formula_info__Folding2 = {
    .id = FORMULA__FOLDING2,
    .name = "_Folding2",
    .param_count = 1,
    .complexity = 310,
    .uses_transcendental = false
};

