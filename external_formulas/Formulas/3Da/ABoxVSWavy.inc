// ABoxVSWavy.inc — Canonical double-precision formula
// The immutable M3F machine code is the parity reference.

// Metal Template for Formula: ABoxVSWavy
// Hand-ported from ASM (375 bytes, Capstone-verified)
// DEoption: 11 (analytic DE via w tracking)
// Extended API required: VaryScale (iteration state), J1/J2/J3 (C components)
//
// ABox Vary Scale with "wavy" sphere inversion: the standard radius r is
// replaced by a trigonometric expression creating wavy surface deformations.
// Suggested by Aexion in FractalForums.

constexpr unsigned int FORMULA_ABOXVSWAVY = 10185;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxVSWavyOriginal_x87(double*, const double*, int, int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXVSWAVY>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    // Include the internal C addition before the original Double stores.
    double state[12] = {p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT]};
    int first=int(params[BFIRSTIT_SLOT]);
    ABoxVSWavyOriginal_x87(state,params,int(params[9]),&first);
    params[DIFS_VARYSCALE_SLOT]=state[4];
    params[BFIRSTIT_SLOT]=first;
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    using mb3d::numeric::extendedSinCos;
    mb3d::numeric::ExtendedScope scope;
    double previous=params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0) {
        previous=params[0];
        params[BFIRSTIT_SLOT]+=1;
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
    const E ax=x*params[6], ay=y*params[6], az=z*params[6];
    auto rangeFailure = [](E a) {
        return (a.signExponent() & 0x7fff) != 0x7fff && abs(a) >= E(0x1p63);
    };
    // An out-of-range FSINCOS makes no push. Its two FSTPs consume a
    // live coordinate. Subsequent duplicates then address the shifted stack;
    // the radius underflows and the unordered branch selects Scale/MinR2.
    if (rangeFailure(ax) || rangeFailure(ay) || rangeFailure(az)) {
        const E nan=mb3d::numeric::extendedNaN(), m=params[1];
        de=double(E(de)*m);
        if (rangeFailure(ax)) return cpu_double3(double(nan),double(nan),double(nan));
        if (rangeFailure(ay)) return cpu_double3(double(nan),double(nan),double(x*m+params[12]));
        return cpu_double3(double(nan),double(x*m+params[11]),double(y*m+params[12]));
    }
    const auto tx=extendedSinCos(ax), ty=extendedSinCos(ay), tz=extendedSinCos(az);
    const double cx=double(tx.cos), sx=double(tx.sin), cy=double(ty.cos),
                 sy=double(ty.sin), cz=double(tz.cos), sz=double(tz.sin);
    const double rho=double(sqrt((z*z+y*y)+x*x));
    const E theta1=((E(cx)*sy+E(cy)*sz)+E(cz)*sx)*params[5];
    const E theta2=((E(cx)+cy)+cz)*params[7];
    const E r=int(params[9]) == 1 ? theta1+(theta2+params[8])*rho
                                 : (theta1+theta2)+E(rho)*params[8];
    const E rr=r*r;
    const E m=!(rr >= params[2]) ? E(params[1])
              : E(1) >= rr ? E(vary)/rr : E(vary);
    de=double(E(de)*m);
    return cpu_double3(double(x*m+params[10]),double(y*m+params[11]),
                       double(z*m+params[12]));
#else
    // Portable fallback remains uncertified.
    const double Scale = params[0];           // Scale
    const double scaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];            // Fold
    const double Scale_vary = params[4];      // Scale vary
    const double WaveAmp = params[5];         // Wave Amp
    const double WaveFrq = params[6];         // Wave Frq
    const double Wave2Amp = params[7];        // Wave2 Amp
    const double RadiusMul = params[8];       // Radius mul
    const int Wave2AltMode = int(params[9]); // Wave2 alt mode (integer)

    // C components from TIteration3D

    // VaryScale tracking — reads evolving VaryScale from params (set by AppendExtState)
    // params[10..13] = J1,J2,J3,J4 (C values), params[DIFS_VARYSCALE_SLOT] = VaryScale, params[15] = bFirstIt
    // nParams after AppendExtState = 10 + 4 + 2 = 16, so VaryScale = params[14]
        double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper to update ext->VaryScale

    // ---------------------------------------------------------------
    // TgladFold on x, y, z (piecewise, same as icen1/icen2)
    // ASM at 0x003C-0x00A5
    // ---------------------------------------------------------------
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

    // Wavy radius (portable approximation of original x87 sequence).
    double fx = x * WaveFrq;
    double fy = y * WaveFrq;
    double fz = z * WaveFrq;

    double cx = cos(fx), sx = sin(fx);
    double cy = cos(fy), sy = sin(fy);
    double cz = cos(fz), sz = sin(fz);

    double rho = sqrt(x * x + y * y + z * z);

    double theta1 = cx * sy + cy * sz + cz * sx;
    double theta2 = cx + cy + cz;

    double r;
    if (Wave2AltMode == 1) {
        // Alt: r = theta1*Amp + rho*(theta2*Amp2 + Rmul)
        r = theta1 * WaveAmp + rho * (theta2 * Wave2Amp + RadiusMul);
    } else {
        // Normal: r = theta1*Amp + theta2*Amp2 + rho*Rmul
        r = theta1 * WaveAmp + theta2 * Wave2Amp + rho * RadiusMul;
    }

    double rr = r * r;  // ASM: fmul st(0) at 0x0127

    // ---------------------------------------------------------------
    // Sphere fold (standard ABox pattern with BOXSCALE)
    // ASM at 0x0129-0x014
    // ---------------------------------------------------------------
    double m_fold;
    if (rr < minR2) {
        m_fold = scaleOverMinR2;
    } else if (rr < 1.0) {
        m_fold = VaryScale / rr;
    } else {
        m_fold = VaryScale;
    }

    // DE tracking
    de = de * m_fold;

    // Apply scaling + C addition (straight mapping)
    // ASM at 0x015D-0x016A:
    //   z'*m + J3/Cz -> [ecx]
    //   y'*m + J2/Cy -> [edx]
    //   x'*m + J1/Cx -> [ebx]
    cpu_double3 result;
    result.z = z * m_fold + params[12];
    result.y = y * m_fold + params[11];
    result.x = x * m_fold + params[10];
    return result;
#endif
}

// Alternative direct function implementation
cpu_double3 ABoxVSWavy_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ABOXVSWAVY>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ABoxVSWavy = {
    .id = FORMULA_ABOXVSWAVY,
    .name = "ABoxVSWavy",
    .param_count = 9,
    .complexity = 980,
    .uses_transcendental = true
};

/*
Original M3 Description:
This formula has been suggested by Aexion in FF. I modified and generalized it a bit.
Same as normal ABox Vary Scale but "r" is replaced by a more complicated expression,
to get new "wavy" effects instead of a normal sphere inversion;

// do the folding, then;
rho = sqrt(x*x+y*y+z*z); // the radius
theta1 = cos(f*x) * sin(f*y) + cos(f*y) * sin(f*z) + cos(f*z) * sin(f*x);
theta2 = cos(f*x) + cos(f*y) + cos(f*z);
if (mulmode == 1) {
r = theta1 * Amp + rho * (theta2 * Amp2 + Rmul);
} else {
r = theta1 * Amp + theta2 * Amp2 + rho * Rmul;
} endif
// use r for the inversion and add c

Parameters:
  [0] Scale: double (default: 2)
  [1] ScaleOverMinR2: BOXSCALE precomputed
  [2] minR2: BOXSCALE precomputed
  [3] Fold: double (default: 1)
  [4] Scale_vary: double (default: 0)
  [5] WaveAmp: double (default: 0.6)
  [6] WaveFrq: double (default: 0.3)
  [7] Wave2Amp: double (default: -0.4)
  [8] RadiusMul: double (default: 1.3)
  [9] Wave2AltMode: integer (default: 0)
  [10] Cx (J1): from TIteration3D
  [11] Cy (J2): from TIteration3D
  [12] Cz (J3): from TIteration3D

NOTES:
  - Setting WaveAmp=0, Wave2Amp=0, RadiusMul=1 gives normal ABoxVS
  - Negative scale often looks better
  - Keep WaveFrq low to avoid overly busy fractal
  - Keep RadiusMul near 1 with low amplitudes for "boxy" appearance
  - Alt mode makes Wave2 interfere with radius multiplier
*/
