// hextgrid3IFS: local shape rotation and original x87 state.
constexpr unsigned int FORMULA_HEXTGRID3IFS = 10228;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HexTubeGrid3_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula<FORMULA_HEXTGRID3IFS>(cpu_double3 p, constant double* params, double &de) {
    const double RadiusMod = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Grid_styling = params[5];
    const int Apply_scale_add = int(params[6]);
    const double Rho_sin = params[16];
    const double Rho_cos = params[17];
    const double Final_add = params[18];

    const double c_half = params[19];
    const double c_invs3 = params[20];
    const double c_ns32 = params[21];
    const double c_nhalf = params[22];
    const double c_s34 = params[23];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    const cpu_double3 transformed(x, y, z);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, 0.0};
    HexTubeGrid3_x87(state, params, Apply_scale_add);
    de = state[3];
#else
    double m00 = params[7],  m01 = params[8],  m02 = params[9];
    double m10 = params[10], m11 = params[11], m12 = params[12];
    double m20 = params[13], m21 = params[14], m22 = params[15];
    double rx = x * m00 + y * m01 + z * m02;
    double ry = x * m10 + y * m11 + z * m12;
    double rz = x * m20 + y * m21 + z * m22;
    x = rx; y = ry; z = rz;

    double ax = fabs(x - Grid_styling);

    double fx = x87_frndint(ax / Grid_styling * c_half - c_half);
    ax = fabs(ax - (fx * 2.0 + 1.0) * Grid_styling);

    double hy = Grid_styling * c_invs3;
    double ay = fabs(y - hy);
    double fy = x87_frndint(ay / hy * c_half - c_half);
    ay = fabs(ay - (fy * 2.0 + 1.0) * hy);

    double d1 = ax * c_ns32 + ay * c_nhalf + c_s34;
    double s1 = d1 - fabs(d1); // 2*min(d1, 0)
    double nax = fabs(ax - s1 * c_ns32);   // fabs applied
    double nay = fabs(ay - s1 * c_nhalf);  // fabs applied

    double d2 = nax * c_ns32 + nay * c_nhalf + c_s34;
    double s2 = d2 - fabs(d2);
    nax = nax - s2 * c_ns32;              // NO fabs for x in iteration 2
    nay = fabs(nay - s2 * c_nhalf);       // fabs applied for y

    double hex_val = nax * Rho_sin + nay * Rho_cos + Final_add;
    double shape_de = sqrt(hex_val * hex_val + z * z) - RadiusMod;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
#endif
    return Apply_scale_add != 0 ? transformed : p;
}

cpu_double3 hextgrid3IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HEXTGRID3IFS>(p, params, de);
}

constant FormulaInfo formula_info_hextgrid3IFS = {
    .id = FORMULA_HEXTGRID3IFS,
    .name = "hextgrid3IFS",
    .param_count = 25,
    .complexity = 431,
    .uses_transcendental = false
};

