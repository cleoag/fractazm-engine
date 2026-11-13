// General: shipped 2003-byte bulb, including its x87 flag-test behavior.
// Expanded options 0..10, literal half-pi at 11, Julia C at 12..14.
constexpr unsigned int FORMULA_GENERAL = 10318;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void GeneralOriginal_x87(double* state, const double* params);
#endif

// Portable fallback follows the shipped branches, not the descriptive labels
// in the .m3f. Exact extended-precision parity is verified on the native path.
#ifdef MB3D_SOFTWARE_EXT80
inline std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>
general_extended_pair(int args, mb3d::numeric::Extended80 x,
                      mb3d::numeric::Extended80 y, mb3d::numeric::Extended80 z) {
    switch (args) {
        case 0: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(x,y);
        case 1: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(x,z);
        case 2: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(y,x);
        case 3: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(y,z);
        case 4: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(z,x);
        case 5: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(z,y);
        case 6: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(x,sqrt(z*z+y*y));
        case 7: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(y,sqrt(x*x+z*z));
        case 8: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(z,sqrt(x*x+y*y));
        case 9: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(sqrt(z*z+y*y),x);
        case 10: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(sqrt(x*x+z*z),y);
        case 11: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(sqrt(x*x+y*y),z);
        // AND clears CF before JA/JB. JA selects the smaller magnitude;
        // JB is never taken, so selectors 18..23 retain the last loaded one.
        case 12: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(x, abs(y)>abs(z) ? abs(z) : abs(y));
        case 13: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(y, abs(x)>abs(z) ? abs(z) : abs(x));
        case 14: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(z, abs(y)>abs(x) ? abs(x) : abs(y));
        case 15: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(abs(y)>abs(z) ? abs(z) : abs(y),x);
        case 16: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(abs(x)>abs(z) ? abs(z) : abs(x),y);
        case 17: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(abs(y)>abs(x) ? abs(x) : abs(y),z);
        case 18: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(x,abs(y));
        case 19: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(y,abs(x));
        case 20: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(z,abs(y));
        case 21: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(abs(y),x);
        case 22: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(abs(x),y);
        case 23: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(abs(y),z);
        default: return std::pair<mb3d::numeric::Extended80,mb3d::numeric::Extended80>(NAN,NAN);
    }
}

#endif

inline cpu_double2 general_angle_pair(int args, double x, double y, double z) {
    switch (args) {
        case 0: return cpu_double2(x,y);
        case 1: return cpu_double2(x,z);
        case 2: return cpu_double2(y,x);
        case 3: return cpu_double2(y,z);
        case 4: return cpu_double2(z,x);
        case 5: return cpu_double2(z,y);
        case 6: return cpu_double2(x,sqrt(z*z+y*y));
        case 7: return cpu_double2(y,sqrt(x*x+z*z));
        case 8: return cpu_double2(z,sqrt(x*x+y*y));
        case 9: return cpu_double2(sqrt(z*z+y*y),x);
        case 10: return cpu_double2(sqrt(x*x+z*z),y);
        case 11: return cpu_double2(sqrt(x*x+y*y),z);
        // AND clears CF before JA/JB. JA selects the smaller magnitude;
        // JB is never taken, so selectors 18..23 retain the last loaded one.
        case 12: return cpu_double2(x, fabs(y)>fabs(z) ? fabs(z) : fabs(y));
        case 13: return cpu_double2(y, fabs(x)>fabs(z) ? fabs(z) : fabs(x));
        case 14: return cpu_double2(z, fabs(y)>fabs(x) ? fabs(x) : fabs(y));
        case 15: return cpu_double2(fabs(y)>fabs(z) ? fabs(z) : fabs(y),x);
        case 16: return cpu_double2(fabs(x)>fabs(z) ? fabs(z) : fabs(x),y);
        case 17: return cpu_double2(fabs(y)>fabs(x) ? fabs(x) : fabs(y),z);
        case 18: return cpu_double2(x,fabs(y));
        case 19: return cpu_double2(y,fabs(x));
        case 20: return cpu_double2(z,fabs(y));
        case 21: return cpu_double2(fabs(y),x);
        case 22: return cpu_double2(fabs(x),y);
        case 23: return cpu_double2(fabs(y),z);
        default: return cpu_double2(NAN,NAN);
    }
}

inline double general_angle(cpu_double2 pair, int tweak, double power, double halfpi) {
    if (tweak == 1 || tweak == 3) {
        power = -power;
        pair.y = fabs(pair.y);
    }
    double angle = atan2(pair.x,pair.y);
    if (tweak == 2 || tweak == 3)
        angle -= fabs(halfpi+angle)-fabs(halfpi-angle);
    return angle*power;
}

template<>
cpu_double3 execute_formula<FORMULA_GENERAL>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,params[12],params[13],params[14]};
    GeneralOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using namespace mb3d::numeric;
    ExtendedScope scope;
    if (int(params[7])) p.x=std::fabs(p.x);
    if (int(params[8])) p.y=std::fabs(p.y);
    if (int(params[9])) p.z=std::fabs(p.z);
    auto angle = [&](int selector,int tweak,double power) {
        auto [a,b]=general_extended_pair(selector,E(p.x),E(p.y),E(p.z));
        if (tweak == 1 || tweak == 3) { power=-power; b=abs(b); }
        E value=extendedAtan2(a,b);
        if (tweak == 2 || tweak == 3)
            value-=abs(E(params[11])+value)-abs(E(params[11])-value);
        return extendedSinCos(value*power);
    };
    const auto t=angle(int(params[3]),int(params[4]),params[0]);
    const auto f=angle(int(params[5]),int(params[6]),params[1]);
    // Four trigonometric Double spills; the radius FST retains its extended
    // value for FYL2X. FPREM uses truncation, then the power is stored Double.
    const double ct=double(t.cos), st=double(t.sin), cp=double(f.cos), sp=double(f.sin);
    const E x=p.x, y=p.y, z=p.z;
    const E radius=sqrt(x*x+(y*y+z*z));
    const double rp=double(extendedExp2(E(params[0])*extendedLog2(abs(radius)),false));
    E ox,oy,oz;
    if (int(params[10]) == 0) { ox=E(ct)*cp; oy=E(st)*cp; oz=E(params[2])*sp; }
    else if (int(params[10]) == 1) { ox=E(ct)*sp; oy=E(st)*sp; oz=E(params[2])*cp; }
    else { ox=E(ct)*cp; oy=st; oz=(E(params[2])*ct)*sp; }
    auto c = [&](int i) { return int(params[7+i]) == -1 ? abs(E(params[12+i])) : E(params[12+i]); };
    return cpu_double3(double(ox*rp+c(0)),double(oy*rp+c(1)),double(oz*rp+c(2)));
#else
    if (int(params[7])) p.x = fabs(p.x);
    if (int(params[8])) p.y = fabs(p.y);
    if (int(params[9])) p.z = fabs(p.z);
    const double th = general_angle(general_angle_pair(int(params[3]),p.x,p.y,p.z),
                                   int(params[4]),params[0],params[11]);
    const double ph = general_angle(general_angle_pair(int(params[5]),p.x,p.y,p.z),
                                   int(params[6]),params[1],params[11]);
    const double radius = sqrt(p.x*p.x+(p.y*p.y+p.z*p.z));
    const double rp = pow(radius,params[0]);
    const double ct=cos(th), st=sin(th), cp=cos(ph), sp=sin(ph);
    const int recipe=int(params[10]);
    cpu_double3 result;
    if (recipe == 0) result=cpu_double3(ct*cp,st*cp,params[2]*sp);
    else if (recipe == 1) result=cpu_double3(ct*sp,st*sp,params[2]*cp);
    else result=cpu_double3(ct*cp,st,params[2]*ct*sp);
    result.x = result.x*rp+(int(params[7]) == -1 ? fabs(params[12]) : params[12]);
    result.y = result.y*rp+(int(params[8]) == -1 ? fabs(params[13]) : params[13]);
    result.z = result.z*rp+(int(params[9]) == -1 ? fabs(params[14]) : params[14]);
    return result;
#endif
}

cpu_double3 General_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_GENERAL>(p,params,de);
}
constant FormulaInfo formula_info_General = {
    .id = FORMULA_GENERAL, .name = "General", .param_count = 11,
    .complexity = 2003, .uses_transcendental = true
};
