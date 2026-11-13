// Preserve original x87 branches, arithmetic and Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void QuatPowOriginal_x87(double*, const double*);
#endif
constexpr unsigned int FORMULA_QUATPOW = 10339;


template<>
cpu_double3 execute_formula<FORMULA_QUATPOW>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    QuatPowOriginal_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    E x=p.x,y=p.y,z=p.z,w=de;
    const E radius=sqrt((w*w+y*y)+z*z),epsilon=params[9];
    const auto logarithm=[](E v) {
        const E infinity=std::numeric_limits<double>::infinity();
        return v==E(0) ? -infinity : v==infinity ? infinity : extendedLog2(v);
    };
    if(radius>E(0) || radius<E(0)) {
        const E direction=extendedAtan2(radius,x)/radius;
        const E square=x*x+radius*radius;
        x=double(epsilon>square ? E(params[11]) : (extendedLn2()*logarithm(square))*params[10]);
        y=double(y*direction); z=double(z*direction); w=double(w*direction);
    } else x=double(epsilon>x ? E(params[11]) : extendedLn2()*logarithm(x));
    x=double(x*params[4]); y=double(y*params[5]);
    z=double(z*params[6]); w=double(w*params[7]);
    const E qx=double(((x*params[0]+(-(y*params[1])))+(-(z*params[2])))+(-(w*params[3])));
    const E qy=double(((x*params[1]+y*params[0])+z*params[3])+(-(w*params[2])));
    const E qz=double(((x*params[2]+(-(y*params[3])))+z*params[0])+w*params[1]);
    const E qw=double(((x*params[3]+y*params[2])+(-(z*params[1])))+w*params[0]);
    const E vectorRadius=sqrt((qw*qw+qy*qy)+qz*qz);
    const auto sc=extendedSinCos(vectorRadius);
    const E log2e=E::fromBits(0x3fff,0xb8aa3b295c17f0bcULL);
    const E exponential=extendedExp2(qx*log2e,false);
    x=double(exponential*sc.cos);
    E factor=exponential*sc.sin;
    if(!(epsilon>vectorRadius)) factor/=vectorRadius;
    // Out-of-range FSINCOS does not push: the later ST(3) is empty, so
    // all three vector components become indefinite while X uses the angle.
    y=double(qy*factor); z=double(qz*factor); w=double(qw*factor);
    de=double(w);
    return cpu_double3(double(x+E(params[12])*params[8]),
                       double(y+E(params[13])*params[8]),
                       double(z+E(params[14])*params[8]));
#else
    // Portable numerical parity remains unverified.

    const double pw = params[0];     // [PVar-0x10] p (real power), default 2
    const double pi_p = params[1];   // [PVar-0x18] p of i, default 0
    const double pj = params[2];     // [PVar-0x20] p of j, default 0
    const double pk = params[3];     // [PVar-0x28] p of k, default 0
    const double apx = params[4];    // [PVar-0x30] anisotropic x, default 1
    const double apy = params[5];    // [PVar-0x38] anisotropic y, default 1
    const double apz = params[6];    // [PVar-0x40] anisotropic z, default 1
    const double apw = params[7];    // [PVar-0x48] anisotropic w, default 1
    const double C_mul = params[8];  // [PVar-0x50] C multiplier, default 1

    const double Cx = params[12];   // [esi+0x18]
    const double Cy = params[13];   // [esi+0x20]
    const double Cz_c = params[14]; // [esi+0x28]

    const double EPSILON = 1e-200;
    const double DEFAULT_LOG = 460.0;

    double x = p.x, y = p.y, z = p.z, w = de;

    double r_vec = sqrt(w*w + y*y + z*z);

    double log_x, log_y, log_z, log_w;

    if (r_vec > 0.0 || r_vec < 0.0) {
        double theta = atan2(r_vec, x);
        double t = theta / r_vec;  // normalized direction
        double r2 = x*x + r_vec*r_vec;

        if (!(EPSILON > r2)) {
            log_x = 0.5 * log(r2);  // = 0.5 * ln(r2)
        } else {
            log_x = DEFAULT_LOG;
        }
        log_y = y * t;
        log_z = z * t;
        log_w = w * t;
    } else {
        if (!(EPSILON > x)) {
            log_x = log(x);
        } else {
            log_x = DEFAULT_LOG;
        }
        log_y = y;
        log_z = z;
        log_w = w;
    }

    log_w *= apw;
    log_z *= apz;
    log_y *= apy;
    log_x *= apx;

    double qx = log_x*pw - log_y*pi_p - log_z*pj - log_w*pk;
    double qy = log_x*pi_p + log_y*pw + log_z*pk - log_w*pj;
    double qz = log_x*pj - log_y*pk + log_z*pw + log_w*pi_p;
    double qw = log_x*pk + log_y*pj - log_z*pi_p + log_w*pw;

    double r_vec2 = sqrt(qw*qw + qy*qy + qz*qz);
    double cos_rv = cos(r_vec2);
    double sin_rv = sin(r_vec2);
    double exp_r = exp(qx);

    double out_x = exp_r * cos_rv;

    double factor = exp_r * sin_rv;
    if (!(EPSILON > r_vec2)) {
        factor /= r_vec2;
    }

    double out_y = qy * factor;
    double out_z = qz * factor;
    double out_w = qw * factor;

    out_x += Cx * C_mul;
    out_y += Cy * C_mul;
    out_z += Cz_c * C_mul;

    de = out_w;
    return cpu_double3(out_x, out_y, out_z);
#endif
}

constant FormulaInfo formula_info_QuatPow = {
    .id = FORMULA_QUATPOW,
    .name = "QuatPow",
    .param_count = 12,
    .complexity = 531,
    .uses_transcendental = true
};
