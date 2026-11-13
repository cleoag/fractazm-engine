// Original transform without C: retain x87 folds and the signed DE multiplier.
constexpr unsigned int FORMULA__AMAZINGBOX = 7039;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void AmazingBoxTransform_x87(double* state, const double* params);
#endif

template<>
cpu_double3 execute_formula<FORMULA__AMAZINGBOX>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de};
    AmazingBoxTransform_x87(state,params);
    de=state[3];return cpu_double3(state[0],state[1],state[2]);
#else
    const numeric::X87Scope extendedScope;
    const numeric::X87Real x=numeric::x87Abs((numeric::X87Real)p.x+1)-(numeric::x87Abs((numeric::X87Real)p.x-1)+p.x);
    const numeric::X87Real y=numeric::x87Abs((numeric::X87Real)p.y+1)-(numeric::x87Abs((numeric::X87Real)p.y-1)+p.y);
    const numeric::X87Real z=numeric::x87Abs((numeric::X87Real)p.z+1)-(numeric::x87Abs((numeric::X87Real)p.z-1)+p.z);
    const numeric::X87Real radius=(z*z+y*y)+x*x;
    const numeric::X87Real scale=!(radius>=params[1])?params[0]:(radius>1?params[2]:params[2]/radius);
    de=double((numeric::X87Real)de*scale);
    return cpu_double3(double(x*scale),double(y*scale),double(z*scale));
#endif
}
constant FormulaInfo formula_info__AmazingBox = {
    .id=FORMULA__AMAZINGBOX, .name="_AmazingBox", .param_count=3,
    .complexity=153, .uses_transcendental=false
};
