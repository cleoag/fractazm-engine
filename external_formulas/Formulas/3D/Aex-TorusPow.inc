// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_AEX_TORUSPOW = 3238;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void AexTorusPowOriginal_x87(double*, const double*, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_AEX_TORUSPOW>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[9]={p.x,p.y,p.z};
    AexTorusPowOriginal_x87(state,params,int(params[4]));
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E x=p.x,y=p.y,z=p.z;
    const E theta=extendedAtan2(y,x);
    const auto thetaPower=extendedSinCos(theta*params[0]);
    const auto thetaBase=extendedSinCos(E(double(theta)));
    const auto polar=extendedSinCos(extendedAtan2(sqrt(x*x+y*y),z)*params[0]);
    const E a=double(int(params[4])==0 ? polar.cos+params[3] : polar.sin);
    const E b=double(int(params[4])==0 ? polar.sin : polar.cos+params[3]);
    const E dx=x+(-(E(double(thetaBase.cos))*params[1]));
    const E dy=y+(-(E(double(thetaBase.sin))*params[1]));
    const E radius=(dx*dx+dy*dy)+z*z;
    const E power=extendedExp2((E(params[0])*params[5])*extendedLog2(abs(radius)),false);
    // FST leaves the power extended for X; Y/Z reload its Double slots.
    const E storedPower=double(power);
    return cpu_double3(double((power*a)*E(double(thetaPower.cos))+params[7]),
                       double((storedPower*a)*E(double(thetaPower.sin))+params[8]),
                       double((storedPower*b+params[9])*params[2]));
#else
    // Portable arithmetic remains uncertified.
    const double Power     = params[0];  // [PVar-0x10] (default: 8)
    const double Donutness = params[1];  // [PVar-0x18] (default: 1.5)
    const double ZMul      = params[2];  // [PVar-0x20] (default: 1)
    const double CosSubt   = params[3];  // [PVar-0x28] (default: 0)
    const double Mode_0_1  = params[4];  // [PVar-0x2c] int (default: 0)
    const double HalfPow   = params[5];  // [PVar+0x00] constant
    const double Cx        = params[7];  // from engine context [esi+0x18]
    const double Cy        = params[8];  // from engine context [esi+0x20]
    const double Cz        = params[9];  // from engine context [esi+0x28]

    double x = p.x, y = p.y, z = p.z;

    double theta = atan2(y, x);

    double thetaP = theta * Power;
    double cos_tP = cos(thetaP);
    double sin_tP = sin(thetaP);

    double cos_th = cos(theta);
    double sin_th = sin(theta);

    double rxy = sqrt(x * x + y * y);
    double phi = atan2(rxy, z);

    double phiP = phi * Power;
    double cos_pP = cos(phiP);
    double sin_pP = sin(phiP);

    double A, B;
    if (Mode_0_1 == 0.0) {
        A = cos_pP + CosSubt;
        B = sin_pP;
    } else {
        A = sin_pP;
        B = cos_pP + CosSubt;
    }

    double dx = x - Donutness * cos_th;
    double dy = y - Donutness * sin_th;
    double r_torus_sq = dx * dx + dy * dy + z * z;

    double rp = pow(fabs(r_torus_sq), Power * HalfPow);

    double new_x = rp * A * cos_tP + Cx;
    double new_y = rp * A * sin_tP + Cy;
    double new_z = (rp * B + Cz) * ZMul;

    return cpu_double3(new_x, new_y, new_z);
#endif
}

constant FormulaInfo formula_info_Aex_TorusPow = {
    .id = FORMULA_AEX_TORUSPOW,
    .name = "Aex-TorusPow",
    .param_count = 8,
    .complexity = 213,
    .uses_transcendental = true
};

