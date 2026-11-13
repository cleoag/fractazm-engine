// PSphereIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PSphereIFS
// Hand-ported from ASM: 274 bytes, DEoption=20
// Tractrix pseudosphere surface with axis swap and upside-down options
// Uses ln() and sqrt() for tractrix parametrization

constexpr unsigned int FORMULA_PSPHEREIFS = 10079;

// ASM trace (x87 section after SSE2 preamble, 0x4B onward):
//   0x4B: cmp [edi-0x60], 1 → Flip_XZ_YZ == 1 → swap x,z
//   0x5: cmp [edi-0x60], -1 → Flip_XZ_YZ == -1 → swap y,z
//   0x75: cmp [edi-0x64], 0 → Upside_down != 0 → negate z
//   0x85: x² + y² → [esp+8] = x²+y²; sqrt → R = sqrt(x²+y²) → [esp]
//   0x9B: load a = PseudoRadius; a² - (x²+y²)
//   0xA4: fld st(0); fabs; faddp → val + |val| = max(0, val)*2
//         (clamps a²-R² to >= 0)
//   0xAA: sqrt → n = sqrt(max(0, a²-R²))
//   0xAC: fld st(0); fadd a; fdiv R → (n+a)/R
//   0xB4: fldln2; fxch; fyl2x → ln((n+a)/R)  [fyl2x = st1*log2(st0)]
//   0xBA: fmul a → a * ln((n+a)/R)
//   0xBD: fxch st1; fchs; faddp → a*ln((n+a)/R) - n
//   0xC3: fsub z → tractrix_de = (a*ln((n+a)/R) - n) - z
//   0xC7: fabs → |tractrix_de|
//   0xC9: fsub Thickness → |tractrix_de| - Thickness
//   0xCC: load R; fmul [edi]; fadd [edi+8]; fabs → |(R * const0 + const1)|
//   0xD4: fmulp → DE_fix = |tractrix_de - Thickness| * |(R*c0+c1)|
//   0xD8: load R; fsub MinRadius → R - MinRadius
//   0xDE-0xEB: min(current, R - MinRadius) [cylinder clip from below]
//   0xED: load R; fsubr MaxRadius → MaxRadius - R
//   0xF3-0x100: min(current, MaxRadius - R) [cylinder clip from above]
//   0x102: Scale correction + store

template<>
cpu_double3 execute_formula<FORMULA_PSPHEREIFS>(cpu_double3 p, constant double* params, double &de) {
    const double PseudoRadius = params[0];
    const double Scale        = params[1];
    const double Z_add        = params[2];
    const double Y_add        = params[3];
    const double X_add        = params[4];
    const double MaxRadius    = params[5];
    const int   Apply_scale_add = int(params[6]);
    const double Thickness    = params[7];
    const double MinRadius    = params[8];
    const int   Flip_XZ_YZ   = int(params[9]);
    const int   Upside_down  = int(params[10]);
    const double const0       = params[11];  // [edi+0] = 0.3
    const double const1       = params[12];  // [edi+8] = 0.02

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Axis swaps
    // ASM: Flip_XZ_YZ == 1 → swap(x, z); Flip_XZ_YZ == -1 → swap(y, z)
    if (Flip_XZ_YZ == 1) {
        double tmp = x;
        x = z;
        z = tmp;
    }
    if (Flip_XZ_YZ == -1) {
        double tmp = y;
        y = z;
        z = tmp;
    }

    // Step 3: Upside-down
    if (Upside_down != 0) {
        z = -z;
    }

    // Step 4: Tractrix pseudosphere DE
    double R_sq = x * x + y * y;          // x² + y²
    double R = sqrt(R_sq);                 // cylindrical radius

    double a = PseudoRadius;
    double diff = a * a - R_sq;            // a² - R²
    // Clamp to >= 0: val + |val| = 2*max(0, val), then sqrt
    // ASM: fld st(0); fabs; faddp → diff + |diff|
    double clamped = diff + fabs(diff);    // = 2 * max(0, diff)
    double n = sqrt(clamped);              // n = sqrt(2*max(0, a²-R²))
    // Note: mathematically this is sqrt(2)*sqrt(max(0, a²-R²))
    // but matches the ASM exactly

    // ln((n + a) / R) using fyl2x = st1 * log2(st0), with fldln2 giving ln(2)
    // So: ln(2) * log2((n+a)/R) = ln((n+a)/R)
    double ln_val = log((n + a) / R);
    double tractrix = a * ln_val - n;

    // Shape DE
    double tractrix_de = fabs(tractrix - z) - Thickness;

    // DE fix: multiply by |(R * const0 + const1)|
    double de_fix = fabs(R * const0 + const1);
    double shape_de = tractrix_de * de_fix;

    // Cylinder clip: x87 fcom/fstp pattern keeps the larger value here.
    double clip_lo = R - MaxRadius;
    shape_de = max(shape_de, clip_lo);

    // Cylinder clip: same x87 pattern again keeps the larger value.
    double clip_hi = MinRadius - R;
    shape_de = max(shape_de, clip_hi);

    // Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(state_x, state_y, state_z);
}

cpu_double3 PSphereIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_PSPHEREIFS>(p, params, de);
}

constant FormulaInfo formula_info_PSphereIFS = {
    .id = FORMULA_PSPHEREIFS,
    .name = "PSphereIFS",
    .param_count = 13,
    .complexity = 580,
    .uses_transcendental = true
};

/*
Original M3 Description:
Pretty 3D "surface" generated from the implicit expression of the tractrix. Similar to QCylinder.
It is infinitely extended but for this purpose I limited it to the most interesting part...

x = sqrt ( y*y + x*x )
m = sqrt ( a*a + x*x )
n = sqrt min (( a*a - x*x ) , 0) // avoids math errors
DE = z - (a ln (( a + n ) / x) - n) // more or less... + some additional juggling and a defix
// I multiplied by ( R+0.5)*0.3 in the end to fix DE
// basic formula is copied from; http://it.wikipedia.org/wiki/Trattrice_(geometria) thanks a lot

Luca GN 2012

Parameters:
  [0] Pseudo Radius: double (default: 3)
  [1] Scale: double (default: 2) (.2Doubles)
  [2] Z_add: double (default: 0) (.2Doubles)
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] Max Radius: double (default: 2.85)
  [6] Apply scale+add: int (default: 1)
  [7] Thickness: double (default: 0.001)
  [8] Min Radius: double (default: 0.1)
  [9] Flip XZ / YZ: int (default: 0) — 0=none, 1=swap x/z, -1=swap y/z
  [10] Upside down: int (default: 0)
  [11] const0: double (default: 0.3) — DE fix multiplier
  [12] const1: double (default: 0.02) — DE fix offset

PVar layout:
  [PVar - 0x10] = Pseudo Radius
  [PVar - 0x20] = Scale (.2Doubles)
  [PVar - 0x30] = Z_add (.2Doubles)
  [PVar - 0x38] = Y_add
  [PVar - 0x40] = X_add
  [PVar - 0x48] = Max Radius  [referenced as edi-0x48 → but listed as params[5]]
  [PVar - 0x4c] = Apply scale+add
  [PVar - 0x54] = Thickness
  [PVar - 0x5c] = Min Radius
  [PVar - 0x60] = Flip XZ / YZ (int)
  [PVar - 0x64] = Upside down (int)
  [edi + 0x00] = const0 (0.3)
  [edi + 0x08] = const1 (0.02)

Note: The ASM uses an unusual clamping pattern (val + |val|) instead of max(0, val)
to avoid conditional branches. This doubles the clamped value but the sqrt absorbs
the factor of 2 differently than a pure mathematical tractrix. The ASM is trusted.
*/
