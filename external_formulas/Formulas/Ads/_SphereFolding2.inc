// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA__SPHEREFOLDING2 = 7013;
template<> struct ExternalFormulaReadsRout<FORMULA__SPHEREFOLDING2> { static constexpr bool value = true; };

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void SphereFolding2Original_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__SPHEREFOLDING2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,params[FORMULA_ROUT_SLOT]};
    SphereFolding2Original_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable arithmetic remains uncertified.
    const double radius=params[FORMULA_ROUT_SLOT];
    if (!(radius>=params[0])) {
        const double r=sqrt(radius);
        return p*((params[1]-r)/r);
    }
    return p;
#endif
}
constant FormulaInfo formula_info__SphereFolding2 = {
    .id = FORMULA__SPHEREFOLDING2,
    .name = "_SphereFolding2",
    .param_count = 1,
    .complexity = 85,
    .uses_transcendental = false
};

