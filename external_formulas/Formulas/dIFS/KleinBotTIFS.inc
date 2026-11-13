// Torical Klein bottle: SSE2 affine coordinates and original x87 distance.
constexpr unsigned int FORMULA_KLEINBOTTIFS = 10221;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void KleinBotTIFSOriginal_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula_dIFS<FORMULA_KLEINBOTTIFS>(cpu_double3 p, double* params,
                                                     double& varyScale, double& rout) {
    const double x=p.x*params[1]+params[4], y=p.y*params[1]+params[3];
    const double z=p.z*params[1]+params[2];
    const int apply=int(params[6]);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={x,y,z,0.0};
    KleinBotTIFSOriginal_x87(state,params,apply);
    rout=state[3];
#else
    // Match original constants and Double stores; native Win64 above also
    // preserves extended intermediates and unordered x87 branch behavior.
    const bool negativeOrUnordered=!(y>=0.0);
    const double shifted=double(static_cast<long double>(x)+(negativeOrUnordered ? params[8] : -params[8]));
    long double radial=sqrt(static_cast<long double>(shifted)*shifted+static_cast<long double>(y)*y)-params[8];
    double outer=double(abs(sqrt(radial*radial+static_cast<long double>(z)*z)-params[0])-params[7]);
    double scratch=0.0;
    long double current;
    if (negativeOrUnordered) {
        const double radius=double(atan2(static_cast<long double>(y),static_cast<long double>(x))*params[12]
            *(static_cast<long double>(params[5])+params[5])+params[0]);
        radial=sqrt(static_cast<long double>(x)*x+static_cast<long double>(y)*y)-params[9];
        current=sqrt(radial*radial+static_cast<long double>(z)*z)-radius;
        scratch=double(current);
        current=abs(current)-params[7];
    } else {
        const double shifted2=double(static_cast<long double>(x)+params[9]);
        radial=(sqrt(static_cast<long double>(shifted2)*shifted2+static_cast<long double>(z)*z)+params[5])-params[0];
        current=abs(sqrt(radial*radial+static_cast<long double>(y)*y)-params[5])-params[7];
    }
    outer=double((abs(static_cast<long double>(scratch))-scratch)+outer);
    if (current>=outer) current=outer;
    if (!apply) current/=params[1];
    rout=double(current);
#endif
    params[DIFS_COMMIT_XYZ_SLOT]=apply!=0 ? 1.0 : 0.0;
    if (apply!=0) varyScale*=params[1];
    return cpu_double3(x,y,z);
}
template<>
cpu_double3 execute_formula<FORMULA_KLEINBOTTIFS>(cpu_double3 p, double* params, double& de) {
    double varyScale=1.0;
    return execute_formula_dIFS<FORMULA_KLEINBOTTIFS>(p,params,varyScale,de);
}
cpu_double3 KleinBotTIFS_formula(cpu_double3 p, double* params, double& de) {
    return execute_formula<FORMULA_KLEINBOTTIFS>(p,params,de);
}
constant FormulaInfo formula_info_KleinBotTIFS = {
    .id=FORMULA_KLEINBOTTIFS, .name="KleinBotTIFS", .param_count=8,
    .complexity=366, .uses_transcendental=true
};
