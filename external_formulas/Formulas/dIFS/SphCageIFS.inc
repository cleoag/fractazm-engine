// Meridian and parallel toruses, using the original clamped latitude loop.
constexpr unsigned int FORMULA_SPHCAGEIFS = 10303;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void SphCage_x87(double* state, const double* params, int apply, int parallels);
#endif

template<>
cpu_double3 execute_formula_dIFS<FORMULA_SPHCAGEIFS>(cpu_double3 p, double* params,
                                                   double& varyScale, double& rout) {
    const double x=p.x*params[1]+params[4], y=p.y*params[1]+params[3];
    const double z=p.z*params[1]+params[2];
    const int apply=int(params[6]);
    const int rawParallels=int(params[7]);
    const int parallels=rawParallels<2 ? 2 : (rawParallels>1000 ? 1000 : rawParallels);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x,y,z,0.0};
    SphCage_x87(state,params,apply,parallels);
    rout=state[3];
#else
    const long double radius=params[0];
    const double step=double(radius/parallels*params[12]);
    long double angle=(params[11]-atan2(static_cast<long double>(x),static_cast<long double>(y)))*params[9];
    angle=rint(angle*params[8])/params[8]*params[10];
    const double cs=double(cos(angle)), sn=double(sin(angle));
    const long double along=static_cast<long double>(x)*cs+static_cast<long double>(y)*sn;
    const long double perp=static_cast<long double>(x)*sn-static_cast<long double>(y)*cs;
    const long double ring=sqrt(along*along+static_cast<long double>(z)*z)-radius;
    long double distance=sqrt(ring*ring+perp*perp)-params[5];
    double latitude=-params[0], dz=double(abs(static_cast<long double>(z))-radius);
    for (int i=0;i<parallels/2;++i) {
        // The original stores the current minimum before each next latitude.
        const double previous=double(distance);
        dz=double(static_cast<long double>(step)+dz);
        latitude=double(static_cast<long double>(step)+latitude);
        const long double radial=sqrt(static_cast<long double>(x)*x+static_cast<long double>(y)*y)
            -sqrt(radius*radius-static_cast<long double>(latitude)*latitude);
        const long double candidate=sqrt(radial*radial+static_cast<long double>(dz)*dz)-params[5];
        distance=previous>=candidate ? candidate : previous;
    }
    if (!apply) distance/=params[1];
    rout=double(distance);
#endif
    params[DIFS_COMMIT_XYZ_SLOT]=apply!=0 ? 1.0 : 0.0;
    if (apply!=0) varyScale*=params[1];
    return cpu_double3(x,y,z);
}

template<>
cpu_double3 execute_formula<FORMULA_SPHCAGEIFS>(cpu_double3 p, double* params, double& de) {
    double varyScale=1.0;
    return execute_formula_dIFS<FORMULA_SPHCAGEIFS>(p,params,varyScale,de);
}
cpu_double3 SphCageIFS_formula(cpu_double3 p, double* params, double& de) {
    return execute_formula<FORMULA_SPHCAGEIFS>(p,params,de);
}
constant FormulaInfo formula_info_SphCageIFS = {
    .id=FORMULA_SPHCAGEIFS, .name="SphCageIFS", .param_count=9,
    .complexity=355, .uses_transcendental=true
};
