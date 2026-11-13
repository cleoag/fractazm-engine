// cayley2IFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: cayley2IFS
// Hand-ported from ASM: 194 bytes, Capstone-verified
// DEoption: 21 (dIFS transform — modified inverse Cayley 2D transformation)
// Similar to a Julia-Newton transform in 2D

constexpr unsigned int FORMULA_CAYLEY2IFS = 10249;

template<>
cpu_double3 execute_formula<FORMULA_CAYLEY2IFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters (from M3 [OPTIONS]):
    //   .Double Add X = -0.04       -> params[0]
    //   .Double Add Y = -0.04       -> params[1]
    //   .Double Add Z = 0           -> params[2]
    //   .3SingleAngles Rotation = 0 -> params[3..5] (3 Euler angles in degrees)
    //   .Double Mul Z = 1.2         -> params[12]
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // params[3..5] = 3 Euler angles (degrees) for rotation matrix
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const double Mul_Z = params[12];

    // ASM trace:
    // 0000-0051: Standard add + rotation block:
    //   z' = z + Add_Z; y' = y + Add_Y; x' = x + Add_X
    //   Apply 3x3 rotation -> rx, ry, rz
    //   Store rx -> [esi-0x78], ry -> [esi-0x70]
    //
    // 0051-005: Scale z and handle DE:
    //   rz *= Mul_Z ([edi-0x4c]) -> fstp [esi-0x68]
    //   w *= |Mul_Z| -> DE tracking
    //
    // 0063-00C1: Cayley 2D transformation on (rx, ry):
    //   Allocate stack space: add esp, -0x1c (28 bytes)
    //
    //   x2 = rx*rx          (fld [esi-0x78]; fmul st0 -> [esp+0x10])
    //   y2 = ry*ry          (fld [esi-0x70]; fmul st0)
    //   diff = 2*(y2 - x2)  (y2 - x2; fadd st0 -> [esp+0x10] overwritten with 2*(y2-x2))
    //
    //   prod = y2 * x2 * 2  (actually: y2_copy * x2 * 2: fmul; fadd st0)
    //   then y2 squared and x2 squared and summed:
    //   m = x2*x2 + y2*y2 + 2*x2*y2 = (x2+y2)^2... no let me re-trace:
    //
    //   0080: fld st(0) = y2 dup
    //   0082: fmul st(2) = y2 * x2
    //   0084: fadd st(0) = 2*y2*x2
    //   0086: fxch st(1) -> st0=y2, st1=2*y2*x2
    //   0088: fmul st(0) -> y2*y2 = y4
    //   008A: fxch st(2) -> st0=x2, st2=y4
    //   008C: fmul st(0) -> x4
    //   008E: faddp st(1) -> x4 + 2*y2*x2
    //   0090: faddp st(1) -> x4 + 2*x2*y2 + y4 = (x2+y2)^2
    //   Wait, that's not right. Let me be more careful.
    //
    //   After 0074 (fld st(0)):
    //     st0=y2_copy, st1=y2, st2=x2  (x2 was stored at [esp+0x10])
    //   0076: fsub [esp+0x10] -> y2_copy - x2 = y2-x2
    //   007A: fadd st(0) -> 2*(y2-x2) -> store [esp+0x10]
    //   Stack after 007C fstp: st0=y2, st1=x2... wait, x2 was stored to memory.
    //   Let me re-trace from 0063:
    //
    //   0066: fld [esi-0x78] -> rx
    //   0069: fmul st(0) -> rx^2 = x2
    //   006B: fst [esp+0x10] -> save x2
    //   006: fld [esi-0x70] -> ry
    //   0072: fmul st(0) -> ry^2 = y2
    //   Stack: st0=y2, st1=x2
    //   0074: fld st(0) -> y2 dup
    //   0076: fsub [esp+0x10] -> y2 - x2
    //   007A: fadd st(0) -> 2*(y2-x2) -> numerator for new_y
    //   007C: fstp [esp+0x10] -> store 2*(y2-x2)
    //   Stack: st0=y2, st1=x2
    //   0080: fld st(0) -> y2 dup
    //   0082: fmul st(2) -> y2*x2
    //   Stack: st0=y2*x2, st1=y2, st2=x2
    //   0084: fadd st(0) -> 2*y2*x2
    //   Stack: st0=2*x2*y2, st1=y2, st2=x2
    //   0086: fxch st(1) -> st0=y2, st1=2*x2*y2, st2=x2
    //   0088: fmul st(0) -> y2^2 = y4
    //   Stack: st0=y4, st1=2*x2*y2, st2=x2
    //   008A: fxch st(2) -> st0=x2, st1=2*x2*y2, st2=y4
    //   008C: fmul st(0) -> x4
    //   Stack: st0=x4, st1=2*x2*y2, st2=y4
    //   008E: faddp st(1) -> x4+2*x2*y2
    //   Stack: st0=x4+2*x2*y2, st1=y4
    //   0090: faddp st(1) -> x4+2*x2*y2+y4 = (x2+y2)^2 = m
    //
    //   So m = (rx^2 + ry^2)^2
    //
    //   0092: fst [esp] -> store m
    //   0095: fld1 -> 1.0
    //   0097: fsubp -> m - 1
    //   Stack: st0 = m-1
    //
    //   0099: fld [esp] -> m
    //   009C: fld [esi-0x70] -> ry
    //   009: fmul [esi-0x78] -> ry*rx
    //   00A2: fadd st(0) -> 2*ry*rx
    //   00A4: fadd st(0) -> 4*ry*rx
    //   00A6: faddp st(1) -> m + 4*ry*rx
    //   Stack: st0 = m+4*rx*ry, st1 = m-1
    //   00A8: fld1 -> 1.0
    //   00AA: faddp -> m+4*rx*ry+1
    //   Stack: st0 = denom = m+4*rx*ry+1, st1 = m-1
    //
    //   00AC: fst [esp] -> store denom
    //   00AF: fdivp -> (m-1)/denom = new_x
    //   00B1: fstp [esi-0x78] -> store new_x
    //
    //   00B4: fld [esp+0x10] -> 2*(y2-x2) = numerator for y
    //   00B8: fdiv [esp] -> 2*(y2-x2)/denom = new_y
    //   00BB: fstp [esi-0x70] -> store new_y
    //
    //   Cayley2 formula:
    //     m = (x^2 + y^2)^2
    //     new_x = (m - 1) / (m + 4*x*y + 1)
    //     new_y = 2*(y^2 - x^2) / (m + 4*x*y + 1)

    double z_val = p.z + Add_Z;
    double y_val = p.y + Add_Y;
    double x_val = p.x + Add_X;

    // Step 1: Apply 3x3 rotation matrix (built from 3 Euler angles)
    cpu_double3 rv = R * cpu_double3(x_val, y_val, z_val);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Step 2: Scale z and DE tracking
    rz *= Mul_Z;
    de *= abs(Mul_Z);

    // Step 3: Cayley 2D transformation on (rx, ry)
    double x2 = rx * rx;
    double y2 = ry * ry;

    double m = (x2 + y2) * (x2 + y2);  // (x^2+y^2)^2
    double denom = m + 4.0 * rx * ry + 1.0;

    double new_x = (m - 1.0) / denom;
    double new_y = 2.0 * (y2 - x2) / denom;

    return cpu_double3(new_x, new_y, rz);
}

// Alternative direct function implementation
cpu_double3 cayley2IFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_CAYLEY2IFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_cayley2IFS = {
    .id = FORMULA_CAYLEY2IFS,
    .name = "cayley2IFS",
    .param_count = 7,
    .complexity = 680,
    .uses_transcendental = false
};

/*
Original M3 Description:
Modified inverse Cayley transformation. Similar to a Julia-Newton transform in 2D.
Try to change Rotation Z (example; -142 with the default parameters).
Disable always "apply Scale+add" and adjust sizes on ALL the shapes or the
transform will have a bad result!
Also try different sizes for each shape if the fractal structure is not well visible.

Formula (from an Ultra Fractal transform made by Javier Lopez Pena, August 2002)
Translate(x,y,z);
Rotate(x,y,z);
m = 2*((x*y)^2) + x^4 + y^4;
Cayley2_2D(x,y) = (m - 1, 2(y*y - x*x) ) / ( m + 4*x*y + 1);

Mathematical Formula:
  DEoption=21 (dIFS transform — Cayley 2D):
    Step 1: Add offsets + 3x3 rotation
    Step 2: Scale z by Mul_Z, DE *= |Mul_Z|
    Step 3: Cayley 2D transformation:
      m = (x^2 + y^2)^2
      new_x = (m - 1) / (m + 4*x*y + 1)
      new_y = 2*(y^2 - x^2) / (m + 4*x*y + 1)

Parameters:
  [0] Add_X: double (default: -0.04)
  [1] Add_Y: double (default: -0.04)
  [2] Add_Z: double (default: 0)
  [3..11] Rotation: 9 floats (row-major 3x3 matrix)
  [12] Mul_Z: double (default: 1.2) — Z axis scaling

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles)
  [PVar - 0x4c] = Mul_Z (double)

Note: m = x^4 + 2*x^2*y^2 + y^4 = (x^2+y^2)^2 as traced from ASM.
The description says m = 2*((x*y)^2) + x^4 + y^4 = 2*x^2*y^2 + x^4 + y^4 = same thing.
194 bytes.
*/
