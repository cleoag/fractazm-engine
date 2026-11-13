// Immutable original M3F CODE is the numerical parity reference.
constexpr unsigned int FORMULA_ABOXVS_ICEN1 = 10184;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxVS_icen1Original_x87(double*, const double*, int, int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXVS_ICEN1>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT]};
    int first=int(params[BFIRSTIT_SLOT]);
    ABoxVS_icen1Original_x87(state,params,int(params[8]),&first);
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
    const double vary=double(E(previous)+(abs(E(previous))-1)*params[3]);
    params[DIFS_VARYSCALE_SLOT]=vary;
    const E fold=params[2], twice=fold+fold;
    auto box = [&](E v) {
        if (v >= fold) return -v+twice;
        if (!(v >= -fold)) return -v-twice;
        return v;
    };
    const E x=box(E(p.x)), y=box(E(p.y)), z=box(E(p.z));
    const E rr=(z*z+y*y)+x*x, minr2=E(params[1])*params[1];
    // The middle branch divides rr by IR before dividing VaryScale by it.
    const E m=minr2 >= rr ? E(vary)*params[4]/minr2
              : E(params[4]) >= rr ? E(vary)/(rr/params[4]) : E(vary);
    de=double(E(de)*m);
    if (int(params[8]) == 0)
        return cpu_double3(double((x*m+params[9])-params[5]),
                           double((y*m+params[10])-params[6]),
                           double((z*m+params[11])-params[7]));
    return cpu_double3(double((x*m-params[9])+params[5]),
                       double((y*m-params[10])+params[6]),
                       double((z*m-params[11])+params[7]));
#else
    // Portable fallback remains uncertified.
    const double Scale = params[0];           // [PVar - 0x10] = Scale
    const double InvR = params[1];            // [PVar - 0x18] = Inversion radius
    const double Fold = params[2];            // [PVar - 0x20] = Fold
    const double Scale_vary = params[3];      // [PVar - 0x28] = Scale vary
    const double InvR2 = params[4];           // [PVar - 0x30] = sqr(InvR) precomputed? Actually...

    const double MinR = params[1];            // [PVar - 0x18]
    const double Fold2 = params[2];           // [PVar - 0x20] = Fold
    const double ScaleVary = params[3];       // [PVar - 0x28] = Scale vary
    const double IR = params[4];              // [PVar - 0x30] = Inversion radius
    const double InvXC = params[5];           // [PVar - 0x38]
    const double InvYC = params[6];           // [PVar - 0x40]
    const double InvZC = params[7];           // [PVar - 0x48]
    const int CSwapMode = int(params[8]);    // [PVar - 0x4C] integer

    double Cx = params[9];
    double Cy = params[10];
    double Cz = params[11];

        double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double VaryScale = prevVaryScale + ScaleVary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

    double x = p.x;
    double y = p.y;
    double z = p.z;

    if (x >= Fold2) {
        x = 2.0 * Fold2 - x;
    } else if (x < -Fold2) {
        x = -2.0 * Fold2 - x;
    }

    if (y >= Fold2) {
        y = 2.0 * Fold2 - y;
    } else if (y < -Fold2) {
        y = -2.0 * Fold2 - y;
    }

    if (z >= Fold2) {
        z = 2.0 * Fold2 - z;
    } else if (z < -Fold2) {
        z = -2.0 * Fold2 - z;
    }

    double rr = x * x + y * y + z * z;

    double MinR2 = MinR * MinR;
    double m;
    if (rr < MinR2) {
        m = VaryScale * IR / MinR2;
    } else if (rr < IR) {
        m = VaryScale * IR / rr;
    } else {
        m = VaryScale;
    }

    de = de * m;

    cpu_double3 result;
    if (CSwapMode == 0) {
        result.z = z * m + Cz - InvZC;
        result.y = y * m + Cy - InvYC;
        result.x = x * m + Cx - InvXC;
    } else {
        result.z = z * m - Cz + InvZC;
        result.y = y * m - Cy + InvYC;
        result.x = x * m - Cx + InvXC;
    }
    return result;
#endif
}

cpu_double3 ABoxVS_icen1_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ABOXVS_ICEN1>(p, params, de);
}

constant FormulaInfo formula_info_ABoxVS_icen1 = {
    .id = FORMULA_ABOXVS_ICEN1,
    .name = "ABoxVS_icen1",
    .param_count = 12,
    .complexity = 870,
    .uses_transcendental = false
};

/*
Original M3 Description:
From a variation found on Buddhi's Mandelbulber, custom version (conformal).
With MinR and selectable radius; always choose MinR<R, or you will create
discontinuity!
Luca GN 2011

Scale = Scale + Scale_vary*(abs(Scale)-1)
x = abs(x+Fold) - abs(x-Fold) - x   (TgladFold)
y = abs(y+Fold) - abs(y-Fold) - y
z = abs(z+Fold) - abs(z-Fold) - z
if (CSm == 0)
x = x+xC; y = y+yC; z = z+zC   (NOTE: ASM shows these as dead code, not used for rr)
else
x = x+Cx; y = y+Cy; z = z+Cz   (NOTE: also dead code in ASM)
endif
rr = x*x + y*y + z*z
if rr < sqr(Min_R) then m = IR*Scale/sqr(Min_R) else
  if rr < IR then m = IR*Scale/rr else
    m = Scale
endif
if (CSm == 0)
x = x * m + Cx - xC
y = y * m + Cy - yC
z = z * m + Cz - zC
else
x = x * m - Cx + xC
y = y * m - Cy + yC
z = z * m - Cz + zC
endif

Parameters:
  [0] Scale: double (default: 2)
  [1] MinR: double (default: 0.5) - NOT Boxscale!
  [2] Fold: double (default: 1)
  [3] Scale_vary: double (default: 0)
  [4] IR: double (default: 1) - Inversion radius
  [5] InvXC: double (default: 0.5) - Inversion center X
  [6] InvYC: double (default: 0) - Inversion center Y
  [7] InvZC: double (default: 0) - Inversion center Z
  [8] CSwapMode: integer (default: 0)
  [9] Cx (J1): from TIteration3D
  [10] Cy (J2): from TIteration3D
  [11] Cz (J3): from TIteration3D

NOTES:
  - IR replaces 1.0 as outer sphere fold threshold
  - MinR is plain double (no BOXSCALE precomputation)
  - Inversion centers appear in C addition (subtract/add) but NOT in radius
  - CSwapMode flips sign: CSm=0 does +C-Inv, CSm!=0 does -C+Inv
  - Always choose MinR < IR to avoid discontinuities
*/
