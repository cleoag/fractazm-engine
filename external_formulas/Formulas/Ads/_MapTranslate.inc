// Original _MapTranslate: sample the sphere map once, then offset/scale its RGB.
constexpr unsigned int FORMULA__MAPTRANSLATE = 10006;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MapTranslateFinish_x87(double* state, const double* params);
#endif

inline cpu_double3 mapTranslateSample(cpu_double3 p, constant double* params, CPUVec3 sample) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,sample.x,sample.y,sample.z};
    MapTranslateFinish_x87(state, params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    const numeric::X87Scope extendedScope;
    return cpu_double3(
        double((numeric::X87Real)p.x+((numeric::X87Real)sample.x+params[1])*params[4]),
        double((numeric::X87Real)p.y+((numeric::X87Real)sample.y+params[2])*params[5]),
        double((numeric::X87Real)p.z+((numeric::X87Real)sample.z+params[3])*params[6]));
#endif
}

template<>
cpu_double3 execute_formula<FORMULA__MAPTRANSLATE>(cpu_double3 p, constant double* params, double &de) {
    (void)de;
    return mapTranslateSample(p, params, cpu_map_sphere_sample(p.x,p.y,p.z,int(params[0])));
}

template<>
cpu_double3 execute_formula_with_sphere_map<FORMULA__MAPTRANSLATE>(cpu_double3 p, double* params,
                                                                  double& de, TLMSfunction map) {
    if (!map) return execute_formula<FORMULA__MAPTRANSLATE>(p,params,de);
    TVec3D input={p.x,p.y,p.z}, sample{};
    map(&sample,&input,int(params[0]));
    return mapTranslateSample(p,params,CPUVec3(sample[0],sample[1],sample[2]));
}

constant FormulaInfo formula_info__maptranslate = {
    .id = FORMULA__MAPTRANSLATE, .name = "_MapTranslate", .param_count = 7,
    .complexity = 80, .uses_transcendental = false
};
