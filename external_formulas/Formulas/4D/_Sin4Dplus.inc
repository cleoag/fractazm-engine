// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA__SIN4DPLUS = 4645;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Sin4DplusOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA__SIN4DPLUS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de};
    Sin4DplusOriginal_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    const E x=double(E(p.x)+params[0]), y=double(E(p.y)+params[1]),
            z=double(E(p.z)+params[2]), w=double(E(de)+params[3]);
    const E r=sqrt((w*w+y*y)+z*z);
    if ((x.signExponent()&0x7fff) != 0x7fff && abs(x) >= E(0x1p63)) {
        // FSINCOS leaves X in place, so FLD ST(2) addresses an empty slot.
        de=double(extendedNaN());
        return cpu_double3(de,de,de);
    }
    const auto sc=extendedSinCos(x);
    const E log2e=E::fromBits(0x3fff,0xb8aa3b295c17f0bcULL);
    const E e=extendedExp2(r*log2e,false);
    const E a=(e-E(1)/e)*params[5], b=(e+E(1)/e)*params[5];
    const double ox=double(sc.sin*b);
    const E factor=(sc.cos*a)/r;
    const double oy=double(y*factor), oz=double(z*factor), ow=double(w*factor);
    de=double(E(ow)*params[4]);
    return cpu_double3(double(E(ox)*params[4]),double(E(oy)*params[4]),
                       double(E(oz)*params[4]));
#else
    // Portable arithmetic remains uncertified.
    const double Px       = params[0];   // [edi-0x10] (default: 0.0)
    const double Py       = params[1];   // [edi-0x18] (default: 0.0)
    const double Pz       = params[2];   // [edi-0x20] (default: 0.0)
    const double Pw       = params[3];   // [edi-0x28] (default: 0.0)
    const double Strength = params[4];   // [edi-0x30] (default: 1.5)
    const double Scale    = params[5];   // [edi+0x00]
    const double thresh   = params[6];   // [edi+0x08] dead x87 branch threshold

    double x = p.x + Px;
    double y = p.y + Py;
    double z = p.z + Pz;
    double w = de + Pw;

    double r = std::sqrt(y * y + z * z + w * w);

    double x_out, y_out, z_out, w_out;

    (void)thresh;
    double e = x87_exp(r);
    double inv_e = 1.0 / e;
    double A = (e - inv_e) * Scale;
    double B = (e + inv_e) * Scale;

    double sin_x = std::sin(x);
    double cos_x = std::cos(x);

    x_out = sin_x * B;

    double factor = cos_x * A / r;

    y_out = y * factor;
    z_out = z * factor;
    w_out = w * factor;

    double strength = Strength;
    x_out *= strength;
    y_out *= strength;
    z_out *= strength;
    w_out *= strength;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
#endif
}

constant FormulaInfo formula_info__Sin4Dplus = {
    .id = FORMULA__SIN4DPLUS,
    .name = "_Sin4Dplus",
    .param_count = 5,
    .complexity = 258,
    .uses_transcendental = true
};

