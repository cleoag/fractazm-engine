// Amazing Surf.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Amazing Surf
// Hand-ported from ASM: 264 bytes, Capstone-verified
// DEoption: 11 (analytic DE via w tracking)
// Extended API required: VaryScale (iteration state), J1/J2/J3 (C components)

constexpr unsigned int FORMULA_AMAZING_SURF = 1509;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void AmazingSurfOriginal_x87(double* state, const double* params, int cylinder);
#endif

template<>
cpu_double3 execute_formula<FORMULA_AMAZING_SURF>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double scale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        scale = params[0];
        params[BFIRSTIT_SLOT] += 1.0;
    }
    double state[] = {p.x,p.y,p.z,de,scale};
    AmazingSurfOriginal_x87(state,params,int(params[14]));
    de=state[3];
    params[DIFS_VARYSCALE_SLOT]=state[4];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    E previous(params[DIFS_VARYSCALE_SLOT]);
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        previous=E(params[0]);
        params[BFIRSTIT_SLOT] += 1.0;
    }
    const double vary=static_cast<double>(previous+(abs(previous)-E(1))*E(params[13]));
    params[DIFS_VARYSCALE_SLOT]=vary;
    const E fold(params[3]);
    auto box = [&](E v) { return abs(v+fold)-(abs(v-fold)+v); };
    const E fx=box(E(p.x)), fy=box(E(p.y));
    // 005D/0063 are FST stores: radius uses the unrounded folds; the later
    // coordinate loads at 00AC/00B3 use their materialized Double values.
    const double storedX=static_cast<double>(fx), storedY=static_cast<double>(fy);
    E rr=fy*fy+fx*fx;
    if (int(params[14])==0) rr += E(p.z)*E(p.z);
    const E m=!(rr>=E(params[2])) ? E(params[1])
        : (E(1)>=rr ? E(vary)/rr : E(vary));
    de=static_cast<double>(E(de)*m);
    const E x=E(storedX)*m+E(params[15]);
    const E y=E(storedY)*m+E(params[16]);
    const E z=E(p.z)*m+E(params[17]);
    E matrix[9];
    for (int i=0; i<9; ++i) matrix[i]=E(static_cast<float>(params[4+i]));
    return cpu_double3(static_cast<double>((x*matrix[0]+y*matrix[1])+z*matrix[2]),
                       static_cast<double>((x*matrix[3]+y*matrix[4])+z*matrix[5]),
                       static_cast<double>((x*matrix[6]+y*matrix[7])+z*matrix[8]));
#else
    const double Scale = params[0];
    const double ScaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];
    const double Scale_vary = params[13];
    const int SphereOrCylinder = int(params[14]);

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 RM = rotMatrix3FromFloats(params, 4);

    // VaryScale tracking: ASM checks bFirstIt, inits VaryScale=Scale on first call
    // Then: VaryScale = VaryScale + Scale_vary * (|VaryScale| - 1)
    double prevVaryScale = params[DIFS_VARYSCALE_SLOT];
    if (params[BFIRSTIT_SLOT] <= 0.0) {
        prevVaryScale = Scale;  // first iteration: init to Scale (ASM at 0x001D)
        params[BFIRSTIT_SLOT] += 1.0;  // x86 INC preserves negative counter progression
    }
    double VaryScale = prevVaryScale + Scale_vary * (abs(prevVaryScale) - 1.0);
    params[DIFS_VARYSCALE_SLOT] = VaryScale;  // write back for wrapper

    // Box fold (TgladFold) on x and y only — z is NOT folded
    // Formula: v' = |v + Fold| - |v - Fold| - v
    double x = abs(p.x + Fold) - abs(p.x - Fold) - p.x;
    double y = abs(p.y + Fold) - abs(p.y - Fold) - p.y;
    double z = p.z;  // z unchanged

    // Sphere/cylinder radius computation
    double rr = x * x + y * y;
    if (SphereOrCylinder == 0) {
        rr += z * z;  // sphere mode
    }
    // else: cylinder mode (rr = x² + y² only)

    // Sphere fold
    double m;
    if (!(rr >= minR2)) {
        m = ScaleOverMinR2;  // uses pre-computed Scale/MinR² from BOXSCALE
    } else if (rr <= 1.0) {
        m = VaryScale / rr;
    } else {
        m = VaryScale;
    }

    // DE tracking (w component)
    de = de * m;

    // Apply scaling + C component addition (ASM adds C BEFORE rotation)
    // ASM: x = x*m + J1, y = y*m + J2, z = z*m + J3, then rotate
    // C values are appended by wrapper at params[15..17] (after 15 PVar params)
    // C values are appended by wrapper at params[15..17] (after 15 PVar params)
    double x2 = x * m + params[15];  // + J1
    double y2 = y * m + params[16];  // + J2
    double z2 = z * m + params[17];  // + J3

    // Apply rotation — ASM stores: x=row2(params[7..9]), y=row1(params[4..6]), z=row0(params[10..12])
    cpu_double3 rv = RM * cpu_double3(x2, y2, z2);
    return cpu_double3(rv.x, rv.y, rv.z);
#endif
}

// Alternative direct function implementation
cpu_double3 Amazing_Surf_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_AMAZING_SURF>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_Amazing_Surf = {
    .id = FORMULA_AMAZING_SURF,
    .name = "Amazing Surf",
    .param_count = 8,
    .complexity = 830,
    .uses_transcendental = true
};

/*
Original M3 Description:
Formula proposed by Kali by a private message. I added some features btw
Luca GN 2012

Mathematical Formula:
  VaryScale tracking (per-iteration state):
    if first_iteration: VaryScale = Scale
    VaryScale = VaryScale + Scale_vary * (|VaryScale| - 1)

  TgladFold on x and y only (z is NOT folded):
    x = |x + Fold| - |x - Fold| - x
    y = |y + Fold| - |y - Fold| - y

  Sphere/cylinder fold radius:
    if SphereOrCylinder != 0: rr = x² + y²  (cylinder)
    else: rr = x² + y² + z²  (sphere)

  Sphere fold:
    if rr < MinR²:      m = Scale / MinR²  (uses initial Scale, not VaryScale)
    elif rr < 1:         m = VaryScale / rr
    else:                m = VaryScale

  Apply + C addition + rotation:
    x' = x * m (J1)
    y' = y * m (J2)
    z' = z * m (J3)
    [x, y, z] = RotationMatrix * [x', y', z']
    w = w * m  (DE tracking)

Parameters:
  [0] Scale: double (default: 1.5)
  [1] Min_R: double (default: 0.5) via BOXSCALE
  [2] Fold: double (default: 1)
  [3] Rotation_X: double (default: 5) - angle in degrees
  [4] Rotation_Y: double (default: 0) - angle in degrees
  [5] Rotation_Z: double (default: 0) - angle in degrees
  [6] Scale_vary: double (default: 0)
  [7] SphereOrCylinder: int (default: 1)
  [8] Cx (J1): from TIteration3D (default: 0 in evaluator)
  [9] Cy (J2): from TIteration3D (default: 0 in evaluator)
  [10] Cz (J3): from TIteration3D (default: 0 in evaluator)

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = Scale/sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x20] = sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x28] = Fold (double)
  [PVar - 0x2C] = M[0,0]   [PVar - 0x30] = M[0,1]   [PVar - 0x34] = M[0,2]
  [PVar - 0x38] = M[1,0]   [PVar - 0x3C] = M[1,1]   [PVar - 0x40] = M[1,2]
  [PVar - 0x44] = M[2,0]   [PVar - 0x48] = M[2,1]   [PVar - 0x4C] = M[2,2]
  [PVar - 0x54] = Scale_vary (double)
  [PVar - 0x58] = SphereOrCylinder (integer)

TIteration3D field access (via edi = PIteration3D + 0x80):
  [edi + 0x48] = VaryScale (offset 200)
  [edi + 0x50] = bFirstIt (offset 208)
  [edi - 0x68] = J1 (offset 24) -> added to x
  [edi - 0x60] = J2 (offset 32) -> added to y
  [edi - 0x58] = J3 (offset 40) -> added to z

NOTES:
  - VaryScale requires persistent iteration state (not available in basic evaluator)
  - When Scale_vary=0 (default), VaryScale = Scale always -> simplified version is exact
  - M3 description says x+=Cy,y+=Cx but ASM shows x+=J1(Cx),y+=J2(Cy) (straight mapping)
  - For rr < minR2 case, ASM uses constant Scale/minR2 from BOXSCALE, not VaryScale/minR2
  - z is NOT box-folded (only x and y undergo TgladFold)
  - Rotation is applied AFTER scaling and C addition
*/
