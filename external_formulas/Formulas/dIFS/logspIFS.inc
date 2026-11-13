#include "ifs_original_state.h"
// Original M3F arithmetic; portable fallback remains uncertified.
constexpr unsigned int FORMULA_LOGSPIFS = 10229;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void logspIFSOriginal_x87(double*, const double*, const float*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_LOGSPIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[6]);
    const cpu_double3 transformed(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[1];
    double state[]={transformed.x,transformed.y,transformed.z,0.,0.};
    float coefficients[11]; for(int i=0;i<11;++i) coefficients[i]=float(params[10+i]);
    const int flags[]={apply,int(params[7]),int(params[9])};
    logspIFSOriginal_x87(state,params,coefficients,flags);
    de=state[4];
    return apply!=0 ? transformed : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const auto rotated=native.rotate(params+10);
    E x=rotated.x,y=rotated.y,z=rotated.z;
    if(std::signbit(params[5])) {
        const E shifted=x+E(1),inverse=E(1)/(shifted*shifted+y*y);
        const E a=double(((x*x+y*y)-E(1))*inverse+params[4]);
        const E b=double(((-y)+(-y))*inverse);
        x=double(b*float(params[20])+a*float(params[19]));
        y=double(b*float(params[19])+(-(a*float(params[20]))));
    }
    const E radius=sqrt(x*x+y*y),infinity=std::numeric_limits<double>::infinity();
    const int style=int(params[9]);
    E radial;
    if(style==1) radial=E(params[0])/radius;
    else {
        // Unlike a power reduction, direct FYL2X retains infinite endpoints.
        const E logarithm=radius==E(0) ? -infinity : radius==infinity ? infinity : extendedLog2(radius);
        radial=E(params[0])*logarithm;
        if(style==2) radial=-(radial*radial);
        else if(style==3) {
            const bool negative=std::signbit(double(radial));
            radial=abs(radial)+params[22];
            if(negative) radial=-radial;
            radial=E(1)/radial;
        }
    }
    const E angle=(extendedAtan2(x,y)*params[21])*E(int(params[7]))+radial;
    const E distance=sqrt(abs(round(angle)-angle)*params[8]+z*z)-abs(E(params[5]));
    return native.finish(distance,de);
#else

    const double Tightness = params[0];           // [edi-0x10]
    const double Scale = params[1];               // .2Doubles
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add_MobTweak = params[4];
    const double Radius_neg_Mobius = params[5];   // [edi-0x48] — if sign bit set, enable Mobius
    const int Apply_scale_add = int(params[6]);
    const int N_spirals = int(params[7]);        // [edi-0x50]
    const double z_Mul = params[8];               // [edi-0x58]
    const int Style = int(params[9]);            // [edi-0x5c] integer

    const double c_inv2pi = params[21];  // -0.159154943
    const double c_offset = params[22];  // 0.05

    double x = p.x * Scale + X_add_MobTweak;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    double m00 = params[10], m01 = params[11], m02 = params[12];
    double m10 = params[13], m11 = params[14], m12 = params[15];
    double m20 = params[16], m21 = params[17], m22 = params[18];
    double rx = x * m00 + y * m01 + z * m02;
    double ry = x * m10 + y * m11 + z * m12;
    double rz = x * m20 + y * m21 + z * m22;
    x = rx; y = ry; z = rz;

    if (Radius_neg_Mobius < 0.0) {
        double denom = (x + 1.0) * (x + 1.0) + y * y;
        double inv = 1.0 / denom;
        double new_y = (x * x + y * y - 1.0) * inv;
        double new_x = -2.0 * y * inv;

        new_y += X_add_MobTweak; // fadd [edi-0x40] = X_add (MobTweak)

        double MobRho_sin = params[19]; // [edi-0x84]
        double MobRho_cos = params[20]; // [edi-0x88]
        double rot_x = new_x * MobRho_sin - new_y * MobRho_cos;
        double rot_y = new_x * MobRho_cos + new_y * MobRho_sin;
        rot_x = new_x * MobRho_sin - new_y * MobRho_cos;
        x = rot_y; // stored to [esp]
        y = rot_x; // stored to [esp+8]
    }

    double r0 = sqrt(x * x + y * y);


    double r_val;
    if (Style == 1) {
        r_val = Tightness / r0;
    } else if (Style == 2) {
        double lv = Tightness * log2(r0);
        r_val = -(lv * lv);
    } else if (Style == 3) {
        double lv = Tightness * log2(r0);
        double alv = fabs(lv) + c_offset;
        if (lv < 0.0) alv = -alv; // restore sign
        r_val = 1.0 / alv;
    } else {
        r_val = Tightness * log2(r0);
    }

    double theta = atan2(x, y) * c_inv2pi * double(N_spirals) + r_val;
    double frac_theta = fabs(theta - rint(theta));

    double tube_val = frac_theta * z_Mul;
    double shape_de = sqrt(tube_val + z * z) - fabs(Radius_neg_Mobius);

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(state_x, state_y, state_z);
#endif
}

cpu_double3 logspIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_LOGSPIFS>(p, params, de);
}

constant FormulaInfo formula_info_logspIFS = {
    .id = FORMULA_LOGSPIFS,
    .name = "logspIFS",
    .param_count = 23,
    .complexity = 441,
    .uses_transcendental = true
};

