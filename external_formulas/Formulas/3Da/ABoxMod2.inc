// ABoxMod2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: ABoxMod2
// Hand-ported from ASM: 202 bytes, Capstone-verified
// DEoption: 2 (analytic DE via w tracking)
// ABox variant with separate xy/z folds + cylinder inversion

constexpr unsigned int FORMULA_ABOXMOD2 = 5079;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void ABoxMod2Original_x87(double*, const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_ABOXMOD2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    ABoxMod2Original_x87(state,params);
    de=state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    auto box = [](E v,E f) { return abs(v+f)-(abs(v-f)+v); };
    const E x=box(E(p.x),E(params[3])), y=box(E(p.y),E(params[3])),
            z=box(E(p.z),E(params[4]));
    E rr;
    if (abs(z) >= E(params[5])) {
        const E cap=abs(z)-params[5];
        rr=(cap*cap+y*y)+x*x;
    } else rr=y*y+x*x;
    const E m=!(rr >= params[2]) ? E(params[1])
              : E(1) >= rr ? E(params[0])/rr : E(params[0]);
    de=double(E(de)*m);
    return cpu_double3(double(x*m+params[6]),double(y*m+params[7]),
                       double(z*m+params[8]));
#else
    // Portable approximation, not certified by the native oracle.
    const double Scale = params[0];
    const double ScaleOverMinR2 = params[1]; // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];           // BOXSCALE pre-computed: sqr(MinR)
    const double Fold_XY = params[3];
    const double Fold_Z = params[4];
    const double Cyl_HalfSize = params[5];

    // Step 1: TgladFold on x and y with Fold_XY
    // v' = |v + Fold| - |v - Fold| - v  (clamp & reflect)
    double x = abs(p.x + Fold_XY) - abs(p.x - Fold_XY) - p.x;
    double y = abs(p.y + Fold_XY) - abs(p.y - Fold_XY) - p.y;

    // Step 2: TgladFold on z with separate Fold_Z
    double z = abs(p.z + Fold_Z) - abs(p.z - Fold_Z) - p.z;

    // Step 3: Cylinder fold — non-conformal inversion
    // z' = |z| - CylHalfSize
    // If z' > 0 (outside cylinder caps): rr = z'² + y² + x²
    // If z' <= 0 (on cylinder body): rr = y² + x² (no z contribution)
    double az = abs(z);
    double rr;
    if (az > Cyl_HalfSize) {
        double zp = az - Cyl_HalfSize;
        rr = zp * zp + y * y + x * x;
    } else {
        rr = y * y + x * x;
    }

    // Step 4: Sphere fold (no VaryScale — uses Scale directly)
    double m;
    if (rr < minR2) {
        m = ScaleOverMinR2;  // uses precomputed Scale/sqr(MinR) from BOXSCALE
    } else if (rr < 1.0) {
        m = Scale / rr;
    } else {
        m = Scale;
    }

    // Step 5: DE tracking (w component)
    de = de * m;

    // Step 6: Apply scaling + C component addition
    // Straight mapping: x += J1 (Cx), y += J2 (Cy), z += J3 (Cz)

    cpu_double3 result;
    result.x = x * m + params[6];
    result.y = y * m + params[7];
    result.z = z * m + params[8];
    return result;
#endif
}

// Alternative direct function implementation
cpu_double3 ABoxMod2_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_ABOXMOD2>(p,params,de);
}

// Formula metadata
constant FormulaInfo formula_info_ABoxMod2 = {
    .id = FORMULA_ABOXMOD2,
    .name = "ABoxMod2",
    .param_count = 5,
    .complexity = 640,
    .uses_transcendental = false
};

/*
Original M3 Description:
A curious ABox non-conformal variation, with a different fold for z and a non-conformal inversion.
Looks like a wardrobe, a phone box... or you name it!

Mathematical Formula:
  TgladFold on x and y with Fold_XY:
    x = |x + Fold_XY| - |x - Fold_XY| - x
    y = |y + Fold_XY| - |y - Fold_XY| - y

  TgladFold on z with separate Fold_Z:
    z = |z + Fold_Z| - |z - Fold_Z| - z

  Cylinder fold (non-conformal):
    z' = |z| - Cyl_HalfSize
    if z' > 0:  rr = z'^2 + y^2 + x^2  (outside cylinder caps)
    else:       rr = y^2 + x^2          (on cylinder body)

  Sphere fold (uses Scale directly, no VaryScale):
    if rr < MinR^2:     m = Scale / MinR^2  (precomputed by BOXSCALE)
    elif rr < 1:        m = Scale / rr
    else:               m = Scale

  Apply + C addition (straight mapping):
    x = x * m (J1)
    y = y * m (J2)
    z = z * m (J3)
    w = w * m  (DE tracking)

Parameters:
  [0] Scale: double (default: 2)
  [1] Min_R: double (default: 0.5) via BOXSCALE
  [2] Fold_XY: double (default: 1)
  [3] Fold_Z: double (default: 1.5)
  [4] Cyl_HalfSize: double (default: 0.5)
  [5] Cx (J1): from TIteration3D (default: 0 in evaluator)
  [6] Cy (J2): from TIteration3D (default: 0 in evaluator)
  [7] Cz (J3): from TIteration3D (default: 0 in evaluator)

PVar layout (verified against ASM offsets):
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = Scale/sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x20] = sqr(MinR) (precomputed by BOXSCALE)
  [PVar - 0x28] = Fold_XY (double)
  [PVar - 0x30] = Fold_Z (double)
  [PVar - 0x38] = Cyl_HalfSize (double)

ASM register mapping:
  eax/@x -> ebx (saved), edx/@y, ecx/@z
  [ecx + 8] = w (DE tracking)
  esi = PIteration3D (from [ebp + 8])
  edi = PVar (from [esi + 0x30])

TIteration3D field access (via esi = PIteration3D):
  [esi + 0x18] = J1 (offset 24) -> added to x
  [esi + 0x20] = J2 (offset 32) -> added to y
  [esi + 0x28] = J3 (offset 40) -> added to z

Key differences from ABoxMod1:
  - Separate fold parameters for xy and z (ABoxMod1 uses single Fold + FoldMod offsets)
  - Cylinder fold instead of standard sphere: z excluded from rr when inside cylinder body
  - No VaryScale — uses plain Scale in all sphere fold branches
  - No FoldMod offsets — standard TgladFold only
  - Straight C mapping (x+=J1, y+=J2, z+=J3) — ABoxMod1 cross-maps (x+=J2, y+=J1)
  - No rotation matrix
*/
