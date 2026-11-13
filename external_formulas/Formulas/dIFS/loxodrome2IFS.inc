#include "ifs_original_state.h"
// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_LOXODROME2IFS = 10213;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Loxodrome2Distance_x87(double*, const double*, int, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_LOXODROME2IFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[6]);
    const cpu_double3 transformed(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[1];
    double state[]={transformed.x,transformed.y,transformed.z,0.,0.,0.};
    Loxodrome2Distance_x87(state,params,int(params[12]),apply);
    de=state[5];
    return apply!=0 ? transformed : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    const E x=native.transformed.x,y=native.transformed.y,z=native.transformed.z;
    E distance=params[16];
    if(E(params[17])>=abs(z)) {
        E v=abs(z);
        const int style=int(params[12]);
        if(style<0) v=extendedSinCos(v*params[19]).sin;
        if(style!=1 && style!=-1) v=-v+E(1);
        const E infinity=std::numeric_limits<double>::infinity();
        const E logarithm=v==E(0) ? -infinity : v==infinity ? infinity : extendedLog2(v);
        E angle=E(params[5])*logarithm;
        // The original tests the stored sign bit, including negative zero.
        if(std::signbit(native.transformed.z)) angle=-angle;
        const auto sc=extendedSinCos(angle+params[8]);
        const E sn=double(sc.sin),cs=double(sc.cos);
        const E sector=(extendedAtan2(x*sn+(-(y*cs)),x*cs+y*sn)*params[13])/params[7];
        const E tooth=abs((sector-round(sector))*params[7])/params[0]-E(1);
        distance=-((tooth*params[0])*params[14]);
    }
    const E quadric=(sqrt(abs(((x*x+y*y)+(z*z)*params[10])-params[11]))-params[9])*params[15];
    if(quadric>=distance) distance=quadric;
    const E plane=abs(z)-params[18];
    if(plane>=distance) distance=plane;
    return native.finish(distance,de);
#else
    // Portable arithmetic remains uncertified.
    const double Angle = params[0];             // [PVar - 0x10] default 30.0
    const double Scale = params[1];             // [PVar - 0x20] .2Doubles
    const double Z_add = params[2];             // [PVar - 0x30] .2Doubles
    const double Y_add = params[3];             // [PVar - 0x38]
    const double X_add = params[4];             // [PVar - 0x40]
    const double SpiralSpeed = params[5];       // [PVar - 0x48]
    const int Apply_scale_add = int(params[6]); // [PVar - 0x4c]
    const double RepeatAngle = params[7];       // [PVar - 0x54]
    const double AtanAdd = params[8];           // [PVar - 0x5c]
    const double Thickness = params[9];         // [PVar - 0x64]
    const double zMul = params[10];             // [PVar - 0x6c]
    const double ZeroZ_radius = params[11];     // [PVar - 0x74]
    const int Style = int(params[12]);         // [PVar - 0x78] integer

    const double K_360_OVER_PI = params[13];    // [PVar + 0x00]
    const double K_PI_OVER_360 = params[14];    // [PVar + 0x08]
    const double K_THICKNESS_CONST = params[15];// [PVar + 0x10]
    const double K_NEG01 = params[16];          // [PVar + 0x18]
    const double K_ALMOST1 = params[17];        // [PVar + 0x20]
    const double K_SPHERE_R = params[18];       // [PVar + 0x28]
    const double K_PI_MUL = params[19];         // [PVar + 0x30]

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    double abs_z = fabs(z);
    double spiral_de;

    if (abs_z > K_ALMOST1) {
        spiral_de = K_NEG01;
    } else {

        double z_val;
        if (Style < 0) {
            z_val = sin(abs_z * K_PI_MUL);
            if (Style != -1) {
                z_val = 1.0 - z_val;
            }
        } else if (Style == 1) {
            z_val = abs_z;
        } else {
            z_val = 1.0 - abs_z;
        }

        z_val = max(z_val, 1e-30);  // clamp to avoid log2 of negative/zero
        double log_val = SpiralSpeed * log2(z_val);

        if (z < 0.0) {
            log_val = -log_val;
        }

        double angle_input = log_val + AtanAdd;

        double sc = sin(angle_input);
        double cc = cos(angle_input);

        double term1 = x * sc - y * cc;
        double term2 = x * cc + y * sc;

        double angle = atan2(term1, term2);

        angle *= K_360_OVER_PI;
        double sector = angle / RepeatAngle;

        double frac = sector - rint(sector);

        double frac_angle = frac * RepeatAngle;

        double tooth = fabs(frac_angle) / Angle - 1.0;
        tooth = tooth * Angle * K_PI_OVER_360;
        spiral_de = -tooth;
    }



    double quadric_val = x * x + y * y + z * z * zMul - ZeroZ_radius;
    double quadric_de = sqrt(fabs(quadric_val)) - Thickness;


    quadric_de = (sqrt(fabs(quadric_val)) - Thickness) * K_THICKNESS_CONST;


    double result = max(quadric_de, spiral_de);

    double z_plane = fabs(z) - K_SPHERE_R;

    double shape_de = max(z_plane, result);

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(x, y, z);
#endif
}

cpu_double3 loxodrome2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_LOXODROME2IFS>(p, params, de);
}

constant FormulaInfo formula_info_loxodrome2IFS = {
    .id = FORMULA_LOXODROME2IFS,
    .name = "loxodrome2IFS",
    .param_count = 13,
    .complexity = 327,
    .uses_transcendental = true
};

