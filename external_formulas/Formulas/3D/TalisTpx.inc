// Original x87 instruction/store order on Win64; portable fallback is not certified.
constexpr unsigned int FORMULA_TALITTPX = 10317;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void TalisTpxOriginal_x87(double*, const double*, int power);
#endif

template<>
cpu_double3 execute_formula<FORMULA_TALITTPX>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[12]={p.x,p.y,p.z};
    TalisTpxOriginal_x87(state,params,int(params[7]));
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E=numeric::Extended80;
    const numeric::ExtendedScope scope;
    const auto store=[](E value) {return E(double(value));};
    const E x(p.x),y(p.y),z(p.z),zmul(params[0]);
    E px=x,py=y,pz=z;
    int power=int(params[7]);
    if(power<2 || power>22) power=8;
    const E originalRadius=store(sqrt(x*x+y*y));
    for(int i=1;i<power;++i) {
        const E radius=sqrt(px*px+py*py);
        // CODE 005D stores radius without popping: Z uses the extended
        // result, while the two separate divisions reload its Double copy.
        const E nextZ=(radius*z-originalRadius*pz)*zmul;
        const E factor=store(-((pz*z/originalRadius)/store(radius))+E(1));
        const E nextX=(x*px-y*py)*factor;
        const E nextY=(y*px+x*py)*factor;
        px=store(nextX);py=store(nextY);pz=store(nextZ);
    }
    const E a1(params[4]),a2(params[5]),a3(params[6]);
    const E cx(params[8]),cy(params[9]),cz(params[10]);
    px=store(px*a1+(x*a2+cx*a3));
    py=store(py*a1+(y*a2+cy*a3));
    pz=store(pz*a1+(z*a2+cz*a3));
    const E qx=store(E(params[1])+x),qy=store(E(params[2])+y),qz=store(E(params[3])+z);
    const E square=store(qx*qx+(qy*qy+qz*qz));
    const E numeratorRadius=store(sqrt(px*px+py*py));
    const E denominatorRadius=store(sqrt(qx*qx+qy*qy));
    const E outZ=store(((denominatorRadius*pz-numeratorRadius*qz)/square)*zmul);
    const E factor=store((((pz*qz/denominatorRadius)/numeratorRadius)+E(1))/square);
    const E outX=store((px*qx+py*qy)*factor);
    const E outY=store((qx*py-px*qy)*factor);
    // The division results are already Double when (1-a3)*C is added.
    const E complement=E(1)-a3;
    return cpu_double3(double(cx*complement+outX),double(cy*complement+outY),
                       double(cz*complement+outZ));
#else
    const double zmul  = params[0];   // Z mul, default -1
    const double p3dx  = params[1];   // default 1
    const double p3dy  = params[2];   // default 0
    const double p3dz  = params[3];   // default 0
    const double a1    = params[4];   // default 1
    const double a2    = params[5];   // default 0
    const double a3    = params[6];   // default 0
    int   ipow  = int(params[7]);    // Int pow, default 8
    const double Cx    = params[8];
    const double Cy    = params[9];
    const double Cz    = params[10];

    // Clamp power to [2, 22]
    if (ipow > 22 || ipow < 2) ipow = 8;

    double x = p.x;
    double y = p.y;
    double z = p.z;

    // Save original coordinates
    double x1 = x, y1 = y, z1 = z;

    // Compute initial r1 = sqrt(x^2 + y^2)
    double r1 = sqrt(x * x + y * y);

    // Power loop: raise (x1,y1,z1) to ipow-th power using triplex multiplication
    for (int i = 0; i < ipow - 1; i++) {
        // r2 = sqrt(x1^2 + y1^2)
        double r2 = sqrt(x1 * x1 + y1 * y1);

        // New z1: z1' = r2*z - r1*z1, then *= zmul
        double new_z1 = (r2 * z - r1 * z1) * zmul;

        // R2 factor: R2 = 1 - z1_old*z/(r1*r2)
        // ASM: fld z1, fmul z, fdiv r1, fdiv r2, fchs, fld1, faddp
        // => -(z1*z)/(r1*r2) + 1 = 1 - z1*z/(r1*r2)
        double R2_factor = 1.0 - (z1 * z) / (r1 * r2 + 1e-30);

        // Triplex xy multiplication:
        // x1' = (x*x1 - y*y1) * R2
        // y1' = (y*x1 + x*y1) * R2
        // But ASM at 0x87-0xA7 shows:
        // fld [ebx]; fmul [ebp-8] => x * x1
        // fld [edx]; fmul [ebp-0x10] => y * y1
        // fsubp => x*x1 - y*y1
        // fmul R2 => new_x1
        //
        // fld [edx]; fmul [ebp-8] => y * x1
        // fld [ebx]; fmul [ebp-0x10] => x * y1
        // faddp => y*x1 + x*y1
        // fmul R2 => new_y1

        double new_x1 = (x * x1 - y * y1) * R2_factor;
        double new_y1 = (y * x1 + x * y1) * R2_factor;

        x1 = new_x1;
        y1 = new_y1;
        z1 = new_z1;
    }

    // ASM includes the a3*C contribution in the numerator and then adds
    // the complementary (1-a3)*C term after the triplex division.
    double nx = x1 * a1 + x * a2 + Cx * a3;
    double ny = y1 * a1 + y * a2 + Cy * a3;
    double nz = z1 * a1 + z * a2 + Cz * a3;

    // Denominator: (p3d + original_z) using triplex division
    double x2 = p3dx + x;
    double y2 = p3dy + y;
    double z2 = p3dz + z;

    double R = x2 * x2 + y2 * y2 + z2 * z2;
    double r1_num = sqrt(nx * nx + ny * ny);
    double r2_den = sqrt(x2 * x2 + y2 * y2);

    // Triplex division result:
    // z_out = (r2_den * nz - r1_num * z2) / R * zmul
    double z_out = (r2_den * nz - r1_num * z2) / (R + 1e-30) * zmul;

    // R_factor = (1 + nz*z2/(r1_num*r2_den)) / R
    double R_factor = (1.0 + (nz * z2) / (r1_num * r2_den + 1e-30)) / (R + 1e-30);

    // x_out = (nx*x2 + ny*y2) * R_factor
    double x_out = (nx * x2 + ny * y2) * R_factor;

    // y_out: ASM uses FSUBRP => st(1) = st(0) - st(1)
    // fld x1, fmul y2 => x1*y2 (at this point x1 is really nx)...
    // Actually at 0x1A9:
    // fld [ebp-8] = x1_final (after coefficients)
    // fmul [ebp-0x28] = * y2
    // fld [ebp-0x20] = x2
    // fmul [ebp-0x10] = x2 * y1_final
    // DE E1 = fsubrp st(1) => st(1) = st(0) - st(1) = (x2*ny) - (nx*y2)
    //   Wait: FSUBRP st(1): st(1) = st(0) - st(1), pop
    //   st0 = x2*ny, st1 = nx*y2
    //   result = x2*ny - nx*y2
    // Then * R_factor
    double y_out = (x2 * ny - nx * y2) * R_factor;

    double one_minus_a3 = 1.0 - a3;
    x_out += Cx * one_minus_a3;
    y_out += Cy * one_minus_a3;
    z_out += Cz * one_minus_a3;

    return cpu_double3(x_out, y_out, z_out);
#endif
}

// Alternative direct function
cpu_double3 TalisTpx_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TALITTPX>(p,params,de);
}

// Formula metadata
constant FormulaInfo formula_info_TalisTpx = {
    .id = FORMULA_TALITTPX,
    .name = "TalisTpx",
    .param_count = 8,
    .complexity = 493,
    .uses_transcendental = false
};

/*
Original M3 Description:
Talis-like 3D set, based on D. White's new "triplex formulas".
The base are the fast non-trig formulas, reported by Bugman's site.
( http://bugman123.com/Hypercomplex/index.html )
This formula comes from Fractal Explorer, and the D.H. Van den Berghe
Ultra Fractal version made in 2010. Added some "extra" parameters.
Too bad, this implementation does not look good in 3D, because Talis
uses a fractional formula. 2D sections at z=0 still show the original fractal.
Maybe, playing with the parameters you can find something interesting?
Tip. Try to make hybrids with Magnet.
This implementation is by Luca GN 2011

Formula: z = (a1*z^pow + a2*z + a3*c) / (z + p3d) + (1-a3)*pixel
  With triplex (non-trig) multiplication for z^pow and division.

Parameters:
  [0] Z mul: double (default: -1) — z-axis multiplication in triplex product
  [1] p3dx: double (default: 1)
  [2] p3dy: double (default: 0)
  [3] p3dz: double (default: 0)
  [4] a1: double (default: 1)
  [5] a2: double (default: 0)
  [6] a3: double (default: 0)
  [7] Int pow: integer (default: 8, clamped to [2,22])

Note: Version=5 formula with .DEscale=1.5, .SIpower=2.
The integer power loop uses triplex multiplication (non-trig method).
*/
