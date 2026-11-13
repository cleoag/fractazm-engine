// Original SSE2 transform and conditional coordinate commit, followed by x87 DE.
#include "ifs_apply_shared.h"
#include "ifs_original_state.h"
constexpr unsigned int FORMULA_SCHWARTZIFS = 10223;


template<>
cpu_double3 execute_formula<FORMULA_SCHWARTZIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    return execute_original_ifs_apply<2>(p,params,de);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const auto r=native.rotate(params+7);
    const E x=r.x,y=r.y,z=r.z;
    double negative=double(((x*x+y*y)+z*z)+abs(E(params[0]))),positive=0;
    for(E value : {x,y,z}) {
        const E angle=value*params[5];
        const auto exponent=angle.signExponent()&0x7fff;
        const E term=exponent>=16383+63 && exponent<0x7fff ? angle : extendedSinCos(angle).cos;
        if(!(term>=E(0))) negative=double(E(negative)-term);
        else positive=double(term+E(positive));
    }
    const E t=abs(E(params[5]))-E(1);
    const E distance=(sqrt(E(negative))-sqrt(E(positive)))/sqrt(((t+abs(t))+E(1))*E(.5));
    return native.finish(distance,de);
#else
    // Portable arithmetic is not yet certified against the original x87 CODE.
    const double Distance = params[0];         // [PVar-0x10]
    const double Scale = params[1];            // [PVar-0x20] (.2Doubles)
    const double Z_add = params[2];            // [PVar-0x30] (.2Doubles)
    const double Y_add = params[3];            // [PVar-0x38]
    const double X_add = params[4];            // [PVar-0x40]
    const double Cos_mul = params[5];          // [PVar-0x48]
    const int Apply_scale_add = int(params[6]); // [PVar-0x4c]
    const double M0 = params[7];
    const double M1 = params[8];
    const double M2 = params[9];
    const double M3 = params[10];
    const double M4 = params[11];
    const double M5 = params[12];
    const double M6 = params[13];
    const double M7 = params[14];
    const double M8 = params[15];
    const double c_half = 0.5;                // [edi+0x00]

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    double rx = x * M0 + y * M1 + z * M2;
    double ry = x * M3 + y * M4 + z * M5;
    double rz = x * M6 + y * M7 + z * M8;

    double r2 = rx * rx + ry * ry + rz * rz;
    double neg_part = r2 + abs(Distance);

    double pos_sum = 0.0;

    double c_x = cos(rx * Cos_mul);
    if (c_x < 0.0) {
        neg_part -= c_x;  // neg_part += |c_x|
    } else {
        pos_sum += c_x;
    }

    double c_y = cos(ry * Cos_mul);
    if (c_y < 0.0) {
        neg_part -= c_y;
    } else {
        pos_sum += c_y;
    }

    double c_z = cos(rz * Cos_mul);
    if (c_z < 0.0) {
        neg_part -= c_z;
    } else {
        pos_sum += c_z;
    }

    double schwartz = sqrt(neg_part) - sqrt(pos_sum);

    double abs_m = abs(Cos_mul);
    double t = abs_m - 1.0;
    double scale_factor = 1.0 + t + abs(t);  // = 1 + 2*max(0, |m|-1)
    double norm = sqrt(scale_factor * c_half);
    double shape_de = schwartz / norm;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
#endif
}

cpu_double3 schwartzIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_SCHWARTZIFS>(p, params, de);
}

constant FormulaInfo formula_info_schwartzIFS = {
    .id = FORMULA_SCHWARTZIFS,
    .name = "schwartzIFS",
    .param_count = 16,
    .complexity = 500,
    .uses_transcendental = true
};


