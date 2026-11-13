// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_KALISETS1 = 7056;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Kalisets1Original_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_KALISETS1>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,0.};
    Kalisets1Original_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    // Portable arithmetic remains uncertified.
    const double radius=p.x*p.x+p.y*p.y+p.z*p.z;
    const double factor=params[0]/(radius+params[1]);
    return cpu_double3(abs(p.x)*factor+params[2],abs(p.y)*factor+params[3],abs(p.z)*factor+params[4]);
#endif
}
constant FormulaInfo formula_info_Kalisets1 = {
    .id = FORMULA_KALISETS1,
    .name = "Kalisets1",
    .param_count = 2,
    .complexity = 72,
    .uses_transcendental = false
};

