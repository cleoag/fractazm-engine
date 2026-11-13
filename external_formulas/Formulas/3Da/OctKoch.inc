// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_OCTKOCH = 10039;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void OctKochOriginal_x87(double*, const double*, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_OCTKOCH>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de};
    OctKochOriginal_x87(state,params,int(params[24]));
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    const E x=abs(E(p.x)),y=abs(E(p.y)),z=abs(E(p.z));
    E hi=(x*params[4]+y*params[5])+z*params[6];
    E mid=(x*params[7]+y*params[8])+z*params[9];
    E lo=(x*params[10]+y*params[11])+z*params[12];
    // The first sort exchanges ties; the second uses JAE and keeps them.
    if (!(hi > mid)) std::swap(hi,mid);
    if (!(hi > lo)) std::swap(hi,lo);
    if (!(mid > lo)) std::swap(mid,lo);
    E xv=double(hi),yv=double(mid),zv=double(lo);
    const E k=E(params[0])-1, f=k*params[22];
    xv=double(xv-k*params[23]);
    xv=abs(xv+f)+(-abs(xv-f)-xv);
    if (int(params[24]) != 0) xv=abs(xv);
    xv=double(xv);
    hi=yv; mid=xv; lo=zv;
    if (!(hi >= mid)) std::swap(hi,mid);
    if (!(hi >= lo)) std::swap(hi,lo);
    if (!(mid >= lo)) std::swap(mid,lo);
    const E r0=(hi*params[13]+mid*params[14])+lo*params[15];
    const E r1=(hi*params[16]+mid*params[17])+lo*params[18];
    const E r2=(hi*params[19]+mid*params[20])+lo*params[21];
    de=double(E(de)*params[0]);
    return cpu_double3(double((r0-k*params[1])*params[0]),
                       double((r1-k*params[2])*params[0]),
                       double((r2-k*params[3])*params[0]));
#else
    // Portable arithmetic remains uncertified.
    const double Scale   = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);

    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    const double FoldX   = params[22];
    const double SubX    = params[23];
    const int doAbsAfterFold = (int)params[24];

    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    cpu_double3 r1v = R1 * cpu_double3(ax, ay, az);
    double r0 = r1v.x;
    double r1 = r1v.y;
    double r2 = r1v.z;

    double max_val = r0, mid_val = r1, min_val = r2;
    if (max_val < mid_val) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (max_val < min_val) { double t = max_val; max_val = min_val; min_val = t; }
    if (mid_val < min_val) { double t = mid_val; mid_val = min_val; min_val = t; }

    double x_val = max_val;
    double y_val = mid_val;
    double z_val = min_val;

    double k = Scale - 1.0;
    x_val -= k * SubX;

    double F = k * FoldX;
    x_val = abs(x_val + F) - abs(x_val - F) - x_val;

    if (doAbsAfterFold != 0) {
        x_val = abs(x_val);
    }

    double s_max = y_val, s_mid = x_val, s_min = z_val;
    if (s_max < s_mid) { double t = s_max; s_max = s_mid; s_mid = t; }
    if (s_max < s_min) { double t = s_max; s_max = s_min; s_min = t; }
    if (s_mid < s_min) { double t = s_mid; s_mid = s_min; s_min = t; }

    cpu_double3 r2v = R2 * cpu_double3(s_mid, s_max, s_min);
    double rot0 = r2v.x;
    double rot1 = r2v.y;
    double rot2 = r2v.z;

    rot1 -= k * CScaleX;
    rot0 -= k * CScaleY;
    rot2 -= k * CScaleZ;

    de *= Scale;
    rot0 *= Scale;
    rot1 *= Scale;
    rot2 *= Scale;

    return cpu_double3(rot1, rot0, rot2);
#endif
}

constant FormulaInfo formula_info_OctKoch = {
    .id = FORMULA_OCTKOCH,
    .name = "OctKoch",
    .param_count = 13,
    .complexity = 1250,
    .uses_transcendental = true
};

