// Mandalay-KIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Mandalay-KIFS
// Hand-ported from ASM: 415 bytes, DEoption=2
// A KIFS (Kaleidoscopic IFS) variant inspired by Mandalay fractal.
// Combines octahedral sort, KIFS edge folding, tower fold,
// conditional branching, ABox-style sphere fold, and rotation matrix.
// C is added from a secondary context pointer (edi-relative offsets).

constexpr unsigned int FORMULA_MANDALAY_KIFS = 10046;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MandalayKIFSOriginal_x87(double* xyzw, const double* params);
#endif

// Original operation order: absolute-value sort; Fold/XY Tower/Z Tower;
// reversed matrix index order; squared radius multiplied by RDiv; sphere
// scale; signed derivative multiplication and Julia C before final stores.
// XY Tower controls the side/top split, including its equality branch.

template<>
cpu_double3 execute_formula<FORMULA_MANDALAY_KIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    MandalayKIFSOriginal_x87(state,params);
    de = state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    E x=abs(E(p.x)), y=abs(E(p.y)), z=abs(E(p.z));
    if (!(x >= y)) std::swap(x,y);
    if (!(x >= z)) std::swap(x,z);
    if (!(y >= z)) std::swap(y,z);
    const E fold=params[3], tower=params[13], ztower=params[14];
    // Original local Double slots at 005A, 006B, 0076 and 0094.
    const E fx=double(x-(fold+fold));
    E xf=double(fold-abs(x-fold)), yf=double(fold-abs(y-fold));
    const E zf=double(!(E(0) >= ztower) ? ztower-abs(z-ztower) : ztower+z);
    if (!(E(0) >= fx) && !(y >= fx)) {
        if (tower+y >= fx) {
            xf=-y;
            yf=double(fold-abs(((x-fold)-fold)-fold));
        } else {
            xf=double(xf+tower);
            yf=double(fold-abs((tower-fold)+y));
        }
    }
    const E rx=(zf*params[10]+yf*params[11])+xf*params[12];
    const E ry=(zf*params[7]+yf*params[8])+xf*params[9];
    const E rz=(zf*params[4]+yf*params[5])+xf*params[6];
    const E rr=((rz*rz+ry*ry)+rx*rx)*params[15];
    const E m=!(rr >= params[2]) ? E(params[1])
              : E(1) >= rr ? E(params[0])/rr : E(params[0]);
    de=double(E(de)*m);
    return cpu_double3(double(rx*m+params[16]),double(ry*m+params[17]),
                       double(rz*m+params[18]));
#else
    // Raw M3 option layout:
    //   .Double Scale = 2
    //   .Boxscale Min R = 0
    //   .Double Fold = 1
    //   .3SingleAngles Rotation1 = 0
    //   .Double XY Tower = 1
    //   .Double Z Tower = 0
    //   .Double RDiv = 1

    const double Scale = params[0];
    const double ScaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];

    const double XYTower = params[13];
    const double ZTower = params[14];
    const double RDiv = params[15];

    // Step 1: Absolute values
    double az = fabs(p.z);
    double ay = fabs(p.y);
    double ax = fabs(p.x);

    // Step 2: Octahedral sort (bubble sort, largest first)
    // After sort: ax >= ay >= az
    if (ax < ay) { double t = ax; ax = ay; ay = t; }
    if (ax < az) { double t = ax; ax = az; az = t; }
    if (ay < az) { double t = ay; ay = az; az = t; }

    // Store sorted back in the direct x/y/z order used by the GLSL reference:
    // x = largest, y = middle, z = smallest.
    double x = ax;
    double y = ay;
    double z = az;

    // Step 3: fx = x - 2*Fold (stored in [ebp-0x10])
    double twoFold = Fold + Fold; // Fold * 2 (ASM: fadd st(0))
    double fx = x - twoFold;

    // Step 4: KIFS fold on x: xf = Fold - |x - Fold|
    double xf = Fold - fabs(x - Fold);

    // Step 5: KIFS fold on y: yf = Fold - |y - Fold|
    double yf = Fold - fabs(y - Fold);

    // Step 6: Z fold
    // ASM at 0x7B: load ZTower ([esi-0x5c] from earlier analysis... actually [esi+0xA4])
    // Compare 0 < ZTower: if ZTower > 0, do fold; else just add
    double zf;
    if (ZTower > 0.0) {
        // zf = ZTower - |z - ZTower|
        zf = ZTower - fabs(z - ZTower);
    } else {
        // zf = ZTower + z
        zf = ZTower + z;
    }

    // Step 7: Conditional branching on fx
    // ASM at 0x9C: load fx, compare 0 < fx
    if (fx > 0.0) {
        // Check if fx > y (compare y vs fx)
        if (fx > y) {
            // Equality belongs to the edge branch.
            if ((XYTower + y) >= fx) {
                // "edges" branch: xf = -y; yf = Fold - |x - 3*Fold|
                xf = -y;
                yf = Fold - fabs(x - 3.0 * Fold);
            } else {
                // "top" branch: xf += XYTower; yf = Fold - |RDiv - Fold + y|
                xf += XYTower;
                yf = Fold - fabs(XYTower - Fold + y);
            }
        }
    }

    // Step 8: Preserve the original reversed matrix indices.
    double rx = (zf * params[10] + yf * params[11]) + xf * params[12];
    double ry = (zf * params[7] + yf * params[8]) + xf * params[9];
    double rz = (zf * params[4] + yf * params[5]) + xf * params[6];

    // Step 9: Standard ABox-style sphere fold.
    double rr = ((rz * rz + ry * ry) + rx * rx) * RDiv;

    double m;
    if (rr < minR2) {
        m = ScaleOverMinR2;
    } else if (rr >= 1.0) {
        m = Scale;
    } else {
        m = Scale / rr;
    }

    // Step 10: DE tracking
    de *= m;

    // Step 11: Scale and add Julia C in the original operation order.
    double out_x = rx * m + params[16];
    double out_y = ry * m + params[17];
    double out_z = rz * m + params[18];

    return cpu_double3(out_x, out_y, out_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_MandalayKIFS = {
    .id = FORMULA_MANDALAY_KIFS,
    .name = "Mandalay-KIFS",
    .param_count = 9,
    .complexity = 415,
    .uses_transcendental = false
};

/*
Original M3 Description:
Attempt to recreate the Mandalex fractal invented by Hiato
and posted at Fractal forums, but using strictly continuous
functions. This means DE is perfect and computation is very fast.
Lots of variations are possible of course, this is
very versatile in particular.
It's a midway between a Menger, an octahedron and an Amazing
Box, with really endless possible fractals!
This formula was born with the precious and essential
help of Knighty, killed all discontinuities and simplified everything.
Speedup version 11/03/2015 by Luca

Parameters:
  [0] Scale: double (default: 2)
  [1] MinR^2: precomputed by BOXSCALE (from Min R = 0)
  [2] Scale/MinR^2: precomputed by BOXSCALE
  [3] Fold: double (default: 1)
  [4..6] Rotation angles: 3 Euler angles in degrees (from .3SingleAngles Rotation1 = 0)
  [7] XY Tower: double (default: 1)
  [8] Z Tower: double (default: 0)
  [9] RDiv: double (default: 1)
  [10..12] Cx, Cy, Cz: Julia constants
  [13] Sphere fold factor

PVar offset layout (esi = PVar):
  [esi-0x10] = Scale
  [esi-0x18] = Scale/MinR^2
  [esi-0x20] = MinR^2
  [esi-0x28] = Fold
  [esi-0x2c..-0x4c] = rotation matrix (9 singles, from .3SingleAngles)
  [esi-0x54] = RDiv
  [esi-0x5c] = Z Tower
  [esi-0x64] = sphere fold factor
*/
