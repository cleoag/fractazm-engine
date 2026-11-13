// Original SSE2 transform and conditional coordinate commit, followed by x87 DE.
#include "ifs_apply_shared.h"
#include "ifs_original_state.h"
constexpr unsigned int FORMULA_GREEKIFS = 10201;

template<>
cpu_double3 execute_formula<FORMULA_GREEKIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    return execute_original_ifs_apply<0>(p,params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    Arm64ApplyIFSState native(p,params,7);
    using E = Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+8);
    const E x=rotated.x,y=rotated.y,z=rotated.z, half=params[17];
    const bool negative=std::signbit(rotated.x) ^ (int(params[6]) == 0);
    const E ym=abs(negative ? y-half : y+half);
    E value=(ym >= abs(x) ? ym : abs(x))-half;
    value-=round(value);
    if (int(params[5]) != 0) value=abs(z) >= abs(value) ? abs(z) : abs(value);
    else value=sqrt(value*value+z*z);
    return native.finish(value-params[0],de);
#else
    // Portable arithmetic is not yet certified against the original x87 CODE.
    const double RadiusMod  = params[0];   // [edi - 0x10]  default: 0.2
    const double Scale      = params[1];   // [edi - 0x20]  .2Doubles
    const double Z_add      = params[2];   // [edi - 0x30]  .2Doubles
    const double Y_add      = params[3];   // [edi - 0x38]
    const double X_add      = params[4];   // [edi - 0x40]
    const int   Squared    = int(params[5]);  // [edi - 0x48]
    const int   Reverse    = int(params[6]);  // [edi - 0x4c + offset]

    const int Apply_scale_add = int(params[7]); // [edi - 0x4c]

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    double rx = params[8]  * x + params[9]  * y + params[10] * z;
    double ry = params[11] * x + params[12] * y + params[13] * z;
    double rz = params[14] * x + params[15] * y + params[16] * z;



    int Squared_i = int(params[5]);
    int Reverse_i = int(params[6]);
    int Apply_i   = int(params[7]);

    double ry_mod;
    bool rx_neg = (rx < 0.0);
    bool use_neg;
    if (Reverse_i == 0) {
        use_neg = !rx_neg;  // flip: positive rx -> negative offset
    } else {
        use_neg = rx_neg;
    }
    ry_mod = use_neg ? (ry - 0.5) : (ry + 0.5);

    double abs_ry_mod = fabs(ry_mod);

    double max_val = max(abs_ry_mod, fabs(rx));

    double v = max_val - 0.5;
    double frac_v = v - rint(v);

    double shape_de;
    if (Squared_i != 0) {
        double abs_frac = fabs(frac_v);
        double abs_rz = fabs(rz);
        shape_de = max(abs_frac, abs_rz) - RadiusMod;
    } else {
        shape_de = sqrt(frac_v * frac_v + rz * rz) - RadiusMod;
    }

    if (Apply_i == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_i != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 greekIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_GREEKIFS>(p, params, de);
}

constant FormulaInfo formula_info_greekIFS = {
    .id = FORMULA_GREEKIFS,
    .name = "greekIFS",
    .param_count = 17,
    .complexity = 550,
    .uses_transcendental = false
};


