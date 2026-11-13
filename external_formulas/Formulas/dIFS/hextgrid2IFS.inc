// hextgrid2IFS: local shape rotation and original x87 state.
constexpr unsigned int FORMULA_HEXTGRID2IFS = 10202;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void HexTubeGrid2_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula<FORMULA_HEXTGRID2IFS>(cpu_double3 p, constant double* params, double &de) {
    const double RadiusMod  = params[0];   // [edi - 0x10]  default: 0.2
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const double Grid_style = params[5];   // [edi - 0x48]  default: 1
    const int Apply_scale_add = int(params[6]); // [edi - 0x4c]
    const double const_neg2   = params[16];
    const double const_half   = params[17];
    const double cos30        = params[18];
    const double const_neg3   = params[19];
    const double const_inv2s3 = params[20];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    const cpu_double3 transformed(x, y, z);
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, 0.0};
    HexTubeGrid2_x87(state, params, Apply_scale_add);
    de = state[3];
#else

    double rx = params[7]  * x + params[8]  * y + params[9]  * z;
    double ry = params[10] * x + params[11] * y + params[12] * z;
    double rz = params[13] * x + params[14] * y + params[15] * z;

    double val_a = fabs(ry - 2.0 * x87_frndint(ry * 0.5));
    double val_b = fabs(rx * cos30 + x87_frndint(rx * const_inv2s3) * const_neg3) + val_a * const_half;
    double grid_max = max(val_a, val_b);

    double grid_val = grid_max - Grid_style;

    double final_grid = min(val_a, grid_val);

    double shape_de = sqrt(final_grid * final_grid + rz * rz) - RadiusMod;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
#endif
    return Apply_scale_add != 0 ? transformed : p;
}

cpu_double3 hextgrid2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_HEXTGRID2IFS>(p, params, de);
}

constant FormulaInfo formula_info_hextgrid2IFS = {
    .id = FORMULA_HEXTGRID2IFS,
    .name = "hextgrid2IFS",
    .param_count = 16,
    .complexity = 550,
    .uses_transcendental = false
};

