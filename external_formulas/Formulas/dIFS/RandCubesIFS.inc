// RandCubesIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: RandCubesIFS
// Hand-ported from ASM: 1122 bytes, DEoption=20
// Pseudorandom pattern of geometric shapes on XY plane
// 7 shape types: prisms, cones, spheres, pyramids, arcs, tubes, rounded prisms
// Formula by Luca GN 2012

constexpr unsigned int FORMULA_RANDCUBESIFS = 10311;

// ASM trace summary:
// DEoption=20: SSE2 IFS preamble + x87 pseudorandom shape DE
//
// Parameters:
//   [edi-0x10] = XVtn (0-1) (double, default 0.8)
//   [edi-0x20] = Scale (.2Doubles, default 2)
//   [edi-0x30] = Z add (.2Doubles, default 0)
//   [edi-0x38] = Y add (double, default 0)
//   [edi-0x40] = X add (double, default 0)
//   [edi-0x48] = YVtn (0-1) (double, default 0.8)
//   [edi-0x4c] = Apply scale+add (integer, default 1)
//   [edi-0x54] = ZMaxSz (double, default 2)
//   [edi-0x58] = RandomizerX (integer, default 127)
//   [edi-0x5c] = RandomizerY (integer, default 172)
//   [edi-0x60] = ShapeType 1 (integer, default 0)
//   [edi-0x64] = ShapeType 2 (integer, default 0)
//   [edi-0x6c] = ZFlight (double, default 0)
//
// Constants:
//   [edi+0x00] = -6.86656e-6
//   [edi+0x08] = -0.45
//   [edi+0x10] = 1.5e-5
//   [edi+0x18] = -1.37331e-5
//   [edi+0x20] = -10.3e-6
//   [edi+0x28] = -3.1e-6
//
// The formula:
// 1. SSE2 preamble: scale and translate
// 2. Quantize x,y to grid cells: ix = int(x), iy = int(y); fx = |frac(x)|, fy = |frac(y)|
// 3. Pseudorandom hash from (ix, iy) using XOR, multiply, shift operations
//    to generate deterministic per-cell random values
// 4. Use random values to compute shape-specific DE for the selected ShapeType
// 5. Select between ShapeType1 and ShapeType2 based on a random bit
//
// Shape types:
//   0 = Rectangular prisms (box DE)
//   1 = Cones/cylinders
//   2 = Spheres/cylinders
//   3 = Pyramids
//   4 = Solid arcs
//   5 = Tubey arcs
//   6 = Rounded square prisms
//
// This is extremely complex procedural geometry. The pseudorandom hash uses
// integer arithmetic that is architecture-specific (32-bit overflow behavior).

template<>
cpu_double3 execute_formula<FORMULA_RANDCUBESIFS>(cpu_double3 p, constant double* params, double &de) {
    const double XVtn       = params[0];
    const double Scale      = params[1];
    const double Zadd       = params[2];
    const double Yadd       = params[3];
    const double Xadd       = params[4];
    const double YVtn       = params[5];
    const int   ApplyScale = int(params[6]);
    const double ZMaxSz     = params[7];
    const int   RandX      = int(params[8]);
    const int   RandY      = int(params[9]);
    const int   ShapeType1 = int(params[10]);
    const int   ShapeType2 = int(params[11]);
    const double ZFlight    = params[12];

    // Constants
    const double c0 = -6.86656e-6;
    const double c1 = -0.45;
    const double c2 = 1.5e-5;
    const double c3 = -1.37331e-5;
    const double c4 = -10.3e-6;
    const double c5 = -3.1e-6;

    // Step 1: SSE2 IFS preamble
    double x = p.x * Scale + Xadd;
    double y = p.y * Scale + Yadd;
    double z = p.z * Scale + Zadd;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (ApplyScale != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Quantize to grid cells.
    // The original ASM uses x87 FIST/FISUB, so the integer cell index follows
    // the current x87 rounding mode (nearest-even in the default Pascal build),
    // not C-style truncation toward zero.
    int ix = x87_fistp_int32(x);
    double fx = fabs(x - double(ix));  // |fractional part|

    int iy = x87_fistp_int32(y);
    double fy = fabs(y - double(iy));

    // Step 3: Pseudorandom hash
    // This uses 32-bit integer arithmetic with specific XOR/multiply patterns
    // ASM traces a complex hash function:
    // iy ^= 0xB641; ix ^= 0x9D81
    // product = (uint)iy * (uint)ix; inv = ~(product >> 32... actually mul edx:eax)
    // Various XOR, AND, shift operations to derive multiple random values

    uint32_t uix = uint32_t(ix) ^ 0x9D81u;
    uint32_t uiy = uint32_t(iy) ^ 0xB641u;
    uint64_t product = uint64_t(uiy) * uint64_t(uix);
    uint32_t product_lo = uint32_t(product & 0xFFFFFFFFu);
    uint32_t product_hi = uint32_t((product >> 32) & 0xFFFFFFFFu);
    uint32_t rand_a = ~product_lo;

    uint32_t mix1 = (rand_a & 0x5A5Au) ^ (product_hi | 0x5A5Au);
    uint32_t mix2 = (uiy | 0x5A5Au) ^ (uix & 0x5A5Au);

    // Pascal ASM: (mix1 - RandY) in eax, mix1 in edx, mul edx → (mix1-RandY)*mix1
    uint32_t sq1 = uint32_t((mix1 - uint32_t(RandY)) * mix1);
    uint32_t h1 = (uint32_t(((sq1 + rand_a) >> 8) ^ ((sq1 + rand_a) << 8)) & 0xFFFFu);

    uint32_t sq2 = uint32_t((mix2 - uint32_t(RandX)) * mix2);
    uint32_t h2 = (uint32_t(((sq2 + rand_a) >> 8) ^ ((sq2 + rand_a) << 8)) & 0xFFFFu);

    // Mix for shape selection and Z-height random
    uint32_t h3 = ((h2 >> 6) ^ (h1 << 6)) & 0xFFFFu;
    uint32_t h4 = (h2 & 0xAAAAu) ^ (h1 & 0x5555u);

    double z_shifted = z - double(h4) * c0 * ZFlight;
    double z_shape = fabs(z_shifted) + double(h3) * c0 * ZMaxSz;
    params[63] = double(h3) * c2; // FILD cell hash; FMUL [edi+0x10].

    // Step 4: Select shape type based on random bit
    // ASM: and eax, 0x40; jne => selects ShapeType1 or ShapeType2
    int shapeType = ((h2 & 0x40u) != 0u) ? ShapeType2 : ShapeType1;

    // Literal branches of the shipped x87 routine (0x188..0x450).
    // Its FCOM/AND 0x41 selection keeps the second operand on unordered input.
    double shape_de;
    const double rx = double(h2) * c0 * XVtn;
    const double ry = double(h1) * c0 * YVtn;
    if (shapeType == 1) {
        double candidate = sqrt(fx * fx + fy * fy) + rx;
        candidate += (1.0 - XVtn) * c1;
        candidate -= fabs(z_shifted) * ry;
        shape_de = candidate > z_shape ? candidate : z_shape;
    } else if (shapeType == 2) {
        double candidate = sqrt(fx * fx + fy * fy -
                                z_shifted * z_shifted * (double(h2) * c3 * XVtn));
        candidate += ry;
        candidate += (1.0 - YVtn) * c1;
        shape_de = candidate > z_shape ? candidate : z_shape;
    } else if (shapeType == 3) {
        double a = fx - rx * fabs(z_shifted);
        double b = fy - ry * fabs(z_shifted);
        double candidate = (b > a ? b : a) + c1;
        shape_de = candidate > z_shape ? candidate : z_shape;
    } else if (shapeType == 4) {
        double ax = h2 >= h1 ? fy : fx;
        double ay = h2 >= h1 ? fx : fy;
        double a = ax + rx + (1.0 - XVtn) * c1;
        double t = z_shifted - double(h3) * c0 * ZMaxSz;
        t = (t - fabs(t)) * c1;
        double b = sqrt(t * t + ay * ay) + ry + (1.0 - YVtn) * c1;
        double candidate = b > a ? b : a;
        // FSTP ST(1) discards the initial Z bound, then FLD loads signed Z.
        shape_de = z_shifted > candidate ? z_shifted : candidate;
    } else if (shapeType == 5) {
        double ax = h2 >= h1 ? fy : fx;
        double ay = h2 >= h1 ? fx : fy;
        uint32_t hx = h2 >= h1 ? h1 : h2;
        uint32_t hy = h2 >= h1 ? h2 : h1;
        // These two stores are explicitly SINGLE precision in the original.
        double radius = double(float(fabs(double(hx) * c5 * XVtn)));
        double offset = double(float(double(hy) * c4 * YVtn));
        double t = z_shifted - double(h3) * c0 * ZMaxSz;
        t = (t - fabs(t)) * c1;
        double t2 = t * t;
        double candidate = sqrt(t2 + ay * ay + ax * ax + radius * (offset * offset));
        candidate += sqrt(sqrt(t2 + ax * ax)) * offset;
        shape_de = candidate > z_shifted ? candidate : z_shifted;
    } else if (shapeType == 6) {
        double radius = double(float(double(h1) * c4 * YVtn));
        double offset = rx + (1.0 - XVtn) * c1 - radius;
        double narrowed_offset = double(float(offset));
        // FST SINGLE does not pop: X uses the original value, Y/Z the store.
        double dx = fx + offset;
        double dy = fy + narrowed_offset;
        double dz = z_shape + narrowed_offset;
        dx = (dx + fabs(dx)) * c1;
        dy = (dy + fabs(dy)) * c1;
        dz = (dz + fabs(dz)) * c1;
        shape_de = sqrt(dx * dx + dy * dy + dz * dz) + radius;
    } else {
        // Type 0 and every unrecognized selector share the same branch.
        double dx = fx + rx + (1.0 - XVtn) * c1;
        double dy = fy + ry + (1.0 - YVtn) * c1;
        double candidate = dy > dx ? dy : dx;
        shape_de = candidate > z_shape ? candidate : z_shape;
    }

    // Step 7: Scale correction
    if (ApplyScale == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(state_x, state_y, state_z);
}

// Alternative direct function
cpu_double3 RandCubesIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_RANDCUBESIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_RandCubesIFS = {
    .id = FORMULA_RANDCUBESIFS,
    .name = "RandCubesIFS",
    .param_count = 13,
    .complexity = 1122,
    .uses_transcendental = false
};

/*
Original M3 Description:
Simple DIFS formula that (using pseudorandom numbers) generates an uniform
pattern of cubes on XY plane. Vtn values near to zero may be a little noisy
on edges due to some simplifications I made.

Two different kinds of shapes can be generated at the same time!
You can choose different shape variations if you change Shape types:
  0 = Rectangular prisms
  1 = Cones (or cylinders)
  2 = Spheres (or cylinders)
  3 = Pyramids (in this case you can use Vtn > 1 for less sharp solids)
  4 = Solid arcs
  5 = Tubey arcs
  6 = Rounded square prisms

Luca GN 2012

Mathematical Formula (DEoption=20, dIFS shape):
  Step 1: SSE2 IFS preamble
  Step 2: Grid quantization — extract cell (ix,iy) and fractional (fx,fy)
  Step 3: Pseudorandom hash from cell coordinates
    Uses XOR with 0xB641/0x9D81, multiply, NOT, bit shuffling
    Produces deterministic per-cell random values for shape parameters
  Step 4: Select shape type (1 of 7) based on random bit and ShapeType params
  Step 5: Compute shape-specific DE using random size/position parameters
  Step 6: Scale correction if needed

Parameters:
  [0] XVtn (0-1): double (default: 0.8) — X variation
  [1] Scale: double (default: 2) — .2Doubles
  [2] Z add: double (default: 0)
  [3] Y add: double (default: 0)
  [4] X add: double (default: 0)
  [5] YVtn (0-1): double (default: 0.8) — Y variation
  [6] Apply scale+add: integer (default: 1)
  [7] ZMaxSz: double (default: 2) — max Z size
  [8] RandomizerX: integer (default: 127)
  [9] RandomizerY: integer (default: 172)
  [10] ShapeType 1: integer (default: 0)
  [11] ShapeType 2: integer (default: 0)
  [12] ZFlight: double (default: 0)

NOTE: The pseudorandom number generation relies on 32-bit integer overflow
behavior. The Metal implementation may produce slightly different random
patterns than the original x86 implementation due to differences in
integer arithmetic handling. The visual result should be similar but
not pixel-identical.
*/
