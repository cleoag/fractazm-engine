// ABoxVaryScale.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ABoxVaryScale
// Hand-ported from ASM: 290 bytes, Capstone-verified
// DEoption: 11 (analytic DE via w tracking)
// Extended API required: VaryScale (iteration state), J1/J2/J3 (C components)

constexpr unsigned int FORMULA_ABOXVARYSCALE = 10054;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxVaryScaleOriginal_x87(double*,const double*,const float*,int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXVARYSCALE>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT],0.};
    int first=int(params[BFIRSTIT_SLOT]);
    
    ABoxVaryScaleOriginal_x87(state,params,nullptr,&first);
    params[DIFS_VARYSCALE_SLOT]=state[4];
    params[BFIRSTIT_SLOT]=first;
    
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E=numeric::Extended80;
    E previous(params[DIFS_VARYSCALE_SLOT]);
    if(params[BFIRSTIT_SLOT]<=0) {previous=E(params[5]);params[BFIRSTIT_SLOT]+=1;}
    const double vary=static_cast<double>(previous+(abs(previous)-E(1))*E(params[3]));
    params[DIFS_VARYSCALE_SLOT]=vary;
    const E fold(params[2]),twice=fold+fold;
    auto folded=[&](E v) {return v>=fold ? -v+twice : (!(v>=-fold) ? -v-twice : v);};
    const E x=folded(E(p.x)),y=folded(E(p.y)),z=folded(E(p.z));
    const E rr=numeric::extendedLogPower(z*z+y*y+x*x,E(params[4]));
    const E scale=!(rr>=E(params[1])) ? E(params[0]) : (rr<=E(1) ? E(vary)/rr : E(vary));
    de=static_cast<double>(E(de)*scale);
    return cpu_double3(static_cast<double>(x*scale+E(params[6])),
                       static_cast<double>(y*scale+E(params[7])),
                       static_cast<double>(z*scale+E(params[8])));
#else

    const double ScaleOverMinR2 = params[0];
    const double minR2 = params[1];
    const double Fold = params[2];
    const double Scale_vary = params[3];
    const double R_power = params[4];
    const double Scale = params[5]; // Original Scale, before BoxScale expansion.
    const double TwoFold = 2.0 * Fold;

    // VaryScale tracking: ASM checks bFirstIt, inits VaryScale=Scale on first call
    double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;

    // TgladFold (box fold) on x, y, z
    // Clamp-based: if v >= Fold: v = 2*Fold - v; elif v <= -Fold: v = -2*Fold - v
    // Equivalent to: v = |v + Fold| - |v - Fold| - v
    double x = p.x;
    if (x >= Fold) {
        x = TwoFold - x;
    } else if (x <= -Fold) {
        x = -TwoFold - x;
    }

    double y = p.y;
    if (y >= Fold) {
        y = TwoFold - y;
    } else if (y <= -Fold) {
        y = -TwoFold - y;
    }

    double z = p.z;
    if (z >= Fold) {
        z = TwoFold - z;
    } else if (z <= -Fold) {
        z = -TwoFold - z;
    }

    // Radius with R_power: rr = (x^2 + y^2 + z^2)^R_power
    double rr_raw = x * x + y * y + z * z;
    double rr = x87_log_power(rr_raw, R_power);

    // Sphere fold
    double m;
    if (!(rr >= minR2)) { // x87 unordered comparison takes this branch too.
        m = ScaleOverMinR2;  // Pascal BOXSCALE precomputes Scale / MinR^2
    } else if (rr <= 1.0) {
        m = VaryScale / rr;
    } else {
        m = VaryScale;
    }

    // DE tracking (w component)
    de = de * m;

    // C values appended by wrapper: params[6]=J1(Cx), params[7]=J2(Cy), params[8]=J3(Cz)
    // Straight mapping: x+=Cx, y+=Cy, z+=Cz — verified from Pascal ASM
    cpu_double3 result;
    result.x = x * m + params[6];
    result.y = y * m + params[7];
    result.z = z * m + params[8];
    return result;
#endif
}

// Alternative direct function implementation
cpu_double3 ABoxVaryScale_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_ABOXVARYSCALE>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_ABoxVaryScale = {
    .id = FORMULA_ABOXVARYSCALE,
    .name = "ABoxVaryScale",
    .param_count = 6,
    .complexity = 880,
    .uses_transcendental = true
};

/*
Original M3 Description:
  (No inline description in .m3 file)
  Standard ABox with VaryScale and configurable radius power.

Mathematical Formula (traced from ASM):

  VaryScale tracking (per-iteration state):
    if first_iteration: VaryScale = Scale
    VaryScale = VaryScale + Scale_vary * (|VaryScale| - 1)

  TgladFold (box fold) on x, y, z:
    if v >= Fold:   v = 2*Fold - v
    elif v <= -Fold: v = -2*Fold - v
    else: v unchanged
    (equivalent to: v = |v+Fold| - |v-Fold| - v)

  Radius with power:
    rr = (x^2 + y^2 + z^2)^R_power

  Sphere fold:
    if rr < MinR^2:      m = Scale / MinR^2  (uses initial Scale, not VaryScale)
    elif rr <= 1:         m = VaryScale / rr
    else:                 m = VaryScale

  Apply + C addition (straight mapping):
    x = x * m (J1)
    y = y * m (J2)
    z = z * m (J3)
    w = w * m  (DE tracking)

ASM Trace Summary (key instructions):
  0x0014-0x0033: VaryScale init/update (bFirstIt check, VS += Sv*(|VS|-1))
  0x0036-0x009: TgladFold on x,y,z using clamp-based comparisons
  0x00A1-0x00D4: rr = (x^2+y^2+z^2)^R_power using fyl2x/f2xm1/fscale
  0x00D6-0x00F9: Sphere fold (3-way branch: <MinR^2, <=1, >1)
  0x00FC-0x0119: DE update (w*=m), scale xyz, add J1/J2/J3

Parameters:
  [0] Scale: double (default: 2)
  [1] Min_R: double (default: 0.5) via BOXSCALE
  [2] Fold: double (default: 1)
  [3] Scale_vary: double (default: 0)
  [4] R_power: double (default: 1)
  [5] Cx (J1): from TIteration3D (default: 0 in evaluator)
  [6] Cy (J2): from TIteration3D (default: 0 in evaluator)
  [7] Cz (J3): from TIteration3D (default: 0 in evaluator)

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = Scale/sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x20] = sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x28] = Fold (double)
  [PVar - 0x30] = Scale_vary (double)
  [PVar - 0x38] = R_power (double)

TIteration3D field access (via edi = PIteration3D + 0x80):
  [edi + 0x48] = VaryScale (offset 200 from PIteration3D)
  [edi + 0x50] = bFirstIt (offset 208 from PIteration3D)
  [edi - 0x68] = J1 (offset 24 from PIteration3D -> Cx)
  [edi - 0x60] = J2 (offset 32 from PIteration3D -> Cy)
  [edi - 0x58] = J3 (offset 40 from PIteration3D -> Cz)

NOTES:
  - VaryScale requires persistent iteration state (not available in basic evaluator)
  - When Scale_vary=0 (default), VaryScale = Scale always -> simplified version is exact
  - When R_power=1 (default), rr = x^2+y^2+z^2 (standard squared radius)
  - C component mapping is STRAIGHT (x+=Cx, y+=Cy, z+=Cz) - verified from ASM
  - For rr < minR2 case, ASM uses constant Scale/minR2 from BOXSCALE, not VaryScale/minR2
  - The power computation uses x87 fyl2x/f2xm1/fscale pattern for rr^R_power
*/
