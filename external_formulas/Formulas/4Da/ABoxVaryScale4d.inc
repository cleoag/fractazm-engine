// ABoxVaryScale4d.inc - Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ABoxVaryScale4d
// Hand-ported from ASM: 295 bytes, DEoption=5
// 4D ABox formula with varying scale

constexpr unsigned int FORMULA_ABOXVARYSCALE4D = 10325;
template<> struct ExternalFormulaUsesXYZState<FORMULA_ABOXVARYSCALE4D> { static constexpr bool value = true; };

// ASM trace:
//   edi = context+0x80, esi = PVar, ebx = &x
//   Parameters:
//     [esi-0x10] = Scale
//     [esi-0x20] = MinR^2
//     [esi-0x28] = Fold
//     [esi-0x30] = Scale vary
//     [esi-0x34] = R power
//     [esi-0x3C] = W add
//   Context:
//     [edi+0x48] = persistent Scale value
//     [edi+0x50] = firstIt counter
//     [edi-0x68/-0x60/-0x58/-0xB8] = Cx/Cy/Cz/Cw
//
//   First iteration: Scale_current = Scale, bFirstIt becomes 1.
//   Each iteration:
//     Scale_current = Scale_current + Scale_vary * (abs(Scale_current) - 1)
//     Box fold 4D, Sphere fold with optional R_power, add C.

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxVaryScale4dOriginal_x87(double*,const double*,const float*,int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXVARYSCALE4D>(cpu_double3 p, double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={p.x,p.y,p.z,de,params[DIFS_VARYSCALE_SLOT],params[FORMULA_STATE_X_SLOT]};
    int first=int(params[BFIRSTIT_SLOT]);
    const float power=float(params[5]);
    ABoxVaryScale4dOriginal_x87(state,params,&power,&first);
    params[DIFS_VARYSCALE_SLOT]=state[4];
    params[BFIRSTIT_SLOT]=first;
    params[FORMULA_STATE_X_SLOT]=state[5];
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E=numeric::Extended80;
    E previous(params[DIFS_VARYSCALE_SLOT]);
    if(params[BFIRSTIT_SLOT]<=0) {previous=E(params[0]);params[BFIRSTIT_SLOT]+=1;}
    const double vary=static_cast<double>(previous+(abs(previous)-E(1))*E(params[4]));
    params[DIFS_VARYSCALE_SLOT]=vary;
    const E fold(params[3]);
    // Original grouping is |v+F| - (|v-F|+v), with no Double fold store.
    auto folded=[&](E v) {return abs(v+fold)-(abs(v-fold)+v);};
    const E x=folded(E(p.x)),y=folded(E(p.y)),z=folded(E(p.z)),w=folded(E(de));
    E rr=z*z+y*y+x*x+w*w;
    const float power=static_cast<float>(params[5]);
    if(power!=1.0f) rr=numeric::extendedLogPower(rr,E(power));
    const E scale=!(rr>=E(params[2])) ? E(vary)/E(params[2]) : (rr<=E(1) ? E(vary)/rr : E(vary));
    params[FORMULA_STATE_X_SLOT]=static_cast<double>(E(params[FORMULA_STATE_X_SLOT])*scale);
    de=static_cast<double>(w*scale+E(params[6])+E(params[10]));
    return cpu_double3(static_cast<double>(x*scale+E(params[7])),
                       static_cast<double>(y*scale+E(params[8])),
                       static_cast<double>(z*scale+E(params[9])));
#else

    const double Scale = params[0];
    const double MinR2 = params[2];
    const double Fold = params[3];
    const double ScaleVary = params[4];
    const double Rpower = params[5];
    const double Wadd = params[6];
    const double Cx = params[7];
    const double Cy = params[8];
    const double Cz = params[9];
    const double Cw = params[10];

    double scaleCurrent = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        params[BFIRSTIT_SLOT] += 1.0;
        scaleCurrent = Scale;
    }
    scaleCurrent = scaleCurrent + ScaleVary * (abs(scaleCurrent) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = scaleCurrent;

    double x = abs(p.x + Fold) - abs(p.x - Fold) - p.x;
    double y = abs(p.y + Fold) - abs(p.y - Fold) - p.y;
    double z = abs(p.z + Fold) - abs(p.z - Fold) - p.z;
    double w = abs(de + Fold) - abs(de - Fold) - de;

    double rr = x * x + y * y + z * z + w * w;
    if (Rpower != 1.0) {
        rr = pow(rr, Rpower);
    }

    double m = scaleCurrent;
    if (rr < MinR2) {
        m = scaleCurrent / MinR2;
    } else if (rr < 1.0) {
        m = scaleCurrent / rr;
    }

    // x86 00F3..00F8: Deriv1 (context+0xE8) *= signed sphere scale.
    params[FORMULA_STATE_X_SLOT] *= m;
    de = w * m + Wadd + Cw;
    return cpu_double3(x * m + Cx, y * m + Cy, z * m + Cz);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_ABoxVaryScale4d = {
    .id = FORMULA_ABOXVARYSCALE4D,
    .name = "ABoxVaryScale4d",
    .param_count = 7,
    .complexity = 295,
    .uses_transcendental = true
};

/*
Original M3 Description:
A 4d "Amazing Box" (invented by TGlad) aka Mandbox variation,
having a changing scale with iterationcount and a power function
on the sphere folding:

Scale = Scale + Scale_vary*(abs(Scale)-1)
x = abs(x+Fold) - abs(x-Fold) - x
y = abs(y+Fold) - abs(y-Fold) - y
z = abs(z+Fold) - abs(z-Fold) - z
w = abs(w+Fold) - abs(w-Fold) - w
rr = pow(x*x + y*y + z*z + w*w, R_power)
if rr < sqr(Min_R) then m = Scale/sqr(Min_R) else
if rr < 1 then m = Scale/rr else m = Scale
x = x * m + Cx
y = y * m + Cy
z = z * m + Cz
w = w * m + W_add + Cw
*/
