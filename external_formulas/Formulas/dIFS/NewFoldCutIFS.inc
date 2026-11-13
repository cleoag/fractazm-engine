// NewFoldCutIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: NewFoldCutIFS
// Hand-ported from ASM: 559 bytes, Capstone-verified
// DEoption: 20 (dIFS shape — SSE2 IFS + 3SingleAngles + iterative fold-cut)
// Generates "cubic spirals" of various kinds

constexpr unsigned int FORMULA_NEWFOLDCUTIFS = 10234;

template<>
cpu_double3 execute_formula<FORMULA_NEWFOLDCUTIFS>(cpu_double3 p, constant double* params, double &de) {
    const double IterScale = params[0];           // [edi-0x10]
    const double Scale = params[1];               // [edi-0x20] .2Doubles
    const double preZ_off = params[2];            // [edi-0x30]
    const double preY_off = params[3];            // [edi-0x38]
    const double preX_off = params[4];            // [edi-0x40]
    // params[5] = SingleAngle XY final angle sin
    // params[6] = SingleAngle XY final angle cos
    const int Apply_NoRestore = int(params[7]);  // [edi-0x4c]: <0 skips apply, >0 skips restore
    const double XOff = params[8];                // [edi-0x54]
    const double YOff = params[9];                // [edi-0x5c]
    const double ZOff = params[10];               // [edi-0x64]
    const int MaxIter = int(params[11]);         // [edi-0x68] integer, & 0x1F, min 1
    // params[12..20] = 3SingleAngles rotation matrix

    // Constants: [edi+0x00] = 2000 (large initial DE)
    const double c_initDE = params[21];           // 2000.0

    // Step 1: Standard SSE2 IFS transform
    // Note: this formula stores pre-add result AND post-add result
    // ASM 0x03-0x2C: the preamble stores x*Scale to [esp] BEFORE adding offsets
    // Then adds offsets to xmm0/xmm1 and stores back to context
    // The pre-add values go to [esp], the post-add go to [esi-0x78] etc
    double pre_x = p.x * Scale;
    double pre_y = p.y * Scale;
    double pre_z = p.z * Scale;

    double x = pre_x + preX_off;
    double y = pre_y + preY_off;
    double z = pre_z + preZ_off;

    // Apply_NoRestore: if >= 0, apply scale+add (store back to context)
    // ASM 0x2C-0x46: checks if [edi-0x4c] < 0, if so skip store
    if (Apply_NoRestore >= 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    // Step 2: Initialize running DE to c_initDE
    // ASM 0x4B-0x53: fld1, store to [esp+0x40]; fld [edi], store to [esi-0x20]
    double running_scale = 1.0;  // [esp+0x40]
    double running_de = c_initDE; // stored to [esi-0x20] = output DE

    // Step 3: MaxIter clamping (& 0x1F, min 1)
    // ASM 0x56-0x61
    int iters = MaxIter & 0x1F;
    if (iters == 0) iters = 1;

    // Step 4: Iteration loop (0x66-0x216)
    for (int it = 0; it < iters; it++) {
        // 4a: 3SingleAngles rotation of (pre_x, pre_y, pre_z)
        // ASM 0x66-0xB1: standard 3x3 matrix multiply
        double m00 = params[12], m01 = params[13], m02 = params[14];
        double m10 = params[15], m11 = params[16], m12 = params[17];
        double m20 = params[18], m21 = params[19], m22 = params[20];
        double rx = pre_x * m00 + pre_y * m01 + pre_z * m02;
        double ry = pre_x * m10 + pre_y * m11 + pre_z * m12;
        double rz = pre_x * m20 + pre_y * m21 + pre_z * m22;

        // 4b: Scale each rotated coord by IterScale, take abs, compute max
        // ASM 0xB3-0xB6: rx * [edi-0x10] (IterScale)
        double ax = rx * IterScale;
        double abs_ax = fabs(ax); // [esp+0x30] holds running max

        // 4c: |rx*IterScale| - preX_off → store to [esp]
        double cut_x = abs_ax - preX_off; // ASM 0xC0: fsub [edi-0x40]

        // 4d: ry * IterScale, |ry*IterScale|
        double ay = ry * IterScale;
        double abs_ay = fabs(ay);

        // max(abs_ax, abs_ay)
        double cur_max = (abs_ax > abs_ay) ? abs_ax : abs_ay;

        // 4e: |ry*IterScale| - preY_off → store to [esp+8]
        double cut_y = abs_ay - preY_off; // ASM 0xE9: fsub [edi-0x38]

        // 4: rz * IterScale, |rz*IterScale|
        double az = rz * IterScale;
        double abs_az = fabs(az);

        // max(cur_max, abs_az)
        cur_max = (cur_max > abs_az) ? cur_max : abs_az;

        // 4g: |rz*IterScale| - preZ_off → store to [esp+0x10]
        double cut_z = abs_az - preZ_off; // ASM 0x113: fsub [edi-0x30]

        // 4h: Sort cut_x, cut_y, cut_z: find max, mid, min
        // ASM 0x11A-0x14B: bubble sort using fcom
        // First pass: sort st0,st1 (cut_x, cut_y) then compare with st2 (cut_z)
        // Result: st0=max, st1=mid, st2=min
        double sorted[3] = {cut_x, cut_y, cut_z};
        // Simple sort of 3 values (ascending for min at [0], max at [2])
        if (sorted[0] > sorted[1]) { double t = sorted[0]; sorted[0] = sorted[1]; sorted[1] = t; }
        if (sorted[1] > sorted[2]) { double t = sorted[1]; sorted[1] = sorted[2]; sorted[2] = t; }
        if (sorted[0] > sorted[1]) { double t = sorted[0]; sorted[0] = sorted[1]; sorted[1] = t; }
        // Subtract each rank's offset, then sort the shifted values again.
        double v0 = sorted[1] - YOff; // mid - YOff
        double v1 = sorted[2] - XOff; // max - XOff
        double v2 = sorted[0] - ZOff; // min - ZOff

        // Second sort of v0, v1, v2
        if (v0 > v1) { double t = v0; v0 = v1; v1 = t; }
        if (v1 > v2) { double t = v1; v1 = v2; v2 = t; }
        if (v0 > v1) { double t = v0; v0 = v1; v1 = t; }
        // After: v0=min, v1=mid, v2=max

        // 4i: Compute scaled running_scale and shape DE
        // ASM 0x19B-0x1A6: running_scale *= |IterScale|
        running_scale *= fabs(IterScale);

        // 0x1AA: fld1; fsubr [esp+0x30] computes cur_max - 1.
        double numerator = cur_max - 1.0;

        // 0x1B0-0x1BF: max*cos + mid*sin (from SingleAngle XY final angle)
        // ASM: [esp]=max(v2), [esp+8]=mid(v1); fmul [edi-0x48]=cos, fmul [edi-0x44]=sin
        double sin_xy = params[5];
        double cos_xy = params[6];
        double denom_val = v2 * cos_xy + v1 * sin_xy;

        // 0x1BF-0x1CE: Check sign of IterScale — if negative, negate denom_val
        // ASM: fldz, fcompp(0 vs IterScale). fchs only when IterScale < 0.
        if (IterScale < 0.0) {
            denom_val = -denom_val;
        }

        // 0x1D0-0x1DC: max(denom_val, numerator)
        double iter_de;
        if (denom_val > numerator) {
            iter_de = denom_val;
        } else {
            iter_de = numerator;
        }

        // 0x1E0: fdivrp → iter_de / running_scale
        iter_de = iter_de / running_scale;

        // 0x1E2-0x1F5: running_de = min(running_de, iter_de)
        if (iter_de < running_de) {
            running_de = iter_de;
        }

        // 4k: Restore scaled coordinates only when Apply_NoRestore <= 0.
        // ASM 0x1F8-0x211: if [edi-0x4c] > 0 (jg 0x215), skip restore
        // If Apply_NoRestore <= 0: restore from [esp+0x18], [esp+0x20], [esp+0x28]
        // which are the pre-rotation scaled coords (ax, ay, az from step 4b)
        if (Apply_NoRestore <= 0) {
            pre_x = ax;
            pre_y = ay;
            pre_z = az;
        } else {
            // JG skips the restore, so the next rotation reads the second
            // sort's actual stack stores: maximum, middle, minimum. Keeping
            // the original pre_x/y/z here repeats the first shape forever.
            pre_x = v2;
            pre_y = v1;
            pre_z = v0;
        }
    }

    // Step 5: Scale correction
    // ASM 0x21C-0x228: if Apply_NoRestore >= 0 (not negative), divide by Scale
    if (Apply_NoRestore >= 0) {
        running_de /= Scale;
    }

    de = running_de;
    if (Apply_NoRestore >= 0) {
        return cpu_double3(x, y, z);
    }
    return p;
}

cpu_double3 NewFoldCutIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_NEWFOLDCUTIFS>(p, params, de);
}

constant FormulaInfo formula_info_NewFoldCutIFS = {
    .id = FORMULA_NEWFOLDCUTIFS,
    .name = "NewFoldCutIFS",
    .param_count = 22,
    .complexity = 559,
    .uses_transcendental = false
};

/*
Original M3 Description:
Loosely inspired by a Knighty script, and by a sculpture
seen in a Flickr album.
Generates "cubic spirals" of various kinds.
Moving adds/offsets results in different asymmetric
shapes of various kinds.
You can get a different type of intersection
using a pos/negative "iterscale".
To apply scale+add, USE 0 on the slider. (Unlike all other DIFS).
LucaGN 2015
*/
