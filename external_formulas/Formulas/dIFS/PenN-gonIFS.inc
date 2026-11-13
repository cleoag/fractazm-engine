// PenN-gonIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PenN-gonIFS
// Hand-ported from ASM: 236 bytes, Capstone-verified
// DEoption: 20 (dIFS — formula computes shape DE directly)
// N-gon torus with Möbius twist (produces Penrose triangle shapes)

constexpr unsigned int FORMULA_PENN_GONIFS = 1446;

template<>
cpu_double3 execute_formula<FORMULA_PENN_GONIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Radius_1 = params[0];        // major radius (polygon wall distance)
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Radius_2 = params[5];        // cross-section half-size
    const int apply_scale_add = int(params[6]);
    const int Poly_Order = int(params[7]);    // N-gon order (3 = triangle, etc.)
    const int Mobius = int(params[8]);         // Möbius twist factor
    const double Atan_add = params[9];         // twist offset

    // Mathematical constants (from M3 CONSTANTS section)
    const double INV_2PI = params[10];  // [PVar + 0x00]
    const double TWO_PI = params[11];   // [PVar + 0x08]
    const double HALF_PI = params[12];  // [PVar + 0x10]
    const double QUARTER = params[13];  // [PVar + 0x18]

    // Step 1: IFS transformation — scale + translate
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    // Step 2: Conditional position/w update
    cpu_double3 pos;
    if (apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;  // w tracking
        pos = cpu_double3(x, y, z);
    } else {
        pos = p;  // position unchanged
    }

    // Step 3: Compute azimuthal angle
    // ASM: fpatan with ST(1)=x, ST(0)=y gives atan2(x, y)
    double raw_angle = atan2(x, y);

    // Step 4: Möbius twist angle
    // mobius_angle = raw_angle * 0.25 * Mobius + Atan_add
    double mobius_angle = raw_angle * QUARTER * double(Mobius) + Atan_add;
    double cos_mob = cos(mobius_angle);
    double sin_mob = sin(mobius_angle);

    // Step 5: Polyfold — quantize angle into Poly_Order sectors
    // ASM: fsubr(pi/2) converts atan2(x,y) to atan2(y,x)
    double polyfold_angle = HALF_PI - raw_angle;

    // Quantize: round(angle * INV_2PI * N) / N * TWO_PI
    double sector = rint(polyfold_angle * INV_2PI * double(Poly_Order));
    sector = sector / double(Poly_Order) * TWO_PI;

    double cs = cos(sector);
    double ss = sin(sector);

    // Step 6: Radial projection — distance from polygon wall
    // Project (x, y) onto sector direction, take absolute value
    double d_radial = abs(x * cs + y * ss) - Radius_1;

    // Step 7: Möbius-rotated cross-section distances
    // The cross-section is a square (L∞ norm) that rotates around the N-gon
    double dist1 = abs(d_radial * sin_mob - z * cos_mob);
    double dist2 = abs(d_radial * cos_mob + z * sin_mob);

    // Step 8: Shape DE — max distance minus cross-section radius
    double shape_de = max(dist1, dist2) - Radius_2;

    // Step 9: DE compensation when not applying scale+add
    if (apply_scale_add == 0) {
        shape_de = shape_de / Scale;
    }

    // dIFS shape_de stored to Rout ([esi-0x20] in ASM).
    de = shape_de;

    return pos;
}

// Shape DE computation (standalone, for use by dIFS engine)
double PenNgonIFS_shape_de(cpu_double3 p, constant double* params) {
    const double Radius_1 = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Radius_2 = params[5];
    const int apply_scale_add = int(params[6]);
    const int Poly_Order = int(params[7]);
    const int Mobius = int(params[8]);
    const double Atan_add = params[9];

    const double INV_2PI = params[10];
    const double TWO_PI = params[11];
    const double HALF_PI = params[12];
    const double QUARTER = params[13];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    // Raw angle and Möbius twist
    double raw_angle = atan2(x, y);
    double mobius_angle = raw_angle * QUARTER * double(Mobius) + Atan_add;
    double cos_mob = cos(mobius_angle);
    double sin_mob = sin(mobius_angle);

    // Polyfold
    double polyfold_angle = HALF_PI - raw_angle;
    double sector = rint(polyfold_angle * INV_2PI * double(Poly_Order));
    sector = sector / double(Poly_Order) * TWO_PI;
    double cs = cos(sector);
    double ss = sin(sector);

    // Radial projection and cross-section
    double d_radial = abs(x * cs + y * ss) - Radius_1;
    double dist1 = abs(d_radial * sin_mob - z * cos_mob);
    double dist2 = abs(d_radial * cos_mob + z * sin_mob);

    double shape_de = max(dist1, dist2) - Radius_2;
    if (apply_scale_add == 0) { shape_de /= Scale; }
    return shape_de;
}

// Alternative direct function implementation
cpu_double3 PenN_gonIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_PENN_GONIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_PenN_gonIFS = {
    .id = FORMULA_PENN_GONIFS,
    .name = "PenN-gonIFS",
    .param_count = 10,
    .complexity = 560,
    .uses_transcendental = true
};

/*
Original M3 Description:
Variation of the torus.
This can generate shapes that resemble the famous "Penrose triangle".
Set mobius=0 to disable the mobius effect.

Luca GN 2012

Mathematical Formula:
  dIFS execution model (DEoption=20):
    - Two angle computations from atan2(x, y):
      1. Möbius twist (uses raw angle directly)
      2. Polyfold (converts to atan2(y, x) via pi/2 - angle)

  Step 1: IFS transformation (if apply_scale_add != 0):
    (x, y, z) = (x, y, z) * Scale + (X_add, Y_add, Z_add)
    w *= Scale

  Step 2: Raw azimuthal angle:
    raw_angle = atan2(x, y)   (from fpatan with ST(1)=x, ST(0)=y)

  Step 3: Möbius twist angle:
    mobius_angle = raw_angle * 0.25 * Mobius + Atan_add
    Compute sin_mob, cos_mob

  Step 4: Polyfold (quantize into Poly_Order sectors):
    polyfold_angle = pi/2 - raw_angle   (= atan2(y, x))
    sector = round(polyfold_angle * INV_2PI * Poly_Order) / Poly_Order * TWO_PI
    Compute cs = cos(sector), ss = sin(sector)

  Step 5: Radial projection:
    d_radial = |x*cs + y*ss| - Radius_1

  Step 6: Möbius-rotated cross-section (square, L-infinity norm):
    dist1 = |d_radial * sin_mob - z * cos_mob|
    dist2 = |d_radial * cos_mob + z * sin_mob|

  Step 7: Shape DE:
    shape_de = max(dist1, dist2) - Radius_2
    If apply_scale_add == 0: shape_de /= Scale

Parameters:
  [0] Radius_1: double (default: 1.18) — major radius to polygon wall
  [1] Scale: double (default: 2) — from .2Doubles
  [2] Z_add: double (default: 0) — from .2Doubles
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 2)
  [5] Radius_2: double (default: 0.3) — cross-section half-size
  [6] apply_scale_add: int (default: 1)
  [7] Poly_Order: int (default: 3) — N-gon order (3=triangle, 5=pentagon, etc.)
  [8] Mobius: int (default: 3) — twist factor (0=disabled)
  [9] Atan_add: double (default: 1.18) — twist offset angle

PVar layout (verified against SSE2+x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 1/(2*pi) = 0.159154943
  [PVar + 0x08] = 2*pi = 6.283185307
  [PVar + 0x10] = pi/2 = 1.57079632675
  [PVar + 0x18] = 0.25 (quarter-turn factor for Möbius)
  [PVar + 0x20] = 5.0 (unused in ASM)

  Options (negative offsets):
  [PVar - 0x10] = Radius_1 (double)
  [PVar - 0x18] = Scale high (from .2Doubles)
  [PVar - 0x20] = Scale low (from .2Doubles — both = Scale)
  [PVar - 0x28] = Z_add high (from .2Doubles)
  [PVar - 0x30] = Z_add low (from .2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = Radius_2 (double)
  [PVar - 0x4C] = apply_scale_add (integer)
  [PVar - 0x50] = Poly_Order (integer)
  [PVar - 0x54] = Mobius (integer)
  [PVar - 0x5C] = Atan_add (double)

dIFS state structure (via esi):
  [esi - 0x78] = x position (double, packed low with y)
  [esi - 0x70] = y position (double, packed high)
  [esi - 0x68] = z position (double)
  [esi + 0x70] = w (DE scale tracking)
  [esi - 0x20] = shape DE output

SSE2 + x87 hybrid execution:
  - SSE2 for IFS position transform (mulpd/addpd on packed (x,y) pair)
  - x87 FPU for fpatan, fsincos, polyfold (fimul, frndint, fidiv)
  - fpatan gives atan2(x,y); fsubr(pi/2) converts to atan2(y,x) for polyfold
  - Stack-allocated 60 bytes for intermediate values

Key notes:
  - N-gon shape from polyfold: azimuthal angle quantized into Poly_Order sectors
  - Möbius twist: cross-section rotates as you traverse the torus
  - Cross-section is square (L∞ norm): max of two perpendicular |distances|
  - Mobius=0 gives a plain N-gon torus
  - Mobius=3 with Poly_Order=3 gives Penrose triangle illusion
  - The 0.25 factor makes Mobius scale in quarter-turns per radian of azimuth
  - Radius_1 = distance from center to polygon face
  - Radius_2 = half-thickness of the tube (square cross-section)
*/
