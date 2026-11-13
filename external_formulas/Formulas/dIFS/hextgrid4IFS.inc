// hextgrid4IFS: local shape rotation and original x87 state.
constexpr unsigned int FORMULA_HEXTGRID4IFS = 10235;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HexTubeGrid4_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula<FORMULA_HEXTGRID4IFS>(cpu_double3 p, constant double* params, double &de) {
    const double RadiusMod = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Styling1 = params[5];
    const int Apply_scale_add = int(params[6]);
    const double Styling2 = params[16];
    const double Styling3 = params[17];

    const double c_half = params[18];           // [edi+0x00]
    const double c_2s3 = params[19];            // [edi+0x08]
    const double c_s32 = params[20];            // [edi+0x10]

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    const cpu_double3 transformed(x, y, z);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, 0.0};
    HexTubeGrid4_x87(state, params, Apply_scale_add);
    de = state[3];
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    E rotated[3];
    for(int row=0;row<3;++row) {
        const double* m=params+7+row*3;
        rotated[row]=double((E(x)*float(m[0])+E(y)*float(m[1]))+E(z)*float(m[2]));
    }
    // FST stores cell size while the two styling products retain its ext80 value.
    const E cell=E(c_2s3)*Styling1,h=double(cell);
    const E adj1=double(cell*Styling3),adj2=double(cell*Styling2);
    const E rx=abs(rotated[0]),ry=rotated[1],rz=rotated[2];
    const auto line=[&](E along,E cross,bool subtract) {
        const int32_t index=along.integer();
        const E fraction=double(abs(E(double(along))-E(index)));
        const E offset=abs(subtract ? E(index)*adj2-cross : E(index)*adj2+cross);
        const E snapped=abs(-(round(offset/h)*h)+E(double(offset)))+adj1;
        return E(double(fraction>=snapped ? fraction : snapped));
    };
    const E a=line(rx,ry,true);
    const E b=line(abs(rx*c_half-ry*c_s32),ry*c_half+rx*c_s32,false);
    const E c=line(abs(rx*c_half+ry*c_s32),ry*c_half+(-(rx*c_s32)),false);
    E nearest=c>=b ? b : c;
    if(nearest>=a) nearest=a;
    E distance=sqrt(nearest*nearest+rz*rz)-RadiusMod;
    if(!Apply_scale_add) distance/=Scale;
    de=double(distance);
#else
    double m00 = params[7],  m01 = params[8],  m02 = params[9];
    double m10 = params[10], m11 = params[11], m12 = params[12];
    double m20 = params[13], m21 = params[14], m22 = params[15];
    double rx = x * m00 + y * m01 + z * m02;
    double ry = x * m10 + y * m11 + z * m12;
    double rz = x * m20 + y * m21 + z * m22;
    x = rx; y = ry; z = rz;

    double cell_h = c_2s3 * Styling1; // [esp+0x44]
    double cell_adj1 = cell_h * Styling3; // [esp+0x54] uses [edi-0x80]
    double cell_adj2 = cell_h * Styling2; // [esp+0x4c] uses [edi-0x78]

    double ax = fabs(x);
    int ix = x87_fistp_int32(ax);
    double frac_x = fabs(ax - double(ix));

    double ay1 = fabs(double(ix) * cell_adj2 - y);
    double fy1 = x87_frndint(ay1 / cell_h);
    ay1 = fabs(ay1 - fy1 * cell_h) + cell_adj1;
    double d1 = (frac_x >= ay1 ? frac_x : ay1);

    double bv = fabs(x * c_half - y * c_s32);
    int ibv = x87_fistp_int32(bv);
    double frac_bv = fabs(bv - double(ibv));
    double by2 = fabs(double(ibv) * cell_adj2 + y * c_half + x * c_s32);
    double fy2 = x87_frndint(by2 / cell_h);
    by2 = fabs(by2 - fy2 * cell_h) + cell_adj1;
    double d2 = (frac_bv >= by2 ? frac_bv : by2);

    double cv = fabs(x * c_half + y * c_s32);
    int icv = x87_fistp_int32(cv);
    double frac_cv = fabs(cv - double(icv));
    double cy3 = fabs(double(icv) * cell_adj2 + y * c_half - x * c_s32);
    double fy3 = x87_frndint(cy3 / cell_h);
    cy3 = fabs(cy3 - fy3 * cell_h) + cell_adj1;
    double d3 = (frac_cv >= cy3 ? frac_cv : cy3);

    double d_min = min(min(d1, d2), d3);

    double shape_de = sqrt(d_min * d_min + z * z) - RadiusMod;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
#endif
    return Apply_scale_add != 0 ? transformed : p;
}

cpu_double3 hextgrid4IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HEXTGRID4IFS>(p, params, de);
}

constant FormulaInfo formula_info_hextgrid4IFS = {
    .id = FORMULA_HEXTGRID4IFS,
    .name = "hextgrid4IFS",
    .param_count = 18,
    .complexity = 580,
    .uses_transcendental = false
};

