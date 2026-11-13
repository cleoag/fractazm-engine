// Preserve original byte decrement loop (zero means 256) and x87 spills.
// Portable fallback is not certified by native oracle evidence.
constexpr unsigned int FORMULA_BT1PINE = 2907;

// ASM trace (299 bytes):
//
// STEP 1: Benesi Transform 1 (repeated T1_Count times)
//   Let c1 = sqrt(2/3), s1 = sqrt(1/3), h = sqrt(1/2)
//
//   Forward rotation:
//     tx = (x*c1 - z*s1) * h
//     z  = |x*s1 + z*c1|         (abs fold)
//
//   Abs fold:
//     x = |tx - y*h|
//     y = |tx + y*h|
//
//   Reverse rotation:
//     tx2 = (x + y) * h
//     y   = (y - x) * h
//     x   = tx2*c1 + z*s1
//     z   = z*c1 - tx2*s1
//
//   Scale/offset:
//     x = x*Scale - xOff
//     y = y*Scale
//     z = z*Scale
//
//   Loop: dec T1_Count; jne back to start
//
// STEP 2: Benesi pinetree (power-2) — identical to BT2Pine
//     xt = x², yt = y², zt = z²
//     t = 2*x / sqrt(yt + zt)
//     x = xt - yt - zt + Cx * pixel_x
//     y = 2*t*z*y + Cy * pixel_z
//     z = t*(yt - zt) + Cz * pixel_y
//
// C handling: Formula handles C internally (DEoption=0).
//   pixel_x/y/z select which C components contribute.
//   Default (1,0,0): only Cx added to x output.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BT1PineOriginal_x87(double*, const double*, int);
#endif

template<>
cpu_double3 execute_formula<FORMULA_BT1PINE>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,0.,0.,0.,0.};
    BT1PineOriginal_x87(state,params,int(params[5]));
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    E x=p.x,y=p.y,z=p.z;
    const E c=params[6],s=params[7],h=params[8];
    const int count=int(params[5])&0xff;
    for(int i=0;i<(count ? count : 256);++i) {
        const E t=double((x*c+(-(z*s)))*h);
        const E zz=double(abs(x*s+z*c));
        const E xx=double(abs(t+(-(y*h))));
        y=double(abs(t+y*h)); x=xx; z=zz;
        const E reverse=double((x+y)*h);
        const E yy=double((-x+y)*h);
        const E rx=double(reverse*c+z*s);
        z=double(-(reverse*s)+z*c); x=rx; y=yy;
        x=double(x*params[0]-params[1]);
        y=double(y*params[0]); z=double(z*params[0]);
    }
    const E xx=double(x*x),yy=double(y*y),zz=double(z*z);
    const E t=double((x+x)/sqrt(yy+zz));
    return cpu_double3(double((xx-yy)-zz+E(params[9])*params[2]),
                       double(((t+t)*z)*y+E(params[10])*params[4]),
                       double(t*(yy-zz)+E(params[11])*params[3]));
#else
    const double Scale    = params[0];   // [edi-0x10] (default: 2.0)
    const double xOff     = params[1];   // [edi-0x18] (default: 2.0)
    const double pixel_x  = params[2];   // [edi-0x20] (default: 1.0)
    const double pixel_y  = params[3];   // [edi-0x28] (default: 0.0)
    const double pixel_z  = params[4];   // [edi-0x30] (default: 0.0)
    const int   T1_Count = int(params[5]) & 0xFF;  // [edi-0x34] byte (default: 1)
    // Positive PVar constants are part of the generated param map.
    const double c1       = params[6];   // [edi+0x00] sqrt(2/3)
    const double s1       = params[7];   // [edi+0x08] sqrt(1/3)
    const double h        = params[8];   // [edi+0x10] sqrt(1/2)
    // C values are appended after the mapped params.
    const double Cx       = params[9];
    const double Cy       = params[10];
    const double Cz       = params[11];

    double x = p.x, y = p.y, z = p.z;

    // ===== STEP 1: Benesi Transform 1 (looped) =====
    for (int iter = 0; iter < (T1_Count ? T1_Count : 256); iter++) {
        // Forward rotation
        double tx = (x * c1 - z * s1) * h;
        z = fabs(x * s1 + z * c1);

        // Abs fold
        double new_x = fabs(tx - y * h);
        double new_y = fabs(tx + y * h);
        x = new_x;
        y = new_y;

        // Reverse rotation
        double tx2 = (x + y) * h;
        y = (y - x) * h;
        x = tx2 * c1 + z * s1;
        z = z * c1 - tx2 * s1;

        // Scale and offset
        x = x * Scale - xOff;
        y = y * Scale;
        z = z * Scale;
    }

    // ===== STEP 2: Benesi pinetree (power-2) =====
    double xt = x * x;
    double yt = y * y;
    double zt = z * z;

    double t = 2.0 * x / sqrt(yt + zt);

    double new_x = xt - yt - zt + Cx * pixel_x;
    double new_y = 2.0 * t * z * y + Cy * pixel_z;
    double new_z = t * (yt - zt) + Cz * pixel_y;

    return cpu_double3(new_x, new_y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_BT1Pine = {
    .id = FORMULA_BT1PINE,
    .name = "BT1Pine",
    .param_count = 9,
    .complexity = 299,
    .uses_transcendental = true
};

