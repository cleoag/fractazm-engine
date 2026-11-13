// Torus knot dIFS: original SSE2 affine transform and x87 shape distance.
constexpr unsigned int FORMULA_KNOTSIFS = 10199;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void KnotsIFSOriginal_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula_dIFS<FORMULA_KNOTSIFS>(cpu_double3 p, double* params,
                                                 double& varyScale, double& rout) {
    const double x=p.x*params[1]+params[4], y=p.y*params[1]+params[3];
    const double z=p.z*params[1]+params[2];
    const int apply=int(params[6]);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x,y,z,0.0};
    KnotsIFSOriginal_x87(state,params,apply);
    rout=state[3];
#else
    const long double order=int(params[7]);
    const long double angle=atan2(static_cast<long double>(x),static_cast<long double>(y))
        *(int(params[8])+static_cast<long double>(int(params[9]))/order);
    const double cs=double(cos(angle)), sn=double(sin(angle));
    const double radial=double(sqrt(static_cast<long double>(x)*x+static_cast<long double>(y)*y)-params[0]);
    const double v=double(static_cast<long double>(radial)*cs+static_cast<long double>(z)*sn);
    const double u=double(static_cast<long double>(radial)*sn-static_cast<long double>(z)*cs);
    const long double snap=rint((params[13]-atan2(static_cast<long double>(v),static_cast<long double>(u)))*params[11]*order)/order*params[12];
    const double c=double(cos(snap)), s=double(sin(snap));
    const long double a=static_cast<long double>(v)*c-static_cast<long double>(u)*s+params[10];
    const long double b=static_cast<long double>(v)*s+static_cast<long double>(u)*c;
    long double distance=sqrt(a*a+b*b)-params[5];
    if (!apply) distance/=params[1];
    rout=double(distance);
#endif
    params[DIFS_COMMIT_XYZ_SLOT]=apply!=0 ? 1.0 : 0.0;
    if (apply!=0) varyScale*=params[1];
    return cpu_double3(x,y,z);
}
template<>
cpu_double3 execute_formula<FORMULA_KNOTSIFS>(cpu_double3 p, double* params, double& de) {
    double varyScale=1.0;
    return execute_formula_dIFS<FORMULA_KNOTSIFS>(p,params,varyScale,de);
}
cpu_double3 KnotsIFS_formula(cpu_double3 p, double* params, double& de) {
    return execute_formula<FORMULA_KNOTSIFS>(p,params,de);
}
constant FormulaInfo formula_info_KnotsIFS = {
    .id=FORMULA_KNOTSIFS, .name="KnotsIFS", .param_count=11,
    .complexity=272, .uses_transcendental=true
};
