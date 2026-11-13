// Original ManowarBulb consumes XMM0.xy and XMM1.w, not previous XYZ.
// The iteration driver owns the explicit SSE lanes; derivative fields are unrelated.
constexpr unsigned int FORMULA_MANOWARBULB = 10262;
template<> struct ExternalFormulaUsesSse01<FORMULA_MANOWARBULB> {
    static constexpr bool value = true;
};
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ManowarBulbOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MANOWARBULB>(cpu_double3 p, constant double* params, double &de) {
    if (params[BFIRSTIT_SLOT] == 0.0) {
        params[BFIRSTIT_SLOT] = 1.0;
        params[FORMULA_STATE_X_SLOT] = params[FORMULA_STATE_Y_SLOT] = 0.0;
        params[FORMULA_STATE_Z_SLOT] = params[FORMULA_STATE_W_SLOT] = 0.0;
        return p;
    }
    double state[18] = {p.x,p.y,p.z,params[FORMULA_ORBIT_W_SLOT],
        params[FORMULA_STATE_X_SLOT],params[FORMULA_STATE_Y_SLOT],
        params[FORMULA_STATE_Z_SLOT],params[FORMULA_STATE_W_SLOT]};
#if defined(_MSC_VER) && defined(_M_X64)
    ManowarBulbOriginal_x87(state, params);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const E x(p.x), y(p.y), z(p.z);
    // CODE 0045..0070 sums z,y,x and spills the power to Double. A zero
    // radius reaches FYL2X/FPREM and produces NaN, unlike libm pow(0,p).
    const E radius = sqrt(z*z + y*y + x*x);
    const double power=static_cast<double>(numeric::extendedExp2(E(params[0])*numeric::extendedLog2(radius),false));
    const auto phi=numeric::extendedSinCos(numeric::extendedAtan2(y,x)*E(params[0]));
    const auto theta=numeric::extendedSinCos(numeric::extendedAtan2(z,sqrt(x*x+y*y))*E(params[0]));
    const double cp=static_cast<double>(phi.cos),sp=static_cast<double>(phi.sin);
    const double ct=static_cast<double>(theta.cos),st=static_cast<double>(theta.sin);
    // Retain the two original Double spills: rotated previous lanes + C,
    // then the current-point contribution. XMM1.high is W, not Z.
    const E a=E(state[4])*E(params[2]), b=E(state[5])*E(params[2]), c=E(state[7])*E(params[2]);
    const double rx=static_cast<double>(a*E(params[4])+b*E(params[5])+c*E(params[6])+E(params[13]));
    const double ry=static_cast<double>(a*E(params[7])+b*E(params[8])+c*E(params[9])+E(params[14]));
    const double rz=static_cast<double>(a*E(params[10])+b*E(params[11])+c*E(params[12])+E(params[15]));
    const double bx=static_cast<double>(x*E(params[3])+E(rx));
    const double by=static_cast<double>(y*E(params[3])+E(ry));
    const double bz=static_cast<double>(z*E(params[3])+E(rz));
    state[0]=static_cast<double>(E(cp)*E(ct)*E(power)+E(bx));
    state[1]=static_cast<double>(E(sp)*E(ct)*E(power)+E(by));
    state[2]=static_cast<double>(E(st)*E(power)*E(params[1])+E(bz));
    state[4]=p.x;state[5]=p.y;state[6]=p.z;state[7]=state[3];
#else
    // Portable algebraic fallback; native x87 parity is tested separately.
    const double r = sqrt(p.x*p.x+p.y*p.y+p.z*p.z);
    const double power = pow(abs(r), params[0]);
    const double phi = atan2(p.y,p.x)*params[0];
    const double theta = atan2(p.z,sqrt(p.x*p.x+p.y*p.y))*params[0];
    const RotMatrix3 rot = rotMatrix3FromFloats(params,4);
    const double a=state[4]*params[2], b=state[5]*params[2], c=state[7]*params[2];
    const double rx=a*rot.col0.x+b*rot.col1.x+c*rot.col2.x+params[13];
    const double ry=a*rot.col0.y+b*rot.col1.y+c*rot.col2.y+params[14];
    const double rz=a*rot.col0.z+b*rot.col1.z+c*rot.col2.z+params[15];
    state[0]=cos(phi)*cos(theta)*power+(rx+params[3]*p.x);
    state[1]=sin(phi)*cos(theta)*power+(ry+params[3]*p.y);
    state[2]=sin(theta)*power*params[1]+(rz+params[3]*p.z);
    state[4]=p.x;state[5]=p.y;state[6]=p.z;state[7]=state[3];
#endif
    params[FORMULA_STATE_X_SLOT]=state[4];
    params[FORMULA_STATE_Y_SLOT]=state[5];
    params[FORMULA_STATE_Z_SLOT]=state[6];
    params[FORMULA_STATE_W_SLOT]=state[7];
    return cpu_double3(state[0],state[1],state[2]);
}
