// NewMenger2IFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: NewMenger2IFS
// Hand-ported from ASM: 622 bytes, DEoption=20
// dIFS shape — SSE2 IFS + iterative Menger sponge with cylindrical holes

constexpr unsigned int FORMULA_NEWMENGER2IFS = 10308;

// SSE2 transforms XYZ; the x87 loop evaluates cylindrical holes in the
// mapped coordinates. FLD1/FSTP initializes only the accumulated scale.

template<>
cpu_double3 execute_formula<FORMULA_NEWMENGER2IFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters from .m3
    const double MengerScale = params[0];          // [PVar-0x10] Double MengerScale = 3
    const double Scale = params[1];                // [PVar-0x20] .2Doubles Scale = 2
    const double Z_add = params[2];                // [PVar-0x30] .2Doubles Z add = 0
    const double Y_add = params[3];                // [PVar-0x38] Double Y add = 0
    const double X_add = params[4];                // [PVar-0x40] Double X add = 0
    const double MengerDivisor = params[5];        // [PVar-0x48] Double MengerDivisor = 3
    const int Apply_sa_AddIts = int(params[6]);   // [PVar-0x4c] Integer Apply s+a/AddIts = 1
    const double XOff = params[7];                 // [PVar-0x54] Double XOff = .75
    const double YOff = params[8];                 // [PVar-0x5c] Double YOff = 0
    const double ZOff = params[9];                 // [PVar-0x64] Double ZOff = 0
    const int MaxIter_raw = int(params[10]);      // [PVar-0x68] Integer MaxIter (<32) = 4
    const double MengerReCenter = params[20];      // [PVar-0x94] Double MengerReCenter = 1.5
    // CONSTANTS:
    // [edi+0x00] = 0.5
    // [edi+0x08] = 3.0
    // [edi+0x10] = 1.5
    const double half_val = params[21];

    // Step 1: Standard SSE2 IFS transform
    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;
    double state_x = p.x;
    double state_y = p.y;
    double state_z = p.z;

    if (Apply_sa_AddIts != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        params[DIFS_COMMIT_XYZ_SLOT] = 1.0;
        state_x = x;
        state_y = y;
        state_z = z;
    }

    // Step 2: Clamp MaxIter to [1, 31]
    int MaxIter = MaxIter_raw & 0x1F;
    if (MaxIter == 0) MaxIter = 1;

    // Step 3: Initial scaling to [0,1] range
    // ASM: x = x * 0.5 + 0.5, same for y, z
    x = x * half_val + half_val;
    y = y * half_val + half_val;
    z = z * half_val + half_val;

    // The x87 loop keeps iterating the original pre-folded x/y/z values from the
    // [0,1]-mapped domain. Only the modulo scale factor changes per iteration.
    const double x0 = x;
    const double y0 = y;
    const double z0 = z;

    // Step 4: Menger iteration
    double accumulated_scale = 1.0;  // [esp+0x30]
    double best_de = fabs(x0 - half_val) - half_val;

    // The initial box distance is max(fx, fy, fz), without a lower bound of 1.
    {
        double fx0 = fabs(x0 - half_val) - half_val;
        double fy0 = fabs(y0 - half_val) - half_val;
        double fz0 = fabs(z0 - half_val) - half_val;
        if (fx0 > best_de) best_de = fx0;
        if (fy0 > best_de) best_de = fy0;
        if (fz0 > best_de) best_de = fz0;
    }

    for (int i = 0; i < MaxIter; i++) {
        // ASM uses fprem(value * MengerDivisor * accumulated_scale, MengerDivisor)
        // on the original mapped coordinates, then subtracts MengerReCenter.
        double mz = x87_fprem(z0 * MengerDivisor * accumulated_scale, MengerDivisor) - MengerReCenter;
        double my = x87_fprem(y0 * MengerDivisor * accumulated_scale, MengerDivisor) - MengerReCenter;
        double mx = x87_fprem(x0 * MengerDivisor * accumulated_scale, MengerDivisor) - MengerReCenter;

        // Apply the asm row order from [edi-0x84..-0x6c], which is not the
        // same row grouping as a straight rotMatrix3FromFloats() multiply.
        double rx_rot = mx * params[17] + my * params[18] + mz * params[19];
        double ry_rot = mx * params[11] + my * params[12] + mz * params[13];
        double rz_rot = mx * params[14] + my * params[15] + mz * params[16];

        // Compute cylindrical hole distances
        // d1 = sqrt(rx² + ry²) - ZOff * rz² - |rz| * YOff - XOff
        double d1 = sqrt(rx_rot * rx_rot + ry_rot * ry_rot)
                  - ZOff * rz_rot * rz_rot - fabs(rz_rot) * YOff - XOff;

        // d2 = sqrt(rx² + rz²) - ZOff * ry² - |ry| * YOff - XOff
        double d2 = sqrt(rx_rot * rx_rot + rz_rot * rz_rot)
                  - ZOff * ry_rot * ry_rot - fabs(ry_rot) * YOff - XOff;

        // d3 = sqrt(rz² + ry²) - ZOff * rx² - |rx| * YOff - XOff
        double d3 = sqrt(rz_rot * rz_rot + ry_rot * ry_rot)
                  - ZOff * rx_rot * rx_rot - fabs(rx_rot) * YOff - XOff;

        // Take minimum of the three
        double min_d = d1;
        if (d2 < min_d) min_d = d2;
        if (d3 < min_d) min_d = d3;

        // ASM scales via: -(min_d / (1 + |ZOff| + |YOff|)) / accumulated_scale
        double scaled_d = -(min_d / (1.0 + fabs(ZOff) + fabs(YOff))) / accumulated_scale;

        // Compare with best DE so far
        // FCOM keeps the new distance on unordered (NaN) comparisons.
        if (!(best_de > scaled_d)) {
            best_de = scaled_d;
        }

        accumulated_scale *= MengerScale;
    }

    double shape_de = best_de;

    // Scale correction
    if (Apply_sa_AddIts == 0) {
        shape_de /= Scale;
    }

    de = shape_de;

    return cpu_double3(state_x, state_y, state_z);
}

cpu_double3 NewMenger2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_NEWMENGER2IFS>(p, params, de);
}

constant FormulaInfo formula_info_NewMenger2IFS = {
    .id = FORMULA_NEWMENGER2IFS,
    .name = "NewMenger2IFS",
    .param_count = 21,
    .complexity = 622,
    .uses_transcendental = false
};

/*
Original M3 Description:
Fractal formula by Twinbee and Knighty, modified by me:

 d1=min(sqrt(pp.x,pp.z)-pp.y*offz-pp.y²*offy-offx, (... the same goes for other axis)

A Menger with cylindrical holes!

It has an unusual form, so it cannot be expressed in a "normal" way,
(for hybrids with other IFS / escapetime formulas)
but it's possible to implement it as a DIFS shape.
Very unique shapes are generated!

Luca GN 2012

The formula iterates a Menger-style fold (abs-fold around center) combined
with modular arithmetic (fprem) and cylindrical hole distance estimation.
Each iteration folds, scales, and computes the minimum cylindrical distance
across all three axis pairs.

Parameters:
  [0] MengerScale: double (default: 3) — iteration scale factor
  [1] Scale: double (default: 2) — IFS scale (.2Doubles)
  [2] Z add: double (default: 0)
  [3] Y add: double (default: 0)
  [4] X add: double (default: 0)
  [5] MengerDivisor: double (default: 3)
  [6] Apply s+a/AddIts: int (default: 1)
  [7] XOff: double (default: 0.75) — cylindrical hole base offset
  [8] YOff: double (default: 0) — linear term coefficient
  [9] ZOff: double (default: 0) — quadratic term coefficient
  [10] MaxIter (<32): int (default: 4) — number of Menger iterations
  [11..19] .3SingleAngles Rotation = 0 (rotation matrix)
  [20] MengerReCenter: double (default: 1.5) — re-centering offset after modulo

CONSTANTS:
  [0] 0.5
  [1] 3.0
  [2] 1.5
*/
