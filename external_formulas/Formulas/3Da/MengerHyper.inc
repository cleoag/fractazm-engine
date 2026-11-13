// MengerHyper.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MengerHyper
// Hand-ported from ASM: 406 bytes, DEoption=2
// Hyperbolic Menger sponge: FoldingOct + SphereInv + MengerIFS combination

constexpr unsigned int FORMULA_MENGERHYPER = 10177;

// ASM trace summary:
//   0x0D-0x17: Load |z|, |y|, |x| onto stack
//   0x19-0x27: Compute rr = x^2 + y^2 + z^2
//   0x2A-0x6D: First sphere inversion:
//     MaxR_s = MaxRadius * CentRadMul; MinR_s = MinRadius * CentRadMul
//     if rr > MaxR_s: factor = 1.0
//     elif rr >= MinR_s: factor = MaxR_s / rr
//     else: factor = MaxR_s / MinR_s  (clamped)
//     x *= factor, y *= factor, z *= factor
//   0x78-0x9A: Sort descending: max >= mid >= min
//   0x9B-0xAA: Add inversion center: +Y_inv, +X_inv, +Z_inv
//   0xAC-0xBC: Compute rr2 = x^2 + y^2 + z^2 (after offset)
//   0xBD-0xF3: Second sphere inversion:
//     if rr2 > MaxRadius: factor2 = 1.0
//     elif rr2 >= MinRadius: factor2 = MaxRadius / rr2
//     else: factor2 = MaxRadius / MinRadius
//     x = x*factor2 - Y_inv; y = y*factor2 - X_inv; z = z*factor2 - Z_inv
//   0xF6-0x129: If "Fold again" enabled: sort descending again
//   0x130-0x16A: Scale + DE + Menger offset:
//     DE *= Scale; x *= Scale; y *= Scale
//     k = Scale - 1
//     x -= k * CScaleX; y -= k * CScaleY
//     z_fold: kz = (k/Scale)*CScaleZ; z = (kz - |z - kz|) * Scale
//   0x166-0x16A: Store results

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MengerHyperOriginal_x87(double* state, const double* params, int foldAgain);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MENGERHYPER>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x, p.y, p.z, de};
    MengerHyperOriginal_x87(state, params, int(params[10]));
    de = state[3];
    return cpu_double3(state[0], state[1], state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E x=abs(E(p.x)),y=abs(E(p.y)),z=abs(E(p.z));
    const E rr=(x*x+y*y)+z*z,maximum=E(params[7])*params[9],minimum=E(params[8])*params[9];
    E factor=1;
    if(maximum>=rr) factor=maximum/(minimum>=rr ? minimum : rr);
    // FSTP ST(1) leaves z/x/y before the original sort network.
    E a=z*factor,b=x*factor,c=y*factor;
    const auto swap=[](E& u,E& v) { const E saved=u;u=v;v=saved; };
    const auto sort=[&](E& u,E& v,E& w) {
        if(!(u>=v)) swap(u,v);
        if(!(u>=w)) swap(u,w);
        if(!(v>=w)) swap(v,w);
    };
    sort(a,b,c);
    E mid=b+params[5],high=a+params[4],low=c+params[6];
    const E rr2=(mid*mid+high*high)+low*low;
    factor=rr2>=E(params[7]) ? E(1) : E(params[7])/(rr2>=E(params[8]) ? rr2 : E(params[8]));
    mid=mid*factor-params[5];high=high*factor-params[4];low=low*factor-params[6];
    if(int(params[10])!=0) sort(mid,high,low);
    else swap(mid,high);
    const E scale=params[0],k=scale-E(1),kz=(k/scale)*params[3];
    de=double(E(de)*scale);
    mid=mid*scale-k*params[1];high=high*scale-k*params[2];
    low=(kz-abs(low-kz))*scale;
    return cpu_double3(double(mid),double(high),double(low));
#else
    const double Scale     = params[0];    // [PVar-0x10] default: 3
    const double CScaleX   = params[1];    // [PVar-0x18] default: 1
    const double CScaleY   = params[2];    // [PVar-0x20] default: 1
    const double CScaleZ   = params[3];    // [PVar-0x28] default: 0.5
    const double X_inv     = params[4];    // [PVar-0x30] default: -0.75
    const double Y_inv     = params[5];    // [PVar-0x38] default: 0
    const double Z_inv     = params[6];    // [PVar-0x40] default: 0
    const double MaxRadius = params[7];    // [PVar-0x48] default: 0.318
    const double MinRadius = params[8];    // [PVar-0x50] default: 0.048
    const double CentRadMul= params[9];    // [PVar-0x58] default: 0.9
    const int   FoldAgain = int(params[10]); // [PVar-0x5C] integer, default: 1

    // Step 1: Absolute values
    double x0 = abs(p.x);
    double y0 = abs(p.y);
    double z0 = abs(p.z);

    // Step 2: First sphere inversion
    double rr = x0 * x0 + y0 * y0 + z0 * z0;
    double MaxR_s = MaxRadius * CentRadMul;
    double MinR_s = MinRadius * CentRadMul;

    double factor;
    if (rr > MaxR_s) {
        factor = 1.0;
    } else if (rr >= MinR_s) {
        factor = MaxR_s / rr;
    } else {
        factor = MaxR_s / MinR_s;
    }

    double x1 = x0 * factor;
    double y1 = y0 * factor;
    double z1 = z0 * factor;

    // Step 3: The FPU path drops the factor with `fstp st(1)`, leaving the
    // coordinate stack in z/x/y order before the sort network runs.
    double a = z1;
    double b = x1;
    double c = y1;
    if (a < b) { double t = a; a = b; b = t; }
    if (a < c) { double t = a; a = c; c = t; }
    if (b < c) { double t = b; b = c; c = t; }

    // Step 4: Add inversion center offsets to the sorted stack:
    // mid += Y_inv, max += X_inv, min += Z_inv.
    double mid = b + Y_inv;
    double maxv = a + X_inv;
    double minv = c + Z_inv;

    // Step 5: Second sphere inversion
    double rr2 = mid * mid + maxv * maxv + minv * minv;

    double factor2;
    if (rr2 > MaxRadius) {
        factor2 = 1.0;
    } else if (rr2 > MinRadius) {
        factor2 = MaxRadius / rr2;
    } else {
        factor2 = MaxRadius / MinRadius;
    }

    // Scale and subtract inversion center on the same stack order.
    mid = mid * factor2 - Y_inv;
    maxv = maxv * factor2 - X_inv;
    minv = minv * factor2 - Z_inv;

    // Step 6: Optional second fold (sort again if FoldAgain != 0)
    if (FoldAgain != 0) {
        // Final sort runs on the top three stack values and restores max/mid/min.
        if (mid < maxv) { double t = mid; mid = maxv; maxv = t; }
        if (mid < minv) { double t = mid; mid = minv; minv = t; }
        if (maxv < minv) { double t = maxv; maxv = minv; minv = t; }
        // Probe: keep the final sorted order as-is after the second fold.
    } else {
        // Without the second fold, the FPU stack still maps to mid/max/min.
        double t = mid;
        mid = maxv;
        maxv = t;
    }

    // Step 7: Scale + DE tracking + Menger offset
    // ASM at 0x132: fld Scale; fld DE; DE *= Scale; fstp DE
    de = de * Scale;
    maxv *= Scale;
    mid *= Scale;

    // k = Scale - 1
    double k = Scale - 1.0;

    // Menger offsets for x and y
    // The final stores assign mid to X and maxv to Y (ASM 0166..0168).
    maxv -= k * CScaleY;
    mid -= k * CScaleX;

    // Z Menger fold: kz = (k/Scale)*CScaleZ; z_out = (kz - |z - kz|) * Scale
    // ASM at 0x157: k / Scale (fdiv st(1)), * CScaleZ, then fold pattern
    double kz = (k / Scale) * CScaleZ;
    minv = (kz - abs(minv - kz)) * Scale;

    // Store: [edx]=y, [ebx]=x, [ecx]=z
    return cpu_double3(mid, maxv, minv);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_MengerHyper = {
    .id = FORMULA_MENGERHYPER,
    .name = "MengerHyper",
    .param_count = 11,
    .complexity = 1200,
    .uses_transcendental = false
};

/*
Original M3 Description:
A combination of FoldingOct, SphereInv, MengerIFS for hyperbolic
Menger sponges.
Inspired by this post by Buddhi @ FF;
http://www.fractalforums.com/3d-fractal-generation/revenge-of-the-half-eaten-menger-sponge/msg8920/#msg8920

Modified for an accurate escape-time DE and more generalized though.

13 oct 2011

Added "Fold Again" option; it causes a different look in some details,
but it's not easy to spot the differences (if turned off should render
a little little bit faster)

Mathematical Formula:
  1. x=|x|, y=|y|, z=|z|
  2. First sphere inversion:
     rr = x^2 + y^2 + z^2
     MaxR_s = MaxRadius * CentRadMul
     MinR_s = MinRadius * CentRadMul
     factor = clamp(MaxR_s / rr, MaxR_s/MinR_s, 1.0)  (when rr > MaxR_s, factor=1)
     xyz *= factor
  3. Sort descending
  4. Add inversion center offsets
  5. Second sphere inversion (using MaxRadius, MinRadius directly)
     xyz = xyz * factor2 - offset
  6. If "Fold again": sort descending again
  7. Menger sponge: Scale, CScale offsets, Z-fold

Parameters:
  [0]  Scale: double (default: 3)
  [1]  CScale X: double (default: 1)
  [2]  CScale Y: double (default: 1)
  [3]  CScale Z: double (default: 0.5)
  [4]  X inv: double (default: -0.75)
  [5]  Y inv: double (default: 0)
  [6]  Z inv: double (default: 0)
  [7]  MaxRadius: double (default: 0.318)
  [8]  MinRadius: double (default: 0.048)
  [9]  CentRadMul: double (default: 0.9)
  [10] Fold again: integer (default: 1)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScale X
  [PVar-0x20] = CScale Y
  [PVar-0x28] = CScale Z
  [PVar-0x30] = X inv
  [PVar-0x38] = Y inv
  [PVar-0x40] = Z inv
  [PVar-0x48] = MaxRadius
  [PVar-0x50] = MinRadius
  [PVar-0x58] = CentRadMul
  [PVar-0x5C] = Fold again (integer)
*/
