// PolyPyraIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: PolyPyraIFS
// Hand-ported from ASM: 283 bytes, Capstone-verified
// DEoption: 20 (dIFS — formula computes shape DE directly)
// Pyramid/polygon shapes with polyfold, star deformation, and OTrap writeback

constexpr unsigned int FORMULA_POLYPYRAIFS = 9090;

template<>
cpu_double3 execute_formula<FORMULA_POLYPYRAIFS>(cpu_double3 p, constant double* params, double &de) {
    const double Height_fix = params[0];       // cap height
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double XZ_angle = params[5];         // pyramid slope angle (degrees)
    const int apply_scale_add = int(params[6]);
    const int PolyFold_Order = int(params[7]); // polygon sides
    const double Cuts_fix = params[8];          // truncation offset
    const double Initial_z_add = params[9];     // z origin shift
    const double Star = params[10];             // star deformation factor

    // Mathematical constants (from M3 CONSTANTS section)
    const double INV_2PI = 0.159154943;    // 1 / (2*pi)
    const double NEG_TWO_PI = -6.283185307; // -(2*pi)
    const double HALF_PI = 1.57079632675;   // pi/2
    const double DEG2RAD = 0.017453293;     // pi/180

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

    // Step 3: Polyfold — quantize azimuthal angle into sectors
    // ASM: fpatan(x, y) → fsubr(pi/2) = atan2(y, x)
    double raw_angle = atan2(x, y);
    double angle = HALF_PI - raw_angle;

    int n = int(rint(angle * INV_2PI * double(PolyFold_Order)));
    double sector = double(n) / double(PolyFold_Order) * NEG_TWO_PI;

    double cs = cos(sector);
    double ss = sin(sector);

    // Step 4: 2D rotation — project along and perpendicular to sector
    // ASM: radial = x*cs - y*ss; perp = x*ss + y*cs
    double radial = x * cs - y * ss;       // along sector direction
    double perp = x * ss + y * cs;         // perpendicular to sector

    // Step 5: Star deformation
    // Add |perpendicular * Star| to radial distance (creates star tips)
    double projected = radial + abs(perp * Star);

    // Step 6: Z processing — flip and shift
    double z_flipped = -(z + Initial_z_add);

    // Step 7: XZ angle projection — pyramid slope
    // Project (projected, z_flipped) along the pyramid face normal
    double xz_rad = DEG2RAD * XZ_angle;
    double plane_dist = projected * sin(xz_rad) + z_flipped * cos(xz_rad);

    // Step 8: Cap distance — truncation plane
    double cap_dist = abs(z - Cuts_fix) - Height_fix;

    // Step 9: Shape DE = max(cap_dist, plane_dist)
    // FCOM/JAE retains the second operand for unordered comparisons.
    double shape_de = cap_dist >= plane_dist ? cap_dist : plane_dist;

    // Step 10: DE compensation when not applying scale+add
    if (apply_scale_add == 0) {
        shape_de = shape_de / Scale;
    }

    // OTrap writeback
    const double iter_count = params[ITER_COUNT_SLOT]; // FILD [esi-0x18]
    params[MAX_FORMULA_PARAMS - 1] = fabs(iter_count * params[11] + params[12] + double(n) * params[13]);

    // dIFS shape_de stored to Rout ([esi-0x20] in ASM).
    de = shape_de;

    return pos;
}

// Shape DE computation (standalone, for use by dIFS engine)
double PolyPyraIFS_shape_de(cpu_double3 p, constant double* params) {
    const double Height_fix = params[0];
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double XZ_angle = params[5];
    const int apply_scale_add = int(params[6]);
    const int PolyFold_Order = int(params[7]);
    const double Cuts_fix = params[8];
    const double Initial_z_add = params[9];
    const double Star = params[10];

    const double INV_2PI = 0.159154943;
    const double NEG_TWO_PI = -6.283185307;
    const double HALF_PI = 1.57079632675;
    const double DEG2RAD = 0.017453293;

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    // Polyfold
    double angle = HALF_PI - atan2(x, y);
    int n = int(rint(angle * INV_2PI * double(PolyFold_Order)));
    double sector = double(n) / double(PolyFold_Order) * NEG_TWO_PI;
    double cs = cos(sector);
    double ss = sin(sector);

    // Rotation + star
    double radial = x * cs - y * ss;
    double perp = x * ss + y * cs;
    double projected = radial + abs(perp * Star);

    // Z flip + XZ projection
    double z_flipped = -(z + Initial_z_add);
    double xz_rad = DEG2RAD * XZ_angle;
    double plane_dist = projected * sin(xz_rad) + z_flipped * cos(xz_rad);

    // Cap + shape DE
    double cap_dist = abs(z - Cuts_fix) - Height_fix;
    // FCOM/JAE retains the second operand for unordered comparisons.
    double shape_de = cap_dist >= plane_dist ? cap_dist : plane_dist;
    if (apply_scale_add == 0) { shape_de /= Scale; }
    return shape_de;
}

// Alternative direct function implementation
cpu_double3 PolyPyraIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_POLYPYRAIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_PolyPyraIFS = {
    .id = FORMULA_POLYPYRAIFS,
    .name = "PolyPyraIFS",
    .param_count = 14,
    .complexity = 580,
    .uses_transcendental = true
};

/*
Original M3 Description:
Powerful generator of pyramid/polygonal shapes. Using the Navi real-time
preview mode is a must with this.

* Truncated pyramids: Set a positive "Cuts fix" value
* Stars: Set a positive "Star" value
* Cones: Use a big value as PolyFold Order
* Extruded polyhedrons: Use xz angle like 90.1, big negative Initial z add
* Cylinders: Combine cone + extrusion techniques

Luca GN 2012

Mathematical Formula:
  dIFS execution model (DEoption=20):
    - Polyfold for N-sided polygon base
    - Star deformation on perpendicular component
    - XZ angle projection for pyramid slope
    - Cap plane for height truncation

  Step 1: IFS transformation (if apply_scale_add != 0):
    (x, y, z) = (x, y, z) * Scale + (X_add, Y_add, Z_add)
    w *= Scale

  Step 2: Polyfold (azimuthal, XY plane):
    angle = atan2(y, x)  (via fpatan + fsubr(pi/2))
    n = round(angle * INV_2PI * PolyFold_Order)
    sector = n / PolyFold_Order * (-TWO_PI)
    cs = cos(sector), ss = sin(sector)

  Step 3: 2D rotation + star:
    radial = x*cs - y*ss     (along sector direction)
    perp = x*ss + y*cs       (perpendicular to sector)
    projected = radial + |perp * Star|

  Step 4: Z processing:
    z_flipped = -(z + Initial_z_add)

  Step 5: XZ angle projection (pyramid slope):
    xz_rad = XZ_angle * DEG2RAD
    plane_dist = projected * sin(xz_rad) + z_flipped * cos(xz_rad)

  Step 6: Cap distance (truncation):
    cap_dist = |z - Cuts_fix| - Height_fix

  Step 7: Shape DE:
    shape_de = max(cap_dist, plane_dist)
    If apply_scale_add == 0: shape_de /= Scale

  Step 8: OTrap coloring (cosmetic):
    OTrap = |iter_count * OTrapOnIts + OTrapOffset + n * OTrapSides|

Parameters:
  [0] Height_fix: double (default: 1) — cap height
  [1] Scale: double (default: 2) — from .2Doubles
  [2] Z_add: double (default: 0) — from .2Doubles
  [3] Y_add: double (default: 0)
  [4] X_add: double (default: 0)
  [5] XZ_angle: double (default: 60) — pyramid slope (degrees)
  [6] apply_scale_add: int (default: 1)
  [7] PolyFold_Order: int (default: 5) — polygon sides
  [8] Cuts_fix: double (default: 0) — truncation offset
  [9] Initial_z_add: double (default: 1) — z origin shift
  [10] Star: double (default: 0) — star deformation (0=off)
  [11] OTrapOnIts: double (default: 0.1) — OTrap iteration scaling
  [12] OTrapOffset: double (default: 0.1) — OTrap base offset
  [13] OTrapSides: double (default: -0.1) — OTrap sector scaling

PVar layout (verified against SSE2+x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 0.159154943 (1/(2*pi))
  [PVar + 0x08] = -6.283185307 (NEGATIVE 2*pi)
  [PVar + 0x10] = 1.57079632675 (pi/2)
  [PVar + 0x18] = 0.017453293 (pi/180, deg-to-rad)

  Options (negative offsets):
  [PVar - 0x10] = Height_fix (double)
  [PVar - 0x18] = Scale high (from .2Doubles)
  [PVar - 0x20] = Scale low (from .2Doubles — both = Scale)
  [PVar - 0x28] = Z_add high (from .2Doubles)
  [PVar - 0x30] = Z_add low (from .2Doubles)
  [PVar - 0x38] = Y_add (double)
  [PVar - 0x40] = X_add (double)
  [PVar - 0x48] = XZ_angle (double, in degrees)
  [PVar - 0x4C] = apply_scale_add (integer)
  [PVar - 0x50] = PolyFold_Order (integer)
  [PVar - 0x58] = Cuts_fix (double)
  [PVar - 0x60] = Initial_z_add (double)
  [PVar - 0x68] = Star (double)
  [PVar - 0x70] = OTrapOnIts (double)
  [PVar - 0x78] = OTrapOffset (double)
  [PVar - 0x80] = OTrapSides (double)

dIFS state structure (via esi):
  [esi - 0x78] = x position (double, packed low with y)
  [esi - 0x70] = y position (double, packed high)
  [esi - 0x68] = z position (double)
  [esi + 0x70] = w (DE scale tracking)
  [esi - 0x20] = shape DE output
  [esi - 0x18] = iteration count (integer, for OTrap)
  [esi + 0x80] = OTrap coloring output

SSE2 + x87 hybrid execution:
  - SSE2 for IFS position transform
  - x87 FPU for fpatan, polyfold, fsincos, projection
  - Stack-allocated 66 bytes for intermediate values

Key notes:
  - Polyfold creates N-sided polygon base (PolyFold_Order sides)
  - XZ_angle=60 gives standard pyramid, 90=prism (extruded polygon)
  - Star>0 adds star-shaped tips (perpendicular distance contribution)
  - Cuts_fix>0 truncates pyramid top (truncated pyramid)
  - Initial_z_add shifts the pyramid apex in z
  - Height_fix is the half-height of the cap planes
  - Cap distance uses the TRANSFORMED z (after IFS), not original
  - NEG_TWO_PI constant: negative sign built into constant, not fchs
  - OTrap combines iteration index and sector index for coloring
*/
