// gearIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: gearIFS
// Hand-ported from ASM: 230 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 gear tooth shape DE)
// Gear tooth profile: angular quantization → triangular wave → cylinder shell intersection

constexpr unsigned int FORMULA_GEARIFS = 8904;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void GearDistance_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula_dIFS<FORMULA_GEARIFS>(cpu_double3 p, double* params,
                                                    double &varyScale, double &rout) {
    const double Angle = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Cyl_Radius = params[5];
    const int Apply_scale_add = int(params[6]);
    const double RepeatAngle = params[7];
    const double Thick1 = params[8];
    const double ThickZ = params[9];
    const double Cyl_R2 = params[10];
    const double Thick2 = params[11];

    const double K_360_OVER_PI = params[12];
    const double K_PI_OVER_360 = params[13];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        varyScale *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    } else {
        params[DIFS_COMMIT_XYZ_SLOT] = 0.0;
    }

#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, 0.0};
    GearDistance_x87(state, params, Apply_scale_add);
    rout = state[3];
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E sector=(extendedAtan2(E(y),E(x))*K_360_OVER_PI)/RepeatAngle;
    const E tooth=-(((abs((sector-round(sector))*RepeatAngle)/Angle-E(1))*Angle)*K_PI_OVER_360);
    const E radius=sqrt(E(x)*x+E(y)*y);
    const E shell1=abs((radius-Cyl_Radius)+Thick1)-Thick1;
    E distance=shell1>=tooth ? shell1 : tooth;
    const E shell2=abs((E(double(radius))-Cyl_R2)+Thick2)-Thick2;
    if(!(shell2>=distance)) distance=shell2;
    const E plane=abs(E(z))-ThickZ;
    if(plane>=distance) distance=plane;
    if(!Apply_scale_add) distance/=Scale;
    rout=double(distance);
#else
    double angle_720 = atan2(y, x) * K_360_OVER_PI;
    double sector_frac = angle_720 / RepeatAngle;
    double frac = sector_frac - rint(sector_frac);
    double frac_angle = frac * RepeatAngle;
    // Original FDIV, FSUBP, FMUL, FCHS: preserve the Angle==0 NaN path.
    double tooth_offset = -((abs(frac_angle) / Angle - 1.0) * Angle * K_PI_OVER_360);

    double r = sqrt(x * x + y * y);
    double cyl1_dist = abs(r - Cyl_Radius + Thick1) - Thick1;
    double A = max(cyl1_dist, tooth_offset);
    double cyl2_dist = abs(r - Cyl_R2 + Thick2) - Thick2;
    double B = min(A, cyl2_dist);
    double z_dist = abs(z) - ThickZ;
    double shape_de = max(z_dist, B);

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    rout = shape_de;
#endif

    // gearIFS has no OTrap — leave params[MAX_FORMULA_PARAMS-1] at 0
    return cpu_double3(x, y, z);
}

template<>
cpu_double3 execute_formula<FORMULA_GEARIFS>(cpu_double3 p, double* params, double &de) {
    double vs = 1.0, rout = 0.0;
    auto r = execute_formula_dIFS<FORMULA_GEARIFS>(p, params, vs, rout);
    de = rout;
    return r;
}

// Alternative direct function implementation
cpu_double3 gearIFS_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_GEARIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_gearIFS = {
    .id = FORMULA_GEARIFS,
    .name = "gearIFS",
    .param_count = 12,
    .complexity = 470,
    .uses_transcendental = true
};

