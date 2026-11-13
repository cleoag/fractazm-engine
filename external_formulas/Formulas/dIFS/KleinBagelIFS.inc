// Original SSE2 transform and conditional coordinate commit, followed by x87 DE.
#include "ifs_apply_shared.h"
#include "ifs_original_state.h"
constexpr unsigned int FORMULA_KLEINBAGELIFS = 10195;

template<>
cpu_double3 execute_formula<FORMULA_KLEINBAGELIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    return execute_original_ifs_apply<1>(p,params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const E x=native.transformed.x,y=native.transformed.y,z=native.transformed.z;
    const auto sc=extendedSinCos((extendedAtan2(x,y)*E(int(params[8])))*E(params[10]));
    const E sn=double(sc.sin),cs=double(sc.cos),scaledX=double(x*params[9]);
    const E radius=int(params[7])==0 ? sqrt(scaledX*scaledX+y*y) : (abs(y)>=abs(scaledX) ? abs(y) : abs(scaledX));
    const E ring=double(radius-params[0]);
    const E u=double(ring*sn+(-(z*cs))),v=ring*cs+z*sn,square=v*v;
    const E distance=sqrt(abs((square*square+(-square))+u*u))*E(params[10])-params[5];
    return native.finish(distance,de);
#else
    // Portable arithmetic is not yet certified against the original x87 CODE.
    const double Radius_1 = params[0];  // [edi-0x10] — major radius
    const double Scale = params[1];     // [edi-0x20] (.2Doubles)
    const double Z_add = params[2];     // [edi-0x30] (.2Doubles)
    const double Y_add = params[3];     // [edi-0x38]
    const double X_add = params[4];     // [edi-0x40]
    const double Radius_2 = params[5];  // [edi-0x48] — tube radius
    const int Apply_scale_add = int(params[6]); // [edi-0x4c]
    const int Boxy_XY = int(params[7]);         // [edi-0x50]
    const int Mobius = int(params[8]);           // [edi-0x54]
    const double Aspect_Ratio = params[9];       // [edi-0x5c]

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    double theta = atan2(x, y);
    double alpha = double(Mobius) * theta * 0.5;
    double cos_alpha = cos(alpha);
    double sin_alpha = sin(alpha);

    double x_scaled = x * Aspect_Ratio;

    double r_xy;
    if (Boxy_XY == 0) {
        r_xy = sqrt(x_scaled * x_scaled + y * y);
    } else {
        r_xy = max(abs(x_scaled), abs(y));
    }

    double ring_dist = r_xy - Radius_1;


    double u = ring_dist * sin_alpha - z * cos_alpha;
    double v = ring_dist * cos_alpha + z * sin_alpha;

    double lemniscate = v * v * v * v - v * v + u * u;
    double shape_de_raw = sqrt(abs(lemniscate));

    double shape_de = shape_de_raw * 0.5 - Radius_2;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 KleinBagelIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_KLEINBAGELIFS>(p, params, de);
}

constant FormulaInfo formula_info_KleinBagelIFS = {
    .id = FORMULA_KLEINBAGELIFS,
    .name = "KleinBagelIFS",
    .param_count = 10,
    .complexity = 500,
    .uses_transcendental = true
};


