// Original local YZX distance; Apply controls only orbit/VaryScale writeback.
constexpr unsigned int FORMULA_COLUMNSIFS = 10220;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ColumnsIFSDistance_x87(double*, const double*, int iterations, int apply);
#endif
template<>
cpu_double3 execute_formula<FORMULA_COLUMNSIFS>(cpu_double3 p, constant double* params, double &de) {
    const int apply=int(params[12]), iterations=int(params[8]);
    const cpu_double3 local(p.x*params[3]+params[6],p.y*params[3]+params[5],p.z*params[3]+params[4]);
    if (apply) params[DIFS_VARYSCALE_SLOT]*=params[3];
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={local.y,local.z,local.x,0.};
    ColumnsIFSDistance_x87(state,params,iterations,apply);
    de=state[3];
#else
    const numeric::X87Scope extendedScope;
    const double a=double((numeric::X87Real)fabs(local.x)-fabs(local.y)-params[9]);
    const double c=double((numeric::X87Real)fabs(local.x)+fabs(local.y)-params[10]);
    numeric::X87Real d1=double((-(numeric::X87Real)c+fabs(a)-params[11])*params[13]);
    numeric::X87Real d2=double((-(numeric::X87Real)fabs(a)-c-params[11])*params[13]);
    const uint32_t count=uint32_t(iterations)-1u;
    if (d1>0 && count>0 && count<=999)
        for (uint32_t i=0;i<count;++i) d1=numeric::x87Abs(params[1]-numeric::x87Abs(params[1]-d1));
    const bool barrelNegative=std::signbit(params[7]);
    if (!barrelNegative) d2=params[7]-numeric::x87Abs(params[7]-d2);
    numeric::X87Real distance;
    if (!std::signbit(params[0])) {
        distance=numeric::x87Sqrt(d1*d1+d2*d2)-params[0];
        if (barrelNegative) distance-=(numeric::X87Real)local.z*local.z*((numeric::X87Real)params[7]+params[1]);
    } else {
        d1=numeric::x87Abs(d1);d2=numeric::x87Abs(d2);
        distance=(d2>=d1?d2:d1)+params[0];
        if (barrelNegative) distance+=(numeric::X87Real)local.z*((numeric::X87Real)params[7]+params[1]);
    }
    const numeric::X87Real height=(numeric::X87Real)fabs(local.z)-params[2];
    if (height>=distance) distance=height;
    if (!apply) distance/=params[3];
    de=double(distance);
#endif
    return apply?local:p;
}
constant FormulaInfo formula_info_columnsIFS = {
    .id=FORMULA_COLUMNSIFS,.name="columnsIFS",.param_count=13,
    .complexity=470,.uses_transcendental=false
};
