// Immutable original M3F CODE is the numerical parity reference.
constexpr unsigned int FORMULA_ABOXVS_ICEN2 = 10182;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxVS_icen2Original_x87(double*, const double*, int, int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXVS_ICEN2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT]};
    int first=int(params[BFIRSTIT_SLOT]);
    ABoxVS_icen2Original_x87(state,params,int(params[9]),&first);
    de=state[3];
    params[DIFS_VARYSCALE_SLOT]=state[4];
    params[BFIRSTIT_SLOT]=first;
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    double previous=params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0) {
        previous=params[0]; params[BFIRSTIT_SLOT]+=1;
    }
    const double vary=double(E(previous)+(abs(E(previous))-1)*params[4]);
    params[DIFS_VARYSCALE_SLOT]=vary;
    const E fold=params[3], twice=fold+fold;
    auto box = [&](E v) {
        if (v >= fold) return -v+twice;
        if (!(v >= -fold)) return -v-twice;
        return v;
    };
    const E x=box(E(p.x)), y=box(E(p.y)), z=box(E(p.z));
    const bool swap=int(params[9]) != 0;
    const E radius=swap ? ((z+params[11])*z+(y+params[12])*y)+(x+params[10])*x
                        : ((z+params[6])*z+(y+params[7])*y)+(x+params[8])*x;
    const E rr=mb3d::numeric::extendedLogPower(radius,E(params[5]));
    const E m=!(rr >= params[2]) ? E(params[1])
              : E(1) >= rr ? E(vary)/rr : E(vary);
    de=double(E(de)*m);
    // The original non-swap tail uses J2/J3/J4, not J1/J2/J3.
    const int c=swap ? 6 : 11;
    return cpu_double3(double(x*m+params[c]),double(y*m+params[c+1]),
                       double(z*m+params[c+2]));
#else
    // Portable fallback remains uncertified.
    const double Scale = params[0];           // Scale
    const double ScaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];            // Fold
    const double Scale_vary = params[4];      // Scale vary
    const double Rpower = params[5];          // R power
    const double InvXC = params[6];           // Inversion xC
    const double InvYC = params[7];           // Inversion yC
    const double InvZC = params[8];           // Inversion zC
    const int CSwapMode = int(params[9]);    // C-Swap mode (integer)


        double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

    double x = p.x;
    double y = p.y;
    double z = p.z;

    if (x >= Fold) {
        x = 2.0 * Fold - x;
    } else if (x < -Fold) {
        x = -2.0 * Fold - x;
    }

    if (y >= Fold) {
        y = 2.0 * Fold - y;
    } else if (y < -Fold) {
        y = -2.0 * Fold - y;
    }

    if (z >= Fold) {
        z = 2.0 * Fold - z;
    } else if (z < -Fold) {
        z = -2.0 * Fold - z;
    }




    double rx, ry, rz;
    if (CSwapMode == 0) {
        rx = x + InvZC;   // x (bottom of stack) + InvZC
        ry = y + InvYC;
        rz = z + InvXC;   // z (top of stack) + InvXC
    } else {
        rx = x + params[10];
        ry = y + params[12];
        rz = z + params[11];
    }

    double S = rz * z + ry * y + rx * x;
    double rr = x87_log_power(S, Rpower);

    double m;
    if (!(rr >= minR2)) { // x87 unordered comparison takes this branch too.
        m = ScaleOverMinR2;
    } else if (rr < 1.0) {
        m = VaryScale / rr;
    } else {
        m = VaryScale;
    }

    de = de * m;

    cpu_double3 result;
    if (CSwapMode == 0) {
        result.z = z * m + params[13];
        result.y = y * m + params[12];
        result.x = x * m + params[11];
    } else {
        result.z = z * m + InvZC;
        result.y = y * m + InvYC;
        result.x = x * m + InvXC;
    }
    return result;
#endif
}

cpu_double3 ABoxVS_icen2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ABOXVS_ICEN2>(p, params, de);
}

constant FormulaInfo formula_info_ABoxVS_icen2 = {
    .id = FORMULA_ABOXVS_ICEN2,
    .name = "ABoxVS_icen2",
    .param_count = 9,
    .complexity = 880,
    .uses_transcendental = true
};

/*
Original M3 Description:
From a variation found on Buddhi's Mandelbulber, custom version (non conformal).
Luca GN 2011

x = abs(x+Fold) - Fold      (actually TgladFold from ASM)
y = abs(y+Fold) - Fold
z = abs(z+Fold) - Fold
if (CSm == 0)
x1 = x+xC; y1 = y+yC; z1 = z+zC
else
x1 = x; y1 = y; z1 = z
endif
rr = x1*x1 + y1*y1 + z1*z1
if rr < 1 then m = Scale/rr else m = Scale
if (CSm == 0)
x = x * m
y = y * m
z = z * m
else
x = x * m + xC
y = y * m + yC
z = z * m + zC
endif

Parameters:
  [0] Scale: double (default: 2)
  [1] ScaleOverMinR2: BOXSCALE precomputed
  [2] minR2: BOXSCALE precomputed (sqr(MinR), default MinR=0.5)
  [3] Fold: double (default: 1)
  [4] Scale_vary: double (default: 0)
  [5] Rpower: double (default: 1) - power applied to radius
  [6] InvXC: double (default: 0.5) - Inversion center X
  [7] InvYC: double (default: 0) - Inversion center Y
  [8] InvZC: double (default: 0) - Inversion center Z
  [9] CSwapMode: integer (default: 0) - swaps C source for radius vs addition
  [10] Cx (J1): from TIteration3D
  [11] Cy (J2): from TIteration3D
  [12] Cz (J3): from TIteration3D

NOTES:
  - CSwapMode swaps which constants go to radius computation vs C addition
  - R power applies pow() to the radius before sphere fold comparison
  - Uses standard BOXSCALE sphere fold (minR2, 1.0 comparison thresholds)
  - TgladFold on all 3 axes (verified from ASM piecewise comparisons)
*/
