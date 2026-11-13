#include "ifs_original_state.h"
// Original operation order, comparison flags and intermediate Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void pyramIFSOriginal_x87(double*, const void*);
#endif
constexpr unsigned int FORMULA_PYRAMIFS = 3700;

template<>
cpu_double3 execute_formula<FORMULA_PYRAMIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    alignas(16) unsigned char buffer[160]{};
    auto* var = buffer + 144; // SSE2 packed memory operands require 16-byte alignment.
    const auto real = [&](int offset, double v) { std::memcpy(var + offset, &v, 8); };
    const auto integer = [&](int offset, double v) {
        const int32_t i = static_cast<int32_t>(v); std::memcpy(var + offset, &i, 4);
    };
    real(-16,params[0]); real(-32,params[1]); real(-24,params[1]);
    real(-48,params[2]); real(-40,params[2]); real(-56,params[3]); real(-64,params[4]);
    real(-72,params[5]); integer(-76,params[6]); real(-84,params[7]);
    double state[] = {p.x,p.y,p.z,params[DIFS_VARYSCALE_SLOT],0.};
    pyramIFSOriginal_x87(state,var);
    params[DIFS_VARYSCALE_SLOT]=state[3]; de=state[4];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const E x=native.transformed.x,y=native.transformed.y,z=native.transformed.z;
    const auto sc=extendedSinCos(z*params[7]);
    const E sn=double(sc.sin),cs=double(sc.cos);
    const E a=double(x*sn+(-(y*cs))),b=double(x*cs+y*sn);
    E distance=(a+b)+z;
    const E d1=(a-b)-z,d2=(b-a)-z,d3=(z-a)-b;
    if(d1>=distance) distance=d1;
    if(d2>=distance) distance=d2;
    if(d3>=distance) distance=d3;
    distance=(distance-params[0])+sqrt((a*a+b*b)+z*z)*params[5];
    return native.finish(distance,de);
#else
    // Portable arithmetic has not yet been certified against original x87 CODE.
    const double Length = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Pinch_Punch = params[5];
    const int Apply_scale_add = int(params[6]);
    const double ZTwist = params[7];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

    double twist_sin, twist_cos;
    twist_sin = sincos(z * ZTwist, twist_cos);

    double rx = x * twist_cos + y * twist_sin;
    double ry = x * twist_sin - y * twist_cos;

    double a = ry;
    double b = rx;
    double c = z;

    double d0 = a + b + c;          // face normal (+1,+1,+1)
    double d1 = a - b - c;          // face normal (+1,-1,-1)
    double d2 = b - a - c;          // face normal (-1,+1,-1)
    double d3 = c - a - b;          // face normal (-1,-1,+1)

    double tetra_dist = max(max(d0, d1), max(d2, d3));

    double r = sqrt(a * a + b * b + c * c);
    double shape_de = (tetra_dist - Length) + r * Pinch_Punch;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return Apply_scale_add != 0 ? cpu_double3(x,y,z) : p;
#endif
}

cpu_double3 pyramIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_PYRAMIFS>(p, params, de);
}

constant FormulaInfo formula_info_pyramIFS = {
    .id = FORMULA_PYRAMIFS,
    .name = "pyramIFS",
    .param_count = 8,
    .complexity = 560,
    .uses_transcendental = true
};

