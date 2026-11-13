// PG-bulb: preserve original multiplication order and Double accumulator spills.
// Each final component receives its Julia seed before the last Double store.
constexpr unsigned int FORMULA_PG_BULB = 10331;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void PGBulbOriginal_x87(double* state, const double* params,
                                    int power, int old_angles);
#endif

template<>
cpu_double3 execute_formula<FORMULA_PG_BULB>(cpu_double3 p, constant double* params, double &de) {
    int power = int(params[1]);
    // The shipped CODE accepts 1..16, replacing other powers by 2. Power 1
    // still executes one multiply because its loop tests at the bottom.
    if (power < 1 || power >= 17) power = 2;
    const int old_angles = int(params[2]);
    double state[] = {p.x, p.y, p.z, de, params[3], params[4], params[5], params[6]};
#if defined(_MSC_VER) && defined(_M_X64)
    PGBulbOriginal_x87(state, params, power, old_angles);
#else
    const numeric::X87Scope extendedScope;
    const numeric::X87Real x=p.x, y=p.y, z=p.z, w=de;
    const numeric::X87Real angle_x=old_angles ? state[4] : p.x;
    const numeric::X87Real angle_y=old_angles ? state[5] : p.y;
    const numeric::X87Real sum=angle_x*angle_x+angle_y*angle_y;
    // Both angle coefficients are explicitly stored to Double by the CODE.
    // A zero denominator deliberately propagates the original NaN/Inf.
    const double ct=double((angle_x*angle_x-angle_y*angle_y)/sum);
    const double st=double((angle_x*angle_y)/sum);
    double a=p.x,b=p.y,c=p.z,d=double(w+params[0]);
    const int iterations=power>1 ? power-1 : 1;
    for (int i=0;i<iterations;++i) {
        const numeric::X87Real aa=a,bb=b,cc=c,dd=d,cos_theta=ct,sin_theta=st;
        const numeric::X87Real na=(((aa*x-bb*y)-(cos_theta*cc)*z)
                              +(cc*w+dd*z)*sin_theta)+(cos_theta*dd)*w;
        const numeric::X87Real nb=(((aa*y+bb*x)-(sin_theta*cc)*z)
                              -(cc*w+dd*z)*cos_theta)+(sin_theta*dd)*w;
        const numeric::X87Real nc=((aa*z+cc*x)-bb*w)-dd*y;
        const numeric::X87Real nd=((aa*w+dd*x)+bb*z)+cc*y;
        if (i+1==iterations) {
            state[0]=double(na+state[4]); state[1]=double(nb+state[5]);
            state[2]=double(nc+state[6]); state[3]=double(nd+state[7]);
        } else {
            a=double(na);b=double(nb);c=double(nc);d=double(nd);
        }
    }
#endif
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
}

constant FormulaInfo formula_info_PG_bulb = {
    .id = FORMULA_PG_BULB,
    .name = "PG-bulb",
    .param_count = 7,
    .complexity = 404,
    .uses_transcendental = false
};
