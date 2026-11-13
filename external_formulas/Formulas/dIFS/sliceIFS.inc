// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_SLICEIFS = 8107;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void SliceDistance_x87(double*, const double*, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_SLICEIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[6]);
    const cpu_double3 transformed(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[1];
    double state[4]={transformed.x,transformed.y,transformed.z};
    SliceDistance_x87(state,params,apply);
    de=state[3];
    return apply!=0 ? transformed : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E=numeric::Extended80;
    const int apply=int(params[6]);
    // SSE2 transform stores precede the x87 distance calculation.
    const cpu_double3 q(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if(apply) params[DIFS_VARYSCALE_SLOT]*=params[1];
    const E x(q.x),y(q.y),z(q.z);
    const E sector=numeric::extendedAtan2(y,x)*E(params[9])/E(params[7]);
    // Do not cancel the division/multiplication by Angle: Angle=0 is
    // observable, as are the stored constants rather than rounded literals.
    const E tooth=-((abs((sector-round(sector))*E(params[7]))/E(params[0])-E(1))*E(params[0])*E(params[10]));
    const E shell=abs(sqrt(x*x+y*y+z*z)-E(params[5])+E(params[8]))-E(params[8]);
    E distance=shell>=tooth ? shell : tooth;
    if(!apply) distance/=E(params[1]);
    de=static_cast<double>(distance);
    return apply ? q : p;
#else
    // Portable arithmetic remains uncertified.
    const double Angle = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Sph_Radius = params[5];
    const int Apply_scale_add = int(params[6]);
    const double RepeatAngle = params[7];
    const double Thickness = params[8];

    const double K_360_OVER_PI = 114.591559;     // 360/pi — converts radians to 720-scale
    const double K_PI_OVER_360 = 0.008726646;    // pi/360 — converts 720-scale to radians

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    double angle_720 = atan2(y, x) * K_360_OVER_PI;
    double sector = angle_720 / RepeatAngle;
    double frac = sector - rint(sector);
    double frac_angle = frac * RepeatAngle;
    double tooth_offset = (Angle - abs(frac_angle)) * K_PI_OVER_360;

    double r = sqrt(x * x + y * y + z * z);
    double sphere_dist = abs(r - Sph_Radius + Thickness) - Thickness;

    double shape_de = max(sphere_dist, tooth_offset);

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(x, y, z);
#endif
}

cpu_double3 sliceIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_SLICEIFS>(p, params, de);
}

constant FormulaInfo formula_info_sliceIFS = {
    .id = FORMULA_SLICEIFS,
    .name = "sliceIFS",
    .param_count = 9,
    .complexity = 360,
    .uses_transcendental = true
};

