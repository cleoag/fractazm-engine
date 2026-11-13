// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_IQBULB_OLD = 10260;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void IQ_bulb_oldOriginal_x87(double*, const double*, int, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_IQBULB_OLD>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[11] = {p.x,p.y,p.z};
    IQ_bulb_oldOriginal_x87(state,params,int(params[3]),int(params[4]));
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E x=p.x,y=p.y,z=p.z,radius=sqrt(x*x+(y*y+z*z));
    const E power=double(extendedExp2(E(params[0])*extendedLog2(abs(radius)),false));
    const E zPower=int(params[3])==0 ? power
        : E(double(extendedExp2(E(params[1])*extendedLog2(abs(E(double(radius)))),false)));
    const E theta=extendedAtan2Wide(sqrt(x*x+z*z),radius+y);
    const auto polar=extendedSinCos((theta+theta)*params[1]);
    E phi=extendedAtan2(z,x);
    // AND AH,41h followed by JA skips correction unless 0 > x.
    if(int(params[4])==0 && E(0)>x) {
        const E shift=x87Pi()*params[5];
        if(E(0)>z) phi+=shift; else phi-=shift;
    }
    const auto azimuth=extendedSinCos(phi*params[1]);
    const E ps=double(polar.sin),pc=double(polar.cos);
    const E azs=double(azimuth.sin),azc=double(azimuth.cos);
    return cpu_double3(double((power*ps)*azs+params[6]),
                       double(power*pc+params[7]),
                       double(((zPower*azc)*ps)*params[2]+params[8]));
#else
    // Portable fallback remains uncertified.
    const double Power     = params[0];  // default: 8
    const double zAnglePow = params[1];  // default: 8
    const double zMul      = params[2];  // default: 1
    const int   Mode      = (int)params[3];  // default: 0
    const int   Atan2Mode = (int)params[4];  // default: 0

    double x = p.x, y = p.y, z = p.z;

    if (x == 0.0 && y == 0.0 && z == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }

    double r = sqrt(x * x + y * y + z * z);

    double r_P = pow(abs(r), Power);

    double r_stored;
    if (Mode == 0) {
        r_stored = r_P;
    } else {
        r_stored = pow(abs(r), zAnglePow);
    }

    double rho_xz = sqrt(x * x + z * z);
    double r_full = sqrt(x * x + y * y + z * z);
    double theta = 2.0 * atan2(rho_xz, r_full + y);

    double theta_scaled = theta * zAnglePow;
    double cos_theta = cos(theta_scaled);
    double sin_theta = sin(theta_scaled);

    double phi = atan2(z, x);

    if (Atan2Mode == 0) {
        if (x <= 0.0 && z <= 0.0) {
            phi += M_PI;
        } else if (x <= 0.0) {
            phi -= M_PI;
        }
    }

    double phi_scaled = phi * zAnglePow;
    double cos_phi = cos(phi_scaled);
    double sin_phi = sin(phi_scaled);

    double new_x = r_P * sin_theta * sin_phi + params[6];
    double new_y = r_P * cos_theta + params[7];
    double new_z = r_stored * cos_phi * sin_theta * zMul + params[8];

    return cpu_double3(new_x, new_y, new_z);
#endif
}

constant FormulaInfo formula_info_IQbulb_old = {
    .id = FORMULA_IQBULB_OLD,
    .name = "IQ-bulb old",
    .param_count = 5,
    .complexity = 322,
    .uses_transcendental = true
};

