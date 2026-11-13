// Original piriform power chain, Double scratch stores and optional orbit writeback.
constexpr unsigned int FORMULA_PIRIFORMIFS = 10196;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void PiriformIFSDistance_x87(double*, const double*, int apply, int curve);
#endif
template<>
cpu_double3 execute_formula<FORMULA_PIRIFORMIFS>(cpu_double3 p, constant double* params, double &de) {
    const int apply=int(params[6]), curve=int(params[7]);
    const cpu_double3 local(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if (apply) params[DIFS_VARYSCALE_SLOT]*=params[1];
    double x=local.x,z=local.z;
    if (int(params[9])) {const double t=x;x=z;z=t;}
    x=int(params[10])?fabs(x):-x;
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={x,local.y,z,0.,0.};
    PiriformIFSDistance_x87(state,params,apply,curve);
    de=state[4];
#else
    const numeric::X87Scope extendedScope;
    numeric::X87Real u=(numeric::X87Real)x*x;
    if (curve==0) u*=x;
    if (curve!=2) u*=x;
    const numeric::X87Real extra=u*x;
    const double storedU=double(u);
    const numeric::X87Real v1=((extra+(numeric::X87Real)local.y*local.y)+(numeric::X87Real)z*z)+params[5];
    const numeric::X87Real v2=(numeric::X87Real)storedU+params[0];
    numeric::X87Real distance=numeric::x87Sqrt((v1+numeric::x87Abs(v1))*params[11])-numeric::x87Sqrt((v2+numeric::x87Abs(v2))*params[11]);
    if (!(params[8]<=0)) {
        const numeric::X87Real limit=(numeric::X87Real)fabs(x)-params[8];
        if (limit>=distance) distance=limit;
    }
    if (!apply) distance/=params[1];
    de=double(distance);
#endif
    return apply?local:p;
}
constant FormulaInfo formula_info_PiriformIFS = {
    .id=FORMULA_PIRIFORMIFS,.name="PiriformIFS",.param_count=11,
    .complexity=450,.uses_transcendental=false
};
