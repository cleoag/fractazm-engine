// Original x87 instruction/store order on Win64; portable fallback is not certified.
constexpr unsigned int FORMULA_CROSSBIFOLD = 10038;

// ASM trace summary:
//   000D-0018: abs(x,y,z) via AND 0x7FFFFFFF
//   001B-0058: R1 rotation on (x,y,z)
//   005A-0078: sort descending (shr ah,1 / jae)
//   007A-008B: de *= Scale; max,mid,min *= Scale; pop Scale
//   008D-0091: store y=max*S, x=mid*S, z=min*S
//   0093-009: x -= (Scale-1)*SubX
//   00A1-00BB: TgladFold: x = |x+F| - |x-F| - x, where F=(Scale-1)*FoldX
//   00BF-00E3: reload z,x,y -> 2nd sort descending
//   00E5-011C: R2 rotation on (mid2', max2', min2)
//   011E-0136: k=Scale-1; cross offsets: rot1-=k*CScaleZ, rot0-=k*CScaleY, rot2=k*CScaleX-rot2
//   0138-013C: store x=rot0-k*CScaleY, y=rot1-k*CScaleZ, z=k*CScaleX-rot2

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CrossBiFoldOriginal_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_CROSSBIFOLD>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[]={fabs(p.x),fabs(p.y),fabs(p.z),de};
    CrossBiFoldOriginal_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    const E x=abs(E(p.x)), y=abs(E(p.y)), z=abs(E(p.z));
    E lo=(x*params[10]+y*params[11])+z*params[12];
    E hi=(x*params[4]+y*params[5])+z*params[6];
    E mid=(x*params[7]+y*params[8])+z*params[9];
    auto sort = [](E &a,E &b,E &c) {
        if (!(a >= b)) std::swap(a,b);
        if (!(a >= c)) std::swap(a,c);
        if (!(b >= c)) std::swap(b,c);
    };
    sort(hi,mid,lo);
    de=double(E(de)*params[0]);
    E xv=double(hi*params[0]), yv=double(mid*params[0]), zv=double(lo*params[0]);
    const E k=E(params[0])-1;
    xv=double(xv-k*params[23]);
    const E f=k*params[22];
    xv=double(abs(xv+f)+(-abs(xv-f)-xv));
    hi=yv; mid=xv; lo=zv;
    sort(hi,mid,lo);
    const E r2=(hi*params[19]+mid*params[20])+lo*params[21];
    const E r0=(hi*params[13]+mid*params[14])+lo*params[15];
    const E r1=(hi*params[16]+mid*params[17])+lo*params[18];
    return cpu_double3(double(r1-k*params[2]),double(r0-k*params[3]),
                       double(k*params[1]-r2));
#else
    const double Scale   = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];
    const double FoldX   = params[22];
    const double SubX    = params[23];

    // Pre-computed rotation matrices from .3SingleAngles (9 floats each in PVar)
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    double R1_00 = R1.m[0][0]; double R1_01 = R1.m[0][1]; double R1_02 = R1.m[0][2];
    double R1_10 = R1.m[1][0]; double R1_11 = R1.m[1][1]; double R1_12 = R1.m[1][2];
    double R1_20 = R1.m[2][0]; double R1_21 = R1.m[2][1]; double R1_22 = R1.m[2][2];

    double R2_00 = R2.m[0][0]; double R2_01 = R2.m[0][1]; double R2_02 = R2.m[0][2];
    double R2_10 = R2.m[1][0]; double R2_11 = R2.m[1][1]; double R2_12 = R2.m[1][2];
    double R2_20 = R2.m[2][0]; double R2_21 = R2.m[2][1]; double R2_22 = R2.m[2][2];

    // Step 1: abs(x,y,z)
    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    // Step 2: R1 rotation on (x, y, z)
    double r0 = ax * R1_00 + ay * R1_01 + az * R1_02;
    double r1 = ax * R1_10 + ay * R1_11 + az * R1_12;
    double r2 = ax * R1_20 + ay * R1_21 + az * R1_22;

    // Step 3: Sort descending
    double max_val = r0, mid_val = r1, min_val = r2;
    if (max_val < mid_val) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (max_val < min_val) { double t = max_val; max_val = min_val; min_val = t; }
    if (mid_val < min_val) { double t = mid_val; mid_val = min_val; min_val = t; }

    // Step 4: Scale + DE tracking (applied before folds)
    de *= Scale;
    double x_val = max_val * Scale;
    double y_val = mid_val * Scale;
    double z_val = min_val * Scale;

    // Step 5: Subtract SubX from x
    // x -= (Scale-1) * SubX
    double k = Scale - 1.0;
    x_val -= k * SubX;

    // Step 6: TgladFold(x, FoldX)
    // F = (Scale-1) * FoldX
    // x_new = |x + F| - |x - F| - x
    double F = k * FoldX;
    x_val = abs(x_val + F) - abs(x_val - F) - x_val;

    // Step 7: Reload and 2nd sort descending
    // ASM loads z, x_new, y and sorts
    double s_max = y_val, s_mid = x_val, s_min = z_val;
    if (s_max < s_mid) { double t = s_max; s_max = s_mid; s_mid = t; }
    if (s_max < s_min) { double t = s_max; s_max = s_min; s_min = t; }
    if (s_mid < s_min) { double t = s_mid; s_mid = s_min; s_min = t; }

    // Step 8: R2 rotation on (mid2, max2, min2)
    double rot0 = s_mid * R2_00 + s_max * R2_01 + s_min * R2_02;
    double rot1 = s_mid * R2_10 + s_max * R2_11 + s_min * R2_12;
    double rot2 = s_mid * R2_20 + s_max * R2_21 + s_min * R2_22;

    // Step 9: Cross-mapped offsets (no additional scaling)
    // ASM: x = rot0 - k*CScaleY (stored to [ebx])
    //      y = rot1 - k*CScaleZ (stored to [edx])
    //      z = k*CScaleX - rot2 (stored to [ecx]) -- REVERSED via FSUBRP
    double x_out = rot0 - k * CScaleY;
    double y_out = rot1 - k * CScaleZ;
    double z_out = k * CScaleX - rot2;

    return cpu_double3(x_out, y_out, z_out);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_CrossBiFold = {
    .id = FORMULA_CROSSBIFOLD,
    .name = "CrossBiFold",
    .param_count = 12,
    .complexity = 1200,
    .uses_transcendental = true
};

/*
Original M3 Description:
Experimental KIFS with mixed folds, for more shape variety.
Formula is long enough, in synthesis:

1) abs(x,y,z)
2) Rot1(x,y,z)
3) foldOct(x,y,z) * Scale
4) x = x - Subx*(Scale-1)
5) x = TgladFold(x,FoldX)
6) foldOct(x,y,z)
7) Rot2(x,y,z)
8) (x,y,z) -= CScalex,y,z*(Scale-1)  [cross-mapped, z reversed]

LucaGN 2012

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 0)
  [2] CScale_Y: double (default: 0)
  [3] CScale_Z: double (default: -1)
  [4..6] Rotation1: 3 angles in degrees (default: 0)
  [7..9] Rotation2: 3 angles in degrees (default: 0)
  [10] Fold X: double (default: 1)
  [11] Sub X: double (default: 3)

PVar layout:
  [PVar-0x10] = Scale
  [PVar-0x18] = CScaleX
  [PVar-0x20] = CScaleY
  [PVar-0x28] = CScaleZ
  [PVar-0x2C..0x4C] = R1 matrix (9 floats)
  [PVar-0x50..0x70] = R2 matrix (9 floats)
  [PVar-0x78] = Fold X
  [PVar-0x80] = Sub X
*/
