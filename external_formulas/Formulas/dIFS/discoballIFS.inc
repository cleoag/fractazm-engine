// Faceted spherical shell: preserve the original quadrant and rounding sequence.
constexpr unsigned int FORMULA_DISCOBALLIFS = 1489;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void DiscoBall_x87(double* state, const double* params, int apply);
#endif

double discoballIFS_shape_de(cpu_double3 p, constant double* params) {
    const double x=p.x*params[2]+params[5], y=p.y*params[2]+params[4];
    const double z=p.z*params[2]+params[3];
    const int apply=int(params[6]);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x,y,z,0.0};
    DiscoBall_x87(state,params,apply);
    return state[3];
#else
    long double angle=(params[11]-atan2(static_cast<long double>(x),static_cast<long double>(y)))*params[9];
    angle=rint(angle*params[0])/params[0]*params[10];
    const double c=double(cos(angle)), sn=double(sin(angle));
    const double u=double(static_cast<long double>(x)*c+static_cast<long double>(y)*sn);
    angle=(params[11]-atan2(static_cast<long double>(u),static_cast<long double>(z)))*params[9];
    angle=rint(angle*params[1])/params[1]*params[10];
    const double c2=double(cos(angle)), sn2=double(sin(angle));
    long double distance=abs(static_cast<long double>(u)*c2+static_cast<long double>(z)*sn2-params[7])-params[8];
    if (!apply) distance/=params[2];
    return double(distance);
#endif
}

template<>
cpu_double3 execute_formula_dIFS<FORMULA_DISCOBALLIFS>(cpu_double3 p, double* params,
                                                    double& varyScale, double& rout) {
    const int apply=int(params[6]);
    rout=discoballIFS_shape_de(p,params);
    params[DIFS_COMMIT_XYZ_SLOT]=apply!=0 ? 1.0 : 0.0;
    if (apply!=0) varyScale*=params[2];
    return cpu_double3(p.x*params[2]+params[5],p.y*params[2]+params[4],p.z*params[2]+params[3]);
}

template<>
cpu_double3 execute_formula<FORMULA_DISCOBALLIFS>(cpu_double3 p, double* params, double& de) {
    double varyScale=1.0;
    return execute_formula_dIFS<FORMULA_DISCOBALLIFS>(p,params,varyScale,de);
}
cpu_double3 discoballIFS_formula(cpu_double3 p, double* params, double& de) {
    return execute_formula<FORMULA_DISCOBALLIFS>(p,params,de);
}
constant FormulaInfo formula_info_discoballIFS = {
    .id=FORMULA_DISCOBALLIFS, .name="discoballIFS", .param_count=9,
    .complexity=229, .uses_transcendental=true
};
