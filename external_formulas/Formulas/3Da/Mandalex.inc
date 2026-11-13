// Mandalex.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Mandalex
// Hand-ported from ASM: 536 bytes, DEoption=2
// Modified Mandalex fractal by Hiato with linear pull mechanism.
// Combines box folding with clamped pull, ABox sphere fold,
// rotation matrix, and a secondary pull applied after C addition.
// C is added from context (edi-relative offsets).

constexpr unsigned int FORMULA_MANDALEX = 10049;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MandalexOriginal_x87(double* xyzw, const double* params);
#endif

// ASM trace summary:
//   edi = [ebp+8] + 0x80 (context pointer offset for TIteration3D fields)
//   esi = PVar from context
//   1. Call linearPull function with first pull parameter (Linear Pull)
//   2. Apply Rotation1 (3x3 matrix)
//   3. Box fold with Fold parameter:
//      For each axis: if coord > 2*Fold: coord = -(coord - 2*Fold) + 2*Fold
//                     if coord < -Fold: coord = -(coord + Fold) - Fold
//      Implemented as: if coord >= Fold: new = -coord + 2*Fold
//                      if coord < -Fold: new = -coord - 2*Fold
//   4. Apply Rotation1 again (second rotation multiply)
//   5. Sphere fold: rr = x^2+y^2+z^2, then clamp
//   6. DE tracking: de *= m
//   7. Scale coordinates by m
//   8. Add C from context
//   9. Call linearPull function with second pull parameter (Postlinear Pull)
//
// linearPull subroutine at 0x2A:
//   Count how many of |x|, |y|, |z| are below the pull threshold
//   If count XOR (fiddler & 3) != 0, skip the pull
//   Otherwise: for each axis, subtract sign(coord)*effect*threshold

// Linear pull helper function
static inline void linearPull(double &x, double &y, double &z,
                                double threshold, double effect, int fiddler) {
    // The shipped code has no negative-threshold shortcut.

    // ASM increments the counter when abs(coord) is below the threshold.
    int count = 0;
    if (fabs(x) < threshold) count++;
    if (fabs(y) < threshold) count++;
    if (fabs(z) < threshold) count++;

    // XOR with fiddler lower 2 bits; if nonzero, skip
    if ((count ^ (fiddler & 3)) != 0) return;

    // Apply pull: subtract sign(coord) * effect * threshold
    double pull = threshold * effect;
    // For each axis: if positive, subtract pull; if negative, add pull
    // Original compares 0 with x: signed zero takes the addition branch.
    x = (x <= 0.0) ? (x + pull) : (x - pull);
    y = (y <= 0.0) ? (y + pull) : (y - pull);
    z = (z <= 0.0) ? (z + pull) : (z - pull);
}

template<>
cpu_double3 execute_formula<FORMULA_MANDALEX>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,de};
    MandalexOriginal_x87(state,params);
    de = state[3];
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    // Both pull calls and the first rotation materialize XYZ as Double.
    // The second rotation, sphere fold and C addition keep the x87 values.
    auto pull = [&](double threshold) {
        int count = !(abs(E(p.x)) >= threshold)
                  + int(!(abs(E(p.y)) >= threshold))
                  + int(!(abs(E(p.z)) >= threshold));
        if ((count ^ (int(params[16]) & 3)) != 0) return;
        const E amount = E(threshold) * params[15];
        auto axis = [&](double v) {
            return double(E(v) - ((E(0) >= E(v)) ? -amount : amount));
        };
        p = cpu_double3(axis(p.x), axis(p.y), axis(p.z));
    };
    auto rotate = [&](E &x, E &y, E &z) {
        const E nx = (z*params[10] + y*params[11]) + x*params[12];
        const E ny = (z*params[7] + y*params[8]) + x*params[9];
        const E nz = (z*params[4] + y*params[5]) + x*params[6];
        x=nx; y=ny; z=nz;
    };
    pull(params[13]);
    E x=p.x, y=p.y, z=p.z;
    rotate(x,y,z);
    x=double(x); y=double(y); z=double(z);
    const E fold=params[3], twice=fold+fold;
    auto box = [&](E v) {
        if (v >= fold) return -v+twice;
        if (!(v >= -fold)) return -v-twice;
        return v;
    };
    x=box(x); y=box(y); z=box(z);
    rotate(x,y,z);
    const E rr=(z*z+y*y)+x*x;
    const E m=!(rr >= params[2]) ? E(params[1])
              : E(1) >= rr ? E(params[0])/rr : E(params[0]);
    de=double(E(de)*m);
    p=cpu_double3(double(x*m+params[17]), double(y*m+params[18]),
                  double(z*m+params[19]));
    pull(params[14]);
    return p;
#else
    // Raw M3 option layout:
    //   .Double Scale = 2
    //   .Boxscale Min R = 0
    //   .Double Fold = 1
    //   .3SingleAngles Rotation1 = 0
    //   .Double Linear Pull = 2
    //   .Double Postlinear Pull = 2
    //   .Double Pull Effect = 1
    //   .Integer Pull fiddler = 0

    const double Scale = params[0];
    const double ScaleOverMinR2 = params[1];  // BOXSCALE pre-computed: Scale/sqr(MinR)
    const double minR2 = params[2];            // BOXSCALE pre-computed: sqr(MinR)
    const double Fold = params[3];
    const double LinearPull = params[13];
    const double PostlinearPull = params[14];
    const double PullEffect = params[15];
    const int PullFiddler = int(params[16]);

    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    // Original stack order reverses both matrix indices; use the expanded
    // Single entries directly. The native path also retains x87 intermediates.
    auto rotate = [&](double &x, double &y, double &z) {
        const double nx = (z*params[10] + y*params[11]) + x*params[12];
        const double ny = (z*params[7] + y*params[8]) + x*params[9];
        const double nz = (z*params[4] + y*params[5]) + x*params[6];
        x=nx; y=ny; z=nz;
    };

    double x = p.x;
    double y = p.y;
    double z = p.z;

    linearPull(x, y, z, LinearPull, PullEffect, PullFiddler);

    // Step 2: Apply Rotation1
    rotate(x, y, z);

    // Step 3: Box fold with Fold parameter
    // ASM at 0x10C: fld Fold; dup twice; fadd st(2) -> 2*Fold; fchs -> -Fold
    // For x (loaded from [ebx]): fcom st(2)=Fold
    //   if x >= Fold: x = 2*Fold - x   (= -x + 2*Fold)
    //   else if x < -Fold: x = -x - 2*Fold
    //   else: keep x
    double twoFold = Fold + Fold;
    double negFold = -Fold;

    // x fold (loaded from [ebx])
    if (x >= Fold) {
        x = -x + twoFold;
    } else if (x < negFold) {
        x = -x - twoFold;
    }

    // y fold (loaded from [edx])
    if (y >= Fold) {
        y = -y + twoFold;
    } else if (y < negFold) {
        y = -y - twoFold;
    }

    // z fold (loaded from [ecx])
    if (z >= Fold) {
        z = -z + twoFold;
    } else if (z < negFold) {
        z = -z - twoFold;
    }

    // Step 4: Apply Rotation1 again
    rotate(x, y, z);

    // Step 5: Sphere fold
    // ASM at 0x1B1: compute rr = z^2+y^2+x^2, compare vs MinR^2 and 1.0
    // [esi-0x20] = MinR^2 threshold (if rr < MinR^2: m = Scale/MinR^2)
    // 1.0 threshold (if rr >= 1.0: m = Scale)
    // else: m = Scale/rr (from fdivr [esi-0x10])
    double rr = x * x + y * y + z * z;

    double m;
    if (rr < minR2) {
        m = ScaleOverMinR2;
    } else if (rr >= 1.0) {
        m = Scale;
    } else {
        m = Scale / rr;
    }

    de *= m;

    // Step 7: Scale coordinates.
    z = z * m;
    y = y * m;
    x = x * m;

    // Julia C precedes the second pull, rather than wrapper-side addition.
    x += params[17]; y += params[18]; z += params[19];
    linearPull(x, y, z, PostlinearPull, PullEffect, PullFiddler);

    return cpu_double3(x, y, z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_Mandalex = {
    .id = FORMULA_MANDALEX,
    .name = "Mandalex",
    .param_count = 10,
    .complexity = 536,
    .uses_transcendental = false
};

/*
Original M3 Description:
KNOWN ISSUE:
This fractal has a bad DE, due to the cuts.

Modified version of a fractal invented by Hiato and posted
at Fractal forums. Dropped "Nonlinear pull" as it is C-dependent
and involves integer division, but kept the interesting
"linear pull", in a more customizable version.
Please note that a negative value for pulls disable them
while a negative effect works!

Apply this function before folding and again after adding C:

void linearPull(inout vec3 p, double lp, double effect, int fiddler) {
    int doLP = 0;
    if (abs(p.x)>w) doLP++;
    if (abs(p.y)>h) doLP++;
    if (abs(p.z)>t) doLP++;
    if (fiddler == doLP) {
        lp *= effect;
        if (p.x<0) p.x += lp;
        else p.x -= lp;
        if (p.y<0) p.y += lp;
        else p.y -= lp;
        if (p.z<0) p.z += lp;
        else p.z -= lp;
    }
}

Raw scene options:
  [0] Scale; [1] Min R; [2] Fold; [3..5] Rotation1 angles
  [6] Linear Pull; [7] Postlinear Pull; [8] Pull Effect; [9] Pull fiddler
Expanded params:
  [0] Scale; [1] Scale/MinR^2; [2] MinR^2; [3] Fold
  [4..12] rotation matrix; [13..14] pulls; [15] effect; [16] fiddler
  [17..19] Julia C, appended by the generated context adapter

PVar offset layout (esi = PVar):
  [esi-0x10] = Scale
  [esi-0x18] = Scale/MinR^2  (precomputed by engine for .Boxscale)
  [esi-0x20] = MinR^2        (precomputed by engine for .Boxscale)
  [esi-0x28] = Fold
  [esi-0x2c..-0x4c] = rotation matrix (9 singles)
  [esi-0x54] = Linear Pull
  [esi-0x5c] = Postlinear Pull
  [esi-0x64] = Pull Effect
  [esi-0x68] = Pull fiddler (integer, 4 bytes)
*/
