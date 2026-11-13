// boxIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: boxIFS
// Hand-ported from ASM: 210 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS transform + SSE2 Chebyshev box DE)
// Box shape with z-Twist rotation, Trapez (z-dependent X offset), and OTrap writeback

constexpr unsigned int FORMULA_BOXIFS = 4963;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BoxIFSTwist_x87(double* xyz, const double* twist);
#endif

template<>
cpu_double3 execute_formula_dIFS<FORMULA_BOXIFS>(cpu_double3 p, double* params,
                                                   double &varyScale, double &rout) {
    const double Z_halfwidth = params[0];
    const double Y_halfwidth = params[1];
    const double X_halfwidth = params[2];
    const double Scale = params[3];
    const double Z_add = params[4];
    const double Y_add = params[5];
    const double X_add = params[6];
    const double Trapez = params[7];
    const double Twist = params[8];
    const int Apply_scale_add = int(params[9]);

    // Step 1: Non-standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        varyScale *= Scale;    // VaryScale tracking (stored to ext->VaryScale)
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
    } else {
        params[DIFS_COMMIT_XYZ_SLOT] = 0.0;
    }

    // Step 2: Optional z-Twist rotation
    double x_tw = x, y_tw = y;
#if defined(_MSC_VER) && defined(_M_X64)
    // The original tests the Single bit pattern (including negative zero),
    // then keeps the angle/product arithmetic in x87 until the XY stores.
    double twisted[] = {x,y,z};
    BoxIFSTwist_x87(twisted, &Twist);
    x_tw = twisted[0]; y_tw = twisted[1];
#elif defined(MB3D_SOFTWARE_EXT80)
    if(std::bit_cast<uint32_t>(float(Twist))!=0) {
        using namespace mb3d::numeric;
        using E=Extended80;
        ExtendedScope scope;
        const auto sc=extendedSinCos(E(float(Twist))*z);
        x_tw=double(sc.sin*E(x)+E(y)*sc.cos);
        y_tw=double(E(x)*sc.cos-E(y)*sc.sin);
    }
#else
    if (Twist != 0.0) {
        double angle = z * Twist;
        double c = cos(angle);
        double s = sin(angle);
        x_tw = s * x + c * y;
        y_tw = c * x - s * y;
    }
#endif

    // Step 3: Chebyshev box distance with Trapez
    double z_trapez = z * Trapez;
    double dx = abs(x_tw) - X_halfwidth - z_trapez;
    double dy = abs(y_tw) - Y_halfwidth;
    double dz = abs(z) - Z_halfwidth;

    // Original MAXSD chain: Z against X, then against Y. MAXSD selects
    // its second operand on unordered comparisons, so neither reassociation
    // nor std::max preserves a singular transform's downstream distance.
    double shape_de = dz > dx ? dz : dx;
    shape_de = shape_de > dy ? shape_de : dy;

    // Step 4: Scale correction
    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    rout = shape_de;    // Shape DE → ext->Rout

    // OTrap writeback for IFS color tracking
    {
        int otrap_opt = int(params[10]) & 3;
        float otrap_scale = static_cast<float>(params[12]);
        float otrap_offset = static_cast<float>(params[11]);
        double source;
        switch (otrap_opt) {
            default:
            case 0: source = params[MAX_FORMULA_PARAMS - 2]; break;  // RStopD passed by wrapper
            case 1: source = x; break;
            case 2: source = y; break;
            case 3: source = z; break;
        }
#ifdef MB3D_SOFTWARE_EXT80
        // OTrap reads the committed orbit, which remains original if Apply=0.
        if(!Apply_scale_add && otrap_opt!=0)
            source=otrap_opt==1 ? p.x : otrap_opt==2 ? p.y : p.z;
        mb3d::numeric::ExtendedScope scope;
        params[MAX_FORMULA_PARAMS-1]=double(mb3d::numeric::Extended80(source)*otrap_scale+otrap_offset);
#else
        params[MAX_FORMULA_PARAMS - 1] = source * otrap_scale + otrap_offset;
#endif
    }

    return cpu_double3(x, y, z);
}

template<>
cpu_double3 execute_formula<FORMULA_BOXIFS>(cpu_double3 p, double* params, double &de) {
    double vs = 1.0, rout = 0.0;
    auto r = execute_formula_dIFS<FORMULA_BOXIFS>(p, params, vs, rout);
    de = rout;
    return r;
}

// Alternative direct function implementation
cpu_double3 boxIFS_formula(cpu_double3 p, double* params, double &de) {
    return execute_formula<FORMULA_BOXIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_boxIFS = {
    .id = FORMULA_BOXIFS,
    .name = "boxIFS",
    .param_count = 13,
    .complexity = 300,
    .uses_transcendental = true
};

/*
Original M3 Description:
Box shape dIFS with OTrap coloring options:

0: Change with DEstop
1: Change with X
2: Change with Y
3: Change with Z

DEstop might vary on distance, choose scale = 0 to switch off.
You have to choose the 2nd coloring choice with orbit trap to see these changes.

Mathematical Formula:
  DEoption=20 (dIFS shape — SSE2 IFS + x87 twist + SSE2 box DE):
    - Non-standard IFS preamble (Scale at [edi-0x30] due to 3 extra halfwidth params)
    - Optional z-Twist: fsincos(z*Twist) rotates XY plane
    - Trapez: z*Trapez subtracted from x box distance (trapezoidal shape)
    - Chebyshev (L-infinity) box: max(|x|-Xhw-z*Trapez, |y|-Yhw, |z|-Zhw)
    - Entirely SSE2 for box computation (x87 only used for Twist and OTrap)

  Step 1: IFS transform (non-standard offsets):
    x = p.x * Scale + X_add;  y = p.y * Scale + Y_add;  z = p.z * Scale + Z_add
    if Apply_scale_add: w *= Scale

  Step 2: Optional z-Twist rotation:
    angle = z * Twist
    new_x = sin(angle)*x + cos(angle)*y
    new_y = cos(angle)*x - sin(angle)*y

  Step 3: Chebyshev box distance:
    dx = |x_tw| - X_halfwidth - z*Trapez
    dy = |y_tw| - Y_halfwidth
    dz = |z| - Z_halfwidth
    shape_de = max(dx, dy, dz)

Parameters:
  [0] Z_halfwidth: double (default: 1) — Z half-extent
  [1] Y_halfwidth: double (default: 1) — Y half-extent
  [2] X_halfwidth: double (default: 1) — X half-extent
  [3] Scale: double (default: 2) — IFS scale factor (.2Doubles)
  [4] Z_add: double (default: 0) — Z offset (.2Doubles)
  [5] Y_add: double (default: 0)
  [6] X_add: double (default: 2)
  [7] Trapez: double (default: 0) — z-dependent X offset
  [8] Twist: double (default: 0) — Single (4-byte), z-dependent XY rotation
  [9] Apply_scale_add: int (default: 1)
  [10] OTrap_option: int (default: 0) — coloring source (0..3)
  [11] OTrap_offset: single (default: 1)
  [12] OTrap_scale: single (default: 1)

PVar layout (verified against SSE2+x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = $7FFFFFFFFFFFFFFF (abs mask, 16 bytes)

  Options (negative offsets):
  [PVar - 0x10] = Z_halfwidth (double)
  [PVar - 0x18] = Y_halfwidth (double)
  [PVar - 0x20] = X_halfwidth (double, packed with Y for subpd)
  [PVar - 0x30] = Scale (.2Doubles)
  [PVar - 0x40] = Z_add (.2Doubles)
  [PVar - 0x48] = Y_add (double)
  [PVar - 0x50] = X_add (double, packed with Y_add for addpd)
  [PVar - 0x58] = Trapez (double)
  [PVar - 0x5c] = Twist (Single, 4-byte double)
  [PVar - 0x60] = apply_scale_add (integer)
  [PVar - 0x64] = OTrap_option (integer)
  [PVar - 0x68] = OTrap_offset (single)
  [PVar - 0x6c] = OTrap_scale (single)

Code structure:
  0x00-0x3C: Non-standard SSE2 IFS preamble (Scale at [edi-0x30])
  0x3D-0x41: Test Twist, skip if zero
  0x43-0x7D: x87 z-Twist rotation: fsincos(z*Twist), rotate XY
  0x80-0x8C: Copy z for Trapez, abs(x,y,z), z*Trapez
  0x91-0xA7: Subtract halfwidths, subtract Trapez, maxsd chain
  0xAB-0xB5: Scale correction + store DE
  0xB6-0xD1: OTrap computation + return

Key notes:
  - Twist rotation is x87 only, box distance is SSE2 only
  - Twist matrix [sin cos; cos -sin] has det=-1 (reflection), not pure rotation
  - Twist uses z as rotation angle parameter (creates helical twist along z-axis)
  - Trapez uses signed z (before abs), creating trapezoidal cross-sections
  - When Twist=0 and Trapez=0: standard axis-aligned box
  - OTrap pattern identical to RoundedBox and SphereIFS
  - Stack allocation 0x18 (24 bytes) only when Twist != 0
  - 210 bytes — moderate complexity with optional twist path
*/
