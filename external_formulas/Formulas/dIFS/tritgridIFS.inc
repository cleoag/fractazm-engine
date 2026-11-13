// Original SSE2 transform and conditional coordinate commit, followed by x87 DE.
#include "ifs_apply_shared.h"
#include "ifs_original_state.h"
constexpr unsigned int FORMULA_TRITGRIDIFS = 10205;

template<>
cpu_double3 execute_formula<FORMULA_TRITGRIDIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    return execute_original_ifs_apply<3>(p,params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+7);
    const E x=rotated.x,y=rotated.y,z=rotated.z;
    // FIST/FILD uses signed int32 indefinite on overflow, not FRNDINT.
    const auto residual=[](E value) { return abs(value-E(value.integer())); };
    const E a=residual(E(params[5])-x),u=x*E(params[16]),v=y*E(params[17]);
    const E b=residual(u+v),c=residual(v-u);
    E nearest=c>=b ? b : c; // C0 also keeps the top value on unordered.
    if(nearest>=a) nearest=a;
    return native.finish(sqrt(nearest*nearest+z*z)-params[0],de);
#else
    // Portable arithmetic is not yet certified against the original x87 CODE.
    const double RadiusMod  = params[0];   // [edi - 0x10]  default: 0.2
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const double Grid_style = params[5];   // [edi - 0x48]  default: 0
    const int Apply_scale_add = int(params[6]); // [edi - 0x4c]
    const double const_half = 0.5;
    const double cos30      = 0.866025;

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    }

    double rx = params[7]  * x + params[8]  * y + params[9]  * z;
    double ry = params[10] * x + params[11] * y + params[12] * z;
    double rz = params[13] * x + params[14] * y + params[15] * z;

    double t = Grid_style - rx;
    double a = fabs(t - x87_frndint(t));

    double u = rx * const_half;
    double v = ry * cos30;
    double sum_uv = u + v;
    double b = fabs(sum_uv - x87_frndint(sum_uv));
    double diff_vu = v - u;
    double c = fabs(diff_vu - x87_frndint(diff_vu));

    double min_abc = min(a, min(b, c));

    double shape_de = sqrt(min_abc * min_abc + rz * rz) - RadiusMod;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 tritgridIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TRITGRIDIFS>(p, params, de);
}

constant FormulaInfo formula_info_tritgridIFS = {
    .id = FORMULA_TRITGRIDIFS,
    .name = "tritgridIFS",
    .param_count = 16,
    .complexity = 450,
    .uses_transcendental = false
};


