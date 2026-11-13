// sliceExIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: sliceExIFS
// Hand-ported from ASM: 334 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + x87 sliced sphere DE)
// Sliced spheres with z-Swirl spirals and spherical checkerboard controls

constexpr unsigned int FORMULA_SLICEEXIFS = 1304;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void SliceExIFSDistance_x87(double*, const double*, int apply, int booleanMode);
#endif

template<>
cpu_double3 execute_formula<FORMULA_SLICEEXIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Angle = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Sph_Radius = params[5];
    const int Apply_scale_add = int(params[6]);
    const double RepeatAngle = params[7];
    const double Thickness = params[8];
    const double z_Swirl = params[9];
    const double z_Angle = params[10];
    const double z_RepeatAngle = params[11];
    const int Or_And_Nor = int(params[12]);

    // Constants (from M3 [CONSTANTS] section)
    const double K_360_OVER_PI = params[13];     // 360/pi — converts radians to 720-scale
    const double K_PI_OVER_360 = params[14];    // pi/360 — converts 720-scale to radians

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;    // w *= Scale
    }

#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={x,y,z,0.,0.,0.};
    SliceExIFSDistance_x87(state,params,Apply_scale_add,Or_And_Nor);
    de=state[5];
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E=numeric::Extended80;
    const auto swirl=numeric::extendedSinCos(E(z)*E(z_Swirl));
    const E s(static_cast<double>(swirl.sin)),c(static_cast<double>(swirl.cos));
    const E ex(x),ey(y),ez(z);
    auto tooth=[&](E angle,double repeat,double width) {
        const E sector=angle*E(K_360_OVER_PI)/E(repeat);
        E value=(abs((sector-round(sector))*E(repeat))/E(width)-E(1))*E(width)*E(K_PI_OVER_360);
        return Or_And_Nor==2 ? value : -value;
    };
    const E xy=tooth(numeric::extendedAtan2(ex*s-ey*c,ex*c+ey*s),RepeatAngle,Angle);
    const E zz=tooth(numeric::extendedAtan2(sqrt(ex*ex+ey*ey),ez),z_RepeatAngle,z_Angle);
    const E combined=(Or_And_Nor!=1 && Or_And_Nor!=2) ? (zz>=xy ? zz : xy) : (zz>=xy ? xy : zz);
    const E shell=abs(sqrt(ex*ex+ey*ey+ez*ez)-E(Sph_Radius)+E(Thickness))-E(Thickness);
    E result=shell>=combined ? shell : combined;
    if(!Apply_scale_add) result/=E(Scale);
    de=static_cast<double>(result);
#else
    // Step 2: z-Swirl rotation (rotates XY plane by z * z_Swirl angle)
    // ASM: fsincos(z * z_Swirl) → store sin/cos to [esp+0x18], [esp+0x20]
    double swirl_sin, swirl_cos;
    swirl_sin = sincos(z * z_Swirl, swirl_cos);

    // Step 3: XY angle with z-Swirl applied
    // ASM: swirl_x = x*cos+y*sin, swirl_y = x*sin-y*cos → fpatan → scale 360/pi
    double swirl_x = x * swirl_cos + y * swirl_sin;
    double swirl_y = x * swirl_sin - y * swirl_cos;
    double xy_angle_720 = atan2(swirl_y, swirl_x) * K_360_OVER_PI;

    // Step 4: XY tooth profile (triangular wave from angular quantization)
    // ASM: fdiv RepeatAngle → frndint → fsubp → fmul → fabs → fdiv Angle → fld1;fsubp → fmul → fmul pi/360
    double xy_sector = xy_angle_720 / RepeatAngle;
    double xy_frac = xy_sector - rint(xy_sector);
    double xy_frac_angle = xy_frac * RepeatAngle;
    double xy_tooth = ((abs(xy_frac_angle) / Angle - 1.0) * Angle) * K_PI_OVER_360;

    // Step 5: Conditional negate for Or/And vs Nor
    // ASM: cmp [edi-0x78], 2; je skip; fchs
    if (Or_And_Nor != 2) {
        xy_tooth = -xy_tooth;   // For Or/And: positive inside tooth
    }

    // Step 6: Z polar angle (angle from z-axis in spherical coords)
    // ASM: sqrt(x²+y²) → fld z → fpatan(r_xy, z) → scale 360/pi
    double r_xy = sqrt(x * x + y * y);
    double z_angle_720 = atan2(r_xy, z) * K_360_OVER_PI;

    // Step 7: Z tooth profile (same triangular wave pattern)
    double z_sector = z_angle_720 / z_RepeatAngle;
    double z_frac = z_sector - rint(z_sector);
    double z_frac_angle = z_frac * z_RepeatAngle;
    double z_tooth = ((abs(z_frac_angle) / z_Angle - 1.0) * z_Angle) * K_PI_OVER_360;

    // Step 8: Conditional negate for Or/And vs Nor (same as XY)
    if (Or_And_Nor != 2) {
        z_tooth = -z_tooth;
    }

    // Step 9: Boolean combination of XY and Z tooth patterns
    // ASM: Or(0)=max @ 0xED, And(1)/Nor(2)=min @ 0xFE
    double combined;
    if (Or_And_Nor != 1 && Or_And_Nor != 2) {
        combined = max(z_tooth, xy_tooth);    // Or = union
    } else {
        combined = min(z_tooth, xy_tooth);    // And/Nor = intersection
    }

    // Step 10: Sphere shell distance (offset shell formula)
    // ASM: sqrt(x²+y²+z²) → fsub Sph_Radius → fadd Thickness → fabs → fsub Thickness
    double r_3d = sqrt(x * x + y * y + z * z);
    double sphere_dist = abs(r_3d - Sph_Radius + Thickness) - Thickness;

    // Step 11: Final shape DE = max(sphere_shell, combined_tooth)
    // ASM: fcom/jae → max
    double shape_de = max(sphere_dist, combined);

    // Step 12: Scale correction
    // ASM: if Apply_scale_add == 0: fdiv Scale
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
#endif

    return Apply_scale_add ? cpu_double3(x,y,z) : p;
}

// Alternative direct function implementation
cpu_double3 sliceExIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_SLICEEXIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_sliceExIFS = {
    .id = FORMULA_SLICEEXIFS,
    .name = "sliceExIFS",
    .param_count = 13,
    .complexity = 820,
    .uses_transcendental = true
};

