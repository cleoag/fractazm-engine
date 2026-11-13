// Original inverse bispherical transform: cancellation and stored Double values
// are part of the formula. Algebraic simplification changes its axis behavior.
constexpr unsigned int FORMULA__INVBISP = 3804;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void InvBispOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__INVBISP>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z};
    InvBispOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    const E x=p.x,y=p.y,z=p.z,a=params[0];
    const E radius=x*x+(y*y+z*z), plus=radius+a*a, az=a*z, twice=az+az;
    const E root=double(sqrt(abs(plus*plus+(-(twice*twice)))));
    const E minus=double(E(double(radius))+(-(a*a))), numerator=double(twice);
    const E perpendicular=sqrt(root*root-minus*minus), v=numerator/root;
    const E log2e=E::fromBits(0x3fff,0xb8aa3b295c17f0bcULL);
    const E argument=v+sqrt(v*v+1), infinity=std::numeric_limits<double>::infinity();
    // This FYL2X is a final logarithm, without the power tail's invalid
    // infinity-minus-infinity reduction. Preserve its infinite endpoints.
    const E logarithm=argument == infinity ? infinity
                      : argument == E(0) ? -infinity : extendedLog2(argument);
    return cpu_double3(double(E(params[1])-extendedAtan2(perpendicular,minus)),
                       double(log2e*logarithm),
                       double(extendedAtan2(y,x)));
#else
    // Portable numerical parity remains unverified; retain original operations.
    const double a = params[0];
    const double radius = p.x*p.x + (p.y*p.y + p.z*p.z);
    const double plus = radius + a*a;
    const double az = a*p.z;
    const double twice = az + az;
    const double discRoot = sqrt(fabs(plus*plus - twice*twice));
    const double minus = radius - a*a;
    const double perpendicular = sqrt(discRoot*discRoot - minus*minus);
    const double v = twice/discRoot;
    return cpu_double3(params[1] - atan2(perpendicular,minus),
                       log(v + sqrt(v*v + 1.0))*M_LOG2E*M_LOG2E,
                       atan2(p.y,p.x));
#endif
}

cpu_double3 _invbisp_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA__INVBISP>(p,params,de);
}

constant FormulaInfo formula_info__invbisp = {
    .id = FORMULA__INVBISP,
    .name = "_invbisp",
    .param_count = 1,
    .complexity = 168,
    .uses_transcendental = true
};
