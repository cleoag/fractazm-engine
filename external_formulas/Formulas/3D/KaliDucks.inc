// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_KALIDUCKS = 7038;
template<> struct ExternalFormulaReadsRout<FORMULA_KALIDUCKS> { static constexpr bool value = true; };

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void KaliDucksOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_KALIDUCKS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,params[FORMULA_ROUT_SLOT]};
    KaliDucksOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable arithmetic remains uncertified.
    const double radius=params[FORMULA_ROUT_SLOT];
    const double factor=params[1]>radius ? 1.0/(params[1]*params[1]) : params[0]/radius;
    return cpu_double3(abs(p.x)*factor+params[2],abs(p.y)*factor+params[3],abs(p.z)*factor+params[4]);
#endif
}
constant FormulaInfo formula_info_KaliDucks = {
    .id = FORMULA_KALIDUCKS,
    .name = "KaliDucks",
    .param_count = 2,
    .complexity = 85,
    .uses_transcendental = false
};

