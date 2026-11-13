// NewMengerIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: NewMengerIFS
// Hand-ported from ASM: 572 bytes, Capstone-verified
// DEoption: 20 (dIFS shape -- SSE2 IFS transform + x87 Menger sponge variant)
// Fractal formula by Twinbee and Knighty. Unusual Menger sponge variant
// implemented as a DIFS shape with fprem-based folding, rotation, and iterative DE.

constexpr unsigned int FORMULA_NEWMENGERIFS = 10119;

template<>
cpu_double3 execute_formula<FORMULA_NEWMENGERIFS>(cpu_double3 p, constant double* params, double &de) {
    const double MengerScale = params[0];    // [PVar - 0x10] double
    const double Scale       = params[1];    // [PVar - 0x20] .2Doubles
    const double Z_add       = params[2];    // [PVar - 0x30] .2Doubles
    const double Y_add       = params[3];    // [PVar - 0x38] double
    const double X_add       = params[4];    // [PVar - 0x40] double
    const double MengerDiv   = params[5];    // [PVar - 0x48] double
    const int   Apply_sa    = int(params[6]); // [PVar - 0x4c] integer
    const double XOff        = params[7];    // [PVar - 0x54] double
    const double YOff        = params[8];    // [PVar - 0x5c] double
    const double ZOff        = params[9];    // [PVar - 0x64] double
    int         MaxIter     = int(params[10]); // [PVar - 0x68] integer
    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    const double MengerRC    = params[20]; // [PVar - 0x94] (MengerReCenter)
    // Constants: [edi] = 0.5, [edi+8] = 3.0, [edi+0x10] = 1.5
    const double C0 = params[21];
    const double C1 = params[22];
    const double C2 = params[23];

    // Step 1: SSE2 IFS preamble
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_sa != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // MaxIter clamped to [1..31]
    MaxIter = MaxIter & 0x1F;
    if (MaxIter == 0) MaxIter = 1;

    // Step 2: Iterative Menger loop
    // ASM loop: 0x5b..0x226 (dec edx, jg 0xd0 at end)
    // Pre-loop: scale coords: coord = coord * C0 + C0 (maps to [0,1] range)
    x = x * C0 + C0;
    y = y * C0 + C0;
    z = z * C0 + C0;

    // Menger cross-section DE = |coord - C0| - C0
    double dx = fabs(x - C0) - C0;   // [esp+0x18]
    double dy = fabs(y - C0) - C0;   // [esp+0x20]
    double dz = fabs(z - C0) - C0;   // [esp+0x28]

    // max_de = max(dx, dy, dz)
    double max_de = dx;
    if (dy > max_de) max_de = dy;
    if (dz > max_de) max_de = dz;
    // [esp+0x38] = max_de, [esp+0x40] = max_de (initial DE accumulator)
    double accum_de = max_de;

    // scale_factor starts at 1.0
    double scale_factor = 1.0; // [esp+0x30]

    for (int iter = 0; iter < MaxIter; iter++) {
        // Original x87 code folds inward, adds the per-axis offset, then
        // rotates using the packed single-precision matrix rows.
        double zf = -fabs(x87_fprem(z * MengerDiv * scale_factor, MengerDiv) - MengerRC) + C0 + ZOff;
        double yf = -fabs(x87_fprem(y * MengerDiv * scale_factor, MengerDiv) - MengerRC) + C0 + YOff;
        double xf = -fabs(x87_fprem(x * MengerDiv * scale_factor, MengerDiv) - MengerRC) + C0 + XOff;
        double rx = xf * params[17] + yf * params[18] + zf * params[19];
        double ry = xf * params[11] + yf * params[12] + zf * params[13];
        double rz = xf * params[14] + yf * params[15] + zf * params[16];

        // The cross distance is the median of the three rotated components.
        double mxy = max(rx, ry);
        double mxz = max(rx, rz);
        double myz = max(ry, rz);

        // Then: take min of (mxy, mxz) -> intermediate
        // Then: take min of (intermediate, myz) -> second_largest
        // ASM: fcom comparisons select minimum of the three maxes
        double cross_de = min(min(mxy, mxz), myz);

        // Divide by scale_factor and store
        double iter_de = cross_de / scale_factor;

        // Update accum_de = max(accum_de, iter_de)
        // ASM at 0x206-0x21A: fcom, if accum_de > iter_de keep accum_de
        // FCOM keeps the new distance on unordered (NaN) comparisons.
        if (!(accum_de > iter_de)) accum_de = iter_de;

        // Update scale_factor *= MengerScale (via [edi-0x10])
        scale_factor *= MengerScale;
    }

    // Step 3: Scale correction
    if (Apply_sa == 0) {
        accum_de /= Scale;
    }

    de = accum_de;

    return cpu_double3(state_x, state_y, state_z);
}

cpu_double3 NewMengerIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_NEWMENGERIFS>(p, params, de);
}

constant FormulaInfo formula_info_NewMengerIFS = {
    .id = FORMULA_NEWMENGERIFS,
    .name = "NewMengerIFS",
    .param_count = 15,
    .complexity = 572,
    .uses_transcendental = false
};

/*
Original M3 Description:
Fractal formula by Twinbee and Knighty.
Unusual Menger sponge variant with fprem-based folding, rotation, and iterative DE.
Very unique shapes are generated!

Mathematical Formula (DEoption=20, dIFS shape):
  Step 1: SSE2 IFS preamble
    x = p.x * Scale + X_add; y = p.y * Scale + Y_add; z = p.z * Scale + Z_add
    if Apply_sa != 0: de *= Scale

  Step 2: Pre-scale coords: c = c * 0.5 + 0.5
    dx = |x - 0.5| - 0.5 (per axis)
    max_de = max(dx, dy, dz)

  Step 3: Iterative Menger loop (MaxIter times):
    For each axis: fold = -abs(fmod(coord * MengerDiv * scale_factor, MengerDiv) - MengerReCenter) + 0.5 + axisOffset
    Apply 3x3 rotation matrix
    cross_de = min(max(rx,ry), max(rx,rz), max(ry,rz)) -- Menger cross section
    iter_de = cross_de / scale_factor
    accum_de = max(accum_de, iter_de)
    scale_factor *= MengerScale

  Step 4: if Apply_sa == 0: accum_de /= Scale

Parameters:
  [0]  MengerScale: double (default: 3)
  [1]  Scale: double (default: 2, .2Doubles)
  [2]  Z_add: double (default: 0, .2Doubles)
  [3]  Y_add: double (default: 0)
  [4]  X_add: double (default: 0)
  [5]  MengerDivisor: double (default: 3)
  [6]  Apply_sa: integer (default: 1)
  [7]  XOff: double (default: 0)
  [8]  YOff: double (default: 0)
  [9]  ZOff: double (default: 0)
  [10] MaxIter: integer (default: 4)
  [11-19] Rotation matrix (9 singles from .3SingleAngles)
  [20] MengerReCenter: double (default: 1.5)

Constants: [edi] = 0.5, [edi+8] = 3.0, [edi+0x10] = 1.5

Luca GN 2012
*/
