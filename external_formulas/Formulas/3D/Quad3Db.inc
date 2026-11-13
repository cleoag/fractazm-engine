// Quad3Db.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Quad3Db
// Hand-ported from ASM: 376 bytes, DEoption=0 (standard fractal)
// Quadratic 3D formula with burning ship-like variants.

constexpr unsigned int FORMULA_QUAD3DB = 10264;

// ASM trace summary:
//
// Parameters:
//   [PVar-0x10] = X mul    (default: 1)
//   [PVar-0x18] = Y mul    (default: 2)
//   [PVar-0x20] = Z mul    (default: 2)
//   [PVar-0x28] = HRadius  (default: 0)
//   [PVar-0x30] = ZRadius  (default: 1)
//   [PVar-0x34] = Variants (integer, default: 40, bits 0-5 encode 3 x 2-bit modes)
//
// The engine may provide r2 = x^2 + y^2 at [esi+0x38]; this port recomputes it locally.
//
// Algorithm from ASM:
//
// 1. Check: R2 = x^2 + y^2 + HRadius
//    if R2 <= 0: output (0, 0, 0) + C  [all zeros, jump to 0x165]
//    if R2 == 0: output (Cx, Cy, Cz)    [jump to 0x16D]
//    Note: this is comparing 0 vs R2 (fldz; fcomp st1; sahf; ja/je)
//    If R2 > 0: continue
//    If R2 < 0 or == 0: special paths
//
//    Actually: at 0x14:
//    x^2 on stack, y^2 on stack, load HRadius, add all three -> R2 = x^2+y^2+HRadius
//    fldz (push 0), fcomp st1 (compare 0 vs R2), sahf
//    ja 0x165: if R2 < 0 (0 > R2): push 0,0,0 -> write to z,y,x -> exit
//    je 0x16D: if R2 == 0: push Cx,Cy,Cz -> write to z,y,x (effectively just C)
//    Otherwise fall through: R2 > 0
//
// 2. new_x computation:
//    val = (x^2 - y^2) * X_mul
//    temp = ZRadius - z^2/R2 (note: z^2/R2 via fdiv st(3) where st3 = R2)
//    val *= temp
//    Apply variant (bits 0-1 of Variants):
//      mode 0: new_x = |val| + Cx
//      mode 1: new_x = |val + Cx|
//      mode 2: new_x = |val+Cx| - |val-Cx| - val  (complex burning ship)
//      mode 3: new_x = val + Cx  (standard, no abs)
//
// 3. new_y computation:
//    val = x * y * Y_mul
//    temp = ZRadius - z^2/R2
//    val *= temp
//    Apply variant (bits 2-3 of Variants):
//      same 4 modes as above, using Cy
//
// 4. new_z computation:
//    val = z * sqrt(R2) * Z_mul
//    Apply variant (bits 4-5 of Variants):
//      same 4 modes, using Cz
//
// Output order: z, y, x stored to [ecx], [edx], [ebx]
// Pop remaining R2 from stack.
//
// The formula adds C inline. Return WITHOUT C (engine adds it).
// Wait - the formula does add C inside variant modes. So we need to be careful.
// Actually, looking at the ASM: each axis adds [esi+0x18/0x20/0x28] = Cx/Cy/Cz.
// So C IS added by the formula. But for DEoption=0, engine also adds C.
// This means we should return WITHOUT C and let engine add it.
// But the variants use C as part of abs patterns... so we need C.
//
// Re-examining: For standard formula (DEoption=0), engine adds C AFTER the formula call.
// The formula itself in ASM does add C. This means the engine is double-adding C.
// NO - for DEoption=0, the engine adds C ONLY if the formula doesn't.
// Actually, looking at CLAUDE.md: "Fractal formulas (DEoption >= 0) add C internally
// in ASM (fadd [esi+0x18/0x20/0x28]). Metal templates return the raw result WITHOUT C
// -- the engine adds it."
//
// So the ASM formula adds C, and the engine also adds C? That would be double-adding.
// The convention is: ASM adds C, Metal returns WITHOUT C (engine adds).
// So in Metal: we compute the raw result (no C) and return it.
// But the burning ship variants use C in the abs patterns...
//
// For mode 0: |val| + Cx -- we can return |val|, engine adds Cx
// For mode 1: |val + Cx| -- this uses C NON-LINEARLY (inside abs). Must keep C.
// For mode 2: |val+Cx| - |val-Cx| - val -- uses C non-linearly. Must keep C.
// For mode 3: val + Cx -- return val, engine adds Cx
//
// For modes 1 and 2, C is used non-linearly. These must include C and be flagged.
// For default Variants=40 (binary: 101000), bits: [5:4]=10, [3:2]=10, [1:0]=00
//   x-axis: mode 0 (|val| + C)
//   y-axis: mode 2 (complex burning ship)
//   z-axis: mode 2 (complex burning ship)
// So default uses C non-linearly for y and z axes!
//
// We'll include C as parameters and handle all modes correctly.
// The engine should NOT add C if the formula handles it.

template<>
cpu_double3 execute_formula<FORMULA_QUAD3DB>(cpu_double3 p, constant double* params, double &de) {
    const double X_mul     = params[0];  // default: 1
    const double Y_mul     = params[1];  // default: 2
    const double Z_mul     = params[2];  // default: 2
    const double HRadius   = params[3];  // default: 0
    const double ZRadius   = params[4];  // default: 1
    const int   Variants  = (int)params[5]; // default: 40

    // C components (needed for non-linear C usage in burning ship variants)
    const double Cx = params[6];
    const double Cy = params[7];
    const double Cz = params[8];

    double x = p.x, y = p.y, z = p.z;
    if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(z)) {
        return cpu_double3(0.0, 0.0, 0.0);
    }

    // R2 = x^2 + y^2 + HRadius
    double R2 = x * x + y * y + HRadius;
    if (!std::isfinite(R2)) {
        return cpu_double3(0.0, 0.0, 0.0);
    }

    // Degenerate cases
    if (R2 < 0.0) {
        return cpu_double3(0.0, 0.0, 0.0);
    }
    if (R2 == 0.0) {
        return cpu_double3(Cx, Cy, Cz);
    }

    // Extract 2-bit variant modes
    int mode_x = Variants & 3;
    int mode_y = (Variants >> 2) & 3;
    int mode_z = (Variants >> 4) & 3;

    // --- X axis ---
    // val_x = (x^2 - y^2) * X_mul * (ZRadius - z^2/R2)
    double val_x = (x * x - y * y) * X_mul * (ZRadius - z * z / R2);
    if (!std::isfinite(val_x)) {
        return cpu_double3(0.0, 0.0, 0.0);
    }
    double new_x;
    if (mode_x == 0) {
        new_x = abs(val_x) + Cx;
    } else if (mode_x == 1) {
        new_x = abs(val_x + Cx);
    } else if (mode_x == 2) {
        new_x = abs(val_x + Cx) - abs(val_x - Cx) + (-val_x);
        // ASM: fsubrp st(1) = st(1) = st(0) - st(1)
        // Then: fxch, fchs, faddp -> -(original val) + (|val+C|-|val-C|)
        // Actually re-reading: at 0x7D-0x95:
        //   fld st(0) -> dup val
        //   fadd Cx -> val+Cx
        //   fabs -> |val+Cx|
        //   fld st(1) -> dup val again
        //   fsub Cx -> val-Cx
        //   fabs -> |val-Cx|
        //   fxch, fsubrp st(1) -> |val+Cx| - |val-Cx|   (fsubrp: st1 = st0 - st1)
        //   fxch st(1) -> swap: now st0=val, st1=|val+Cx|-|val-Cx|
        //   fchs -> -val
        //   faddp st(1) -> |val+Cx|-|val-Cx| + (-val) = |val+Cx|-|val-Cx|-val
        new_x = abs(val_x + Cx) - abs(val_x - Cx) - val_x;
    } else {
        new_x = val_x + Cx;
    }

    // --- Y axis ---
    // val_y = x * y * Y_mul * (ZRadius - z^2/R2)
    double val_y = x * y * Y_mul * (ZRadius - z * z / R2);
    if (!std::isfinite(val_y)) {
        return cpu_double3(0.0, 0.0, 0.0);
    }
    double new_y;
    if (mode_y == 0) {
        new_y = abs(val_y) + Cy;
    } else if (mode_y == 1) {
        new_y = abs(val_y + Cy);
    } else if (mode_y == 2) {
        new_y = abs(val_y + Cy) - abs(val_y - Cy) - val_y;
    } else {
        new_y = val_y + Cy;
    }

    // --- Z axis ---
    // val_z = z * sqrt(R2) * Z_mul
    double val_z = z * sqrt(R2) * Z_mul;
    if (!std::isfinite(val_z)) {
        return cpu_double3(0.0, 0.0, 0.0);
    }
    double new_z;
    if (mode_z == 0) {
        new_z = abs(val_z) + Cz;
    } else if (mode_z == 1) {
        new_z = abs(val_z + Cz);
    } else if (mode_z == 2) {
        new_z = abs(val_z + Cz) - abs(val_z - Cz) - val_z;
    } else {
        new_z = val_z + Cz;
    }

    // NOTE: C is already included in the result due to burning ship variants.
    // Engine should NOT add C again for this formula.
    if (!std::isfinite(new_x) || !std::isfinite(new_y) || !std::isfinite(new_z)) {
        return cpu_double3(0.0, 0.0, 0.0);
    }
    return cpu_double3(new_x, new_y, new_z);
}

// Formula metadata
constant FormulaInfo formula_info_Quad3Db = {
    .id = FORMULA_QUAD3DB,
    .name = "Quad3Db",
    .param_count = 9,
    .complexity = 376,
    .uses_transcendental = false
};

/*
Original M3 Description:
Quadratic 3D plus variations (optional), that may transform the fractal into
burning ship-like customized variants.

Parameters:
  [0] X mul: double (default: 1)
  [1] Y mul: double (default: 2)
  [2] Z mul: double (default: 2)
  [3] HRadius: double (default: 0)
  [4] ZRadius: double (default: 1)
  [5] Variants (0...63): integer (default: 40)
  [6] Cx, [7] Cy, [8] Cz: from context

PVar layout:
  [PVar-0x10] = X mul
  [PVar-0x18] = Y mul
  [PVar-0x20] = Z mul
  [PVar-0x28] = HRadius
  [PVar-0x30] = ZRadius
  [PVar-0x34] = Variants (4-byte integer)

Variant encoding (6 bits):
  bits 0-1: X axis mode
  bits 2-3: Y axis mode
  bits 4-5: Z axis mode

  Mode 0: |val| + C        (abs before add)
  Mode 1: |val + C|        (abs after add)
  Mode 2: |val+C|-|val-C|-val  (burning ship complex variant)
  Mode 3: val + C          (standard, no abs)

Default 40 = 0b101000:
  X: mode 0 (|val|+Cx)
  Y: mode 2 (burning ship)
  Z: mode 2 (burning ship)

C handling: C is used non-linearly inside abs() for modes 1 and 2.
The formula includes C in the result. Engine should NOT add C again.

Base formula:
  R2 = x^2 + y^2 + HRadius
  new_x = variant((x^2-y^2) * Xmul * (ZRadius - z^2/R2), Cx, mode_x)
  new_y = variant(x*y * Ymul * (ZRadius - z^2/R2), Cy, mode_y)
  new_z = variant(z*sqrt(R2) * Zmul, Cz, mode_z)
*/
