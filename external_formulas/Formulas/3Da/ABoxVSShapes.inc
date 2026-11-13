// ABoxVSShapes: translation of M3Formulas/ABoxVSShapes.m3f, 1450-byte x86 code.
// Shape stack at 00A8 is [z,y,x]; the cap axis is x.
constexpr unsigned int FORMULA_ABOXVSSHAPES = 10188;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxVSShapesOriginal_x87(void*, const double*, int shape);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXVSSHAPES>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    struct State { double x,y,z,de,vary,scratch[4]; int32_t first; };
    static_assert(offsetof(State,first)==72);
    State state{p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT],{},int32_t(params[BFIRSTIT_SLOT])};
    ABoxVSShapesOriginal_x87(&state,params,int(params[5]));
    de=state.de;
    params[DIFS_VARYSCALE_SLOT]=state.vary;
    params[BFIRSTIT_SLOT]=state.first;
    return cpu_double3(state.x,state.y,state.z);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    E previous(params[DIFS_VARYSCALE_SLOT]);
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        previous = E(params[0]);
        params[BFIRSTIT_SLOT] += 1.0;
    }
    // 0029..0039: VaryScale is materialized as Double before the shape.
    const double vary = static_cast<double>(
        previous + (abs(previous) - E(1)) * E(params[4]));
    params[DIFS_VARYSCALE_SLOT] = vary;
    const E fold(params[3]), twice = fold + fold;
    auto box = [&](E v) {
        return v >= fold ? -v + twice : (!(v >= -fold) ? -v - twice : v);
    };
    // 003C..00A5: all three folded coordinates remain on the x87 stack.
    const E x = box(E(p.x)), y = box(E(p.y)), z = box(E(p.z));
    const E xx = x*x, yy = y*y, zz = z*z;
    const E a(params[6]), b(params[7]);
    const int mode = int(params[5]);
    E rr;
    switch (mode) {
    case 1: { // 00AE: torus, with an extended square root.
        const E r = sqrt(zz + yy) - a;
        rr = (r*r + xx)*b;
        break;
    }
    case 2: case 3: {
        const E axis = abs(x);
        const E cap = a >= axis ? (a-axis)*(a-axis) : E(0);
        const E radial = (zz+yy)*b;
        // Keep operand order: unordered chooses cap in mode 2, radial in 3.
        rr = mode == 2 ? (radial >= cap ? radial : cap)
                       : (radial >= cap ? cap : radial);
        break;
    }
    case 4: case 5: case 6: case 7: {
        const E axis = mode <= 5 ? abs(x) : xx;
        const E cap = a >= axis ? a-axis : E(1);
        const E radial = (mode <= 5 ? zz+yy : (yy >= zz ? yy : zz))*b;
        // 0185/01CD/021E/026F compare the cap before any division.
        const bool divide = (mode == 4 || mode == 6)
            ? cap >= radial : !(cap >= radial);
        rr = divide ? radial/cap : radial;
        break;
    }
    case 8:
        rr = (xx*(xx-a) + (zz+yy))*b;
        break;
    case 9: {
        // 0360 is a Double store; the second quadratic sum is still extended.
        const E quartic(static_cast<double>((xx*xx + yy*yy) + zz*zz));
        rr = ((xx+yy)+zz)*b + quartic + a;
        break;
    }
    case 10:
        rr = ((xx*xx + yy*yy) + zz*zz)*b - a;
        break;
    case 11: {
        // 02E8/02EF/02F6 spill the squares only. The cubic term uses the
        // unspilled coordinates and positive-PVar constants (3 and 0.1).
        const E sx(static_cast<double>(xx)), sy(static_cast<double>(yy));
        const E sz(static_cast<double>(zz));
        rr = (((((x*y)*z)*E(params[8]) + (sx*sy)*sz)
              + ((sx*sy + sx*sz) + sz*sy)*a)
              + ((sx+sy)+sz)*a*a + b)*E(params[9]);
        break;
    }
    case 12: {
        E r = (-x)*a + sqrt(zz+yy)*b;
        if (E(0) >= r) r = E(0);
        rr = r*r;
        break;
    }
    case 13:
        rr = ((E(1) >= -x ? -x : E(1))*a + (zz+yy))*b;
        break;
    case 14: {
        const E u = xx*a + yy, v = xx*a + zz;
        rr = (v >= u ? v : u)*b;
        break;
    }
    case 15: {
        const E axis = abs(x);
        const E cap = axis >= a ? (axis-a)*(axis-a) : E(0);
        rr = (zz+yy)*b + cap;
        break;
    }
    case 16: {
        const E xm=x-a, ym=y-a, zm=z-a, xp=x+a, yp=y+a, zp=z+a;
        // 04E3/04FD/0517/0534 round all four distances before MAX comparisons.
        const E d1(static_cast<double>((zz + ym*ym) + xm*xm));
        const E d2(static_cast<double>((zm*zm + yy) + xm*xm));
        const E d3(static_cast<double>((zm*zm + ym*ym) + xx));
        const E d4(static_cast<double>((zp*zp + yp*yp) + xp*xp));
        rr = d2 >= d1 ? d2 : d1;
        rr = d3 >= rr ? d3 : rr;
        rr = d4 >= rr ? d4 : rr;
        rr *= b;
        break;
    }
    case 17: {
        E axis = abs(x);
        axis += axis;
        axis += axis;
        const E ratio = (((xx+xx)+yy)+zz) / axis;
        rr = (ratio*ratio)*b + a;
        break;
    }
    default:
        // 05A5 consumes z as the radius; the last coordinate store underflows.
        rr = z;
        break;
    }
    const E scale = !(rr >= E(params[2])) ? E(params[1])
        : (E(1) >= rr ? E(vary)/rr : E(vary));
    de = static_cast<double>(E(de)*scale);
    if (mode < 1 || mode > 17)
        return cpu_double3(static_cast<double>(numeric::extendedNaN()),
                           static_cast<double>(x*scale+E(params[12])),
                           static_cast<double>(y*scale+E(params[13])));
    return cpu_double3(static_cast<double>(x*scale+E(params[11])),
                       static_cast<double>(y*scale+E(params[12])),
                       static_cast<double>(z*scale+E(params[13])));
#else
    const double scale = params[0], scaleOverMinR2 = params[1], minR2 = params[2];
    const double fold = params[3], variation = params[4];
    const int mode = int(params[5]);
    const double a = params[6], b = params[7];
    double vary = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        vary = scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    vary += variation * (abs(vary) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = vary;
    auto box = [fold](double v) {
        if (v >= fold) return 2.0 * fold - v;
        if (!(v >= -fold)) return -2.0 * fold - v;
        return v;
    };
    const double x = box(p.x), y = box(p.y), z = box(p.z);
    const double xx = x*x, yy = y*y, zz = z*z;
    double rr;
    switch (mode) {
    case 1: { // Torus, 00AE
        const double r = sqrt(zz + yy) - a;
        rr = (r*r + xx)*b;
        break;
    }
    case 2: case 3: { // Cylinders, 00D4 / 011C
        const double cap = a >= abs(x) ? (a-abs(x))*(a-abs(x)) : 0.0;
        const double radial = (zz+yy)*b;
        rr = mode == 2 ? (radial >= cap ? radial : cap)
                       : (radial >= cap ? cap : radial);
        break;
    }
    case 4: case 5: case 6: case 7: {
        // Cones and square pyramids: compare cap with radial BEFORE division.
        const double axis = mode <= 5 ? abs(x) : xx;
        const double cap = a >= axis ? a-axis : 1.0;
        const double radial = (mode <= 5 ? zz+yy : (yy >= zz ? yy : zz))*b;
        const bool divide = (mode == 4 || mode == 6) ? cap >= radial : !(cap >= radial);
        rr = divide ? radial/cap : radial;
        break;
    }
    case 8: // Gerono, 0296
        rr = ((xx-a)*xx + (zz+yy))*b;
        break;
    case 9: { // Pseudo-cube, 034A; quartic sum is stored as Double.
        const double quartic = (xx*xx + yy*yy) + zz*zz;
        rr = ((xx+yy)+zz)*b + quartic + a;
        break;
    }
    case 10: // Pseudo-cube 2, 02BC
        rr = ((xx*xx + yy*yy) + zz*zz)*b - a;
        break;
    case 11: // Pseudo-octahedron, 02E4; squared coordinates are Double stores.
        rr = (((3.0*((x*y)*z) + (xx*yy)*zz)
             + ((xx*yy + xx*zz) + zz*yy)*a)
             + ((xx+yy)+zz)*a*a + b)*0.1;
        break;
    case 12: { // One-sided cone, 0388
        double r = -x*a + sqrt(zz+yy)*b;
        if (0.0 >= r) r = 0.0;
        rr = r*r;
        break;
    }
    case 13: // Paraboloid, 03BC
        rr = ((1.0 >= -x ? -x : 1.0)*a + (zz+yy))*b;
        break;
    case 14: { // Intersection of cylinders: MAXIMUM, 03EE.
        const double u = xx*a + yy, v = xx*a + zz;
        rr = (v >= u ? v : u)*b;
        break;
    }
    case 15: { // Rounded cylinder, 0496
        const double cap = abs(x) >= a ? (abs(x)-a)*(abs(x)-a) : 0.0;
        rr = (zz+yy)*b + cap;
        break;
    }
    case 16: { // Intersection of four spheres, 04CD; four Double stores.
        const double xm = x-a, ym = y-a, zm = z-a;
        const double xp = x+a, yp = y+a, zp = z+a;
        const double d1 = (zz + ym*ym) + xm*xm;
        const double d2 = (zm*zm + yy) + xm*xm;
        const double d3 = (zm*zm + ym*ym) + xx;
        const double d4 = (zp*zp + yp*yp) + xp*xp;
        rr = d2 >= d1 ? d2 : d1;
        rr = d3 >= rr ? d3 : rr;
        rr = d4 >= rr ? d4 : rr;
        rr *= b;
        break;
    }
    case 17: { // Bisphere, 057C. Zero denominator is NOT clamped in x86.
        const double ratio = ((xx+xx)+yy+zz) / (4.0*abs(x));
        rr = (ratio*ratio)*b + a;
        break;
    }
    default:
        // 05A5 jumps to sphere fold without pushing a radius. z is consumed;
        // the final x store underflows the x87 stack (masked invalid => NaN).
        rr = z;
        break;
    }
    // C0 also selects the minimum-radius branch on an unordered comparison.
    const double m = !(rr >= minR2) ? scaleOverMinR2
                   : (1.0 >= rr ? vary/rr : vary);
    de *= m;
    if (mode < 1 || mode > 17)
        return cpu_double3{NAN, x*m+params[12], y*m+params[13]};
    return cpu_double3{x*m+params[11], y*m+params[12], z*m+params[13]};
#endif
}

cpu_double3 ABoxVSShapes_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ABOXVSSHAPES>(p, params, de);
}
constant FormulaInfo formula_info_ABoxVSShapes = {
    .id = FORMULA_ABOXVSSHAPES,
    .name = "ABoxVSShapes",
    .param_count = 10,
    .complexity = 1400,
    .uses_transcendental = true
};
