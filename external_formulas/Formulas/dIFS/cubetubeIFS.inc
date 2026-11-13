// Original cubetubeIFS: local shape evaluation is independent of XYZ writeback.
constexpr unsigned int FORMULA_CUBETUBEIFS = 10302;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CubetubeIFSDistance_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula<FORMULA_CUBETUBEIFS>(cpu_double3 p, constant double* params, double &de) {
    const int apply = int(params[11]);
    // SSE2 preamble: each multiplication/addition rounds to Double.
    const cpu_double3 local(p.x*params[3]+params[6],
                            p.y*params[3]+params[5],
                            p.z*params[3]+params[4]);
    if (apply != 0) params[DIFS_VARYSCALE_SLOT] *= params[3];
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {local.y, local.z, local.x, params[MAX_FORMULA_PARAMS-1], 0.};
    CubetubeIFSDistance_x87(state, params, apply);
    params[MAX_FORMULA_PARAMS-1] = state[3];
    de = state[4];
#else
    const numeric::X87Scope extendedScope;
    // Mirror the original stack exchanges, including unordered/equal branches.
    numeric::X87Real a=fabs(local.x), b=fabs(local.z), c=fabs(local.y), t;
    if (!(a>b)) {t=a;a=b;b=t;}
    if (!(a>c)) {t=a;a=c;c=t;}
    t=a;a=b;b=t;
    if (!(a>c)) {t=a;a=c;c=t;}
    double x=double(c), y=double(b), z=double(a);
    double first=double((numeric::X87Real)x*params[7]+(numeric::X87Real)z*params[8]);
    x=double((numeric::X87Real)x*params[8]-(numeric::X87Real)z*params[7]);z=first;
    first=double((numeric::X87Real)x*params[9]+(numeric::X87Real)y*params[10]);
    x=double((numeric::X87Real)x*params[10]-(numeric::X87Real)y*params[9]);y=first;
    const numeric::X87Real dy=(numeric::X87Real)y-params[1], dz=(numeric::X87Real)z-params[2];
    y=double(dy);z=double(dz);
    const numeric::X87Real tube=numeric::x87Sqrt(dy*dy+dz*dz)-params[0];
    const numeric::X87Real dx=numeric::x87Abs((numeric::X87Real)x)-params[12];
    const numeric::X87Real sphere=numeric::x87Sqrt((dx*dx+(numeric::X87Real)y*y)+(numeric::X87Real)z*z)-params[13];
    const bool selectSphere=!(sphere>tube);
    params[MAX_FORMULA_PARAMS-1]=params[selectSphere?15:14];
    numeric::X87Real distance=selectSphere?sphere:tube;
    if (apply==0) distance/=params[3];
    de=double(distance);
#endif
    return apply != 0 ? local : p;
}

constant FormulaInfo formula_info_cubetubeIFS = {
    .id = FORMULA_CUBETUBEIFS, .name = "cubetubeIFS", .param_count = 16,
    .complexity = 333, .uses_transcendental = true
};
