// _mulC3D.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _mulC3D
// Hand-ported from ASM: 296 bytes, DEoption=0
// 3D generalization of complex multiplication.
// Rotates C values, then performs complex-style multiplication in the XY plane
// with z-axis cross terms using cylindrical radii.

constexpr unsigned int FORMULA__MULC3D = 8540;

// ASM trace (296 bytes, 0 branches):
//
// Phase 1: Compute weighted C + xyz terms
//   term_x = Cx*CAdd_Scale + x*xyzAdd_Scale
//   term_y = Cy*CAdd_Scale + y*xyzAdd_Scale
//   term_z = Cz*CAdd_Scale + z*xyzAdd_Scale
//
//   NOTE: Terms are computed z-first, y-second, x-third (stack order),
//   but the math is the same for all axes.
//
// Phase 2: 3x3 rotation matrix multiply + constant offsets
//   The 9 matrix elements are single-precision floats (dword).
//   Matrix row → output mapping is permuted:
//     Rx = Mrow1 · term + Const_real_x     (row1: [edi-0x34/38/3c])
//     Ry = Mrow2 · term + Const_imag_y     (row2: [edi-0x40/44/48])
//     Rz = Mrow0 · term + Const_imag_z     (row0: [edi-0x4c/50/54])
//
// Phase 3: Cylindrical radii
//   r_xy = sqrt(x² + y²)          (original coordinates)
//   R_xy = sqrt(Rx² + Ry²)        (rotated C)
//
// Phase 4: Z output
//   z_out = (r_xy * Rz + R_xy * z) * Z_multiplier
//
// Phase 5: Scale factor for XY
//   scale = 1 - (z * Rz) / (r_xy * R_xy)
//
// Phase 6: Complex multiplication in XY plane
//   x_out = (x * Rx - y * Ry) * scale
//   y_out = (Rx * y + x * Ry) * scale
//
// C handling: Formula handles C internally (DEoption=0).
//   Cx/Cy/Cz read from [esi+0x18/20/28].

template<>
cpu_double3 execute_formula<FORMULA__MULC3D>(cpu_double3 p, constant double* params, double &de) {
    const double CAdd_Scale   = params[0];   // [edi-0x10] (default: 0.25)
    const double xyzAdd_Scale = params[1];   // [edi-0x18] (default: 0)
    const double Const_real_x = params[2];   // [edi-0x20] (default: 1)
    const double Const_imag_y = params[3];   // [edi-0x28] (default: 0)
    const double Const_imag_z = params[4];   // [edi-0x30] (default: 0)
    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles).
    // ASM uses the natural row mapping here: row0 -> Rx, row1 -> Ry, row2 -> Rz.
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 5);
    const double Mx0 = Rot.col0.x, Mx1 = Rot.col1.x, Mx2 = Rot.col2.x;  // row 0 → Rx
    const double My0 = Rot.col0.y, My1 = Rot.col1.y, My2 = Rot.col2.y;  // row 1 → Ry
    const double Mz0 = Rot.col0.z, Mz1 = Rot.col1.z, Mz2 = Rot.col2.z;  // row 2 → Rz
    const double Z_mul = params[14];  // [edi-0x5c] (default: 1)
    // C values appended by wrapper after PVar params
    const double Cx = params[15];
    const double Cy = params[16];
    const double Cz = params[17];

    double x = p.x, y = p.y, z = p.z;

    // Phase 1: Weighted C + coordinate terms
    double term_x = Cx * CAdd_Scale + x * xyzAdd_Scale;
    double term_y = Cy * CAdd_Scale + y * xyzAdd_Scale;
    double term_z = Cz * CAdd_Scale + z * xyzAdd_Scale;

    // Phase 2: 3x3 rotation + constant offsets
    double Rx = Mx0 * term_x + Mx1 * term_y + Mx2 * term_z + Const_real_x;
    double Ry = My0 * term_x + My1 * term_y + My2 * term_z + Const_imag_y;
    double Rz = Mz0 * term_x + Mz1 * term_y + Mz2 * term_z + Const_imag_z;

    // Phase 3: Cylindrical radii
    double r_xy = sqrt(x * x + y * y);
    double R_xy = sqrt(Rx * Rx + Ry * Ry);

    // Phase 4: Z output
    double z_out = (r_xy * Rz + R_xy * z) * Z_mul;

    // Phase 5: Scale factor
    double scale = 1.0 - (z * Rz) / (r_xy * R_xy);

    // Phase 6: Complex multiplication in XY plane
    double x_out = (x * Rx - y * Ry) * scale;
    double y_out = (Rx * y + x * Ry) * scale;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info__mulC3D = {
    .id = FORMULA__MULC3D,
    .name = "_mulC3D",
    .param_count = 12,
    .complexity = 296,
    .uses_transcendental = false
};

/*
Original M3 Description:
3D generalization of complex multiplication with rotated C values.

The formula:
1. Computes weighted sum of C values and coordinates:
   term = C * CAdd_Scale + coord * xyzAdd_Scale

2. Rotates terms through a 3x3 matrix (from 3 Euler angles) and adds constants:
   R = M * term + Const

3. Performs 3D "complex multiplication":
   - XY plane: standard complex multiply (x+iy)(Rx+iRy)
   - Z axis: cross terms of cylindrical radii
   - Scale factor: 1 - z*Rz/(r_xy*R_xy)

Mathematical Formula:
  term_i = Ci * CAdd_Scale + coord_i * xyzAdd_Scale
  (Rx, Ry, Rz) = M * term + (Const_real_x, Const_imag_y, Const_imag_z)

  r_xy = sqrt(x² + y²)
  R_xy = sqrt(Rx² + Ry²)
  scale = 1 - z*Rz / (r_xy * R_xy)

  x_out = (x*Rx - y*Ry) * scale
  y_out = (Rx*y + x*Ry) * scale
  z_out = (r_xy*Rz + R_xy*z) * Z_multiplier

C handling: NON-STANDARD — C is internally rotated and scaled.
Engine should NOT add C; template handles it.

Parameters:
  [0] CAdd_Scale: double (default: 0.25) - C weighting factor
  [1] xyzAdd_Scale: double (default: 0) - coordinate weighting factor
  [2] Const_real_x: double (default: 1) - x offset after rotation
  [3] Const_imag_y: double (default: 0) - y offset after rotation
  [4] Const_imag_z: double (default: 0) - z offset after rotation
  [5..13] Rotation matrix: 9 floats (pre-computed from 3 Euler angles)
  [14] Z_multiplier: double (default: 1) - z output scale
  [15..17] Cx, Cy, Cz: passed by engine from context
*/
