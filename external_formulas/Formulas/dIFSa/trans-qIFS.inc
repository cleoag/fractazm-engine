// trans-qIFS.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: trans-qIFS
// Hand-ported from ASM: 198 bytes, Capstone-verified
// DEoption: 21 (dIFS transform — non-conformal quadratic transformation)
// Quadratic folding of x and y axes

constexpr unsigned int FORMULA_TRANS_QIFS = 10250;

template<>
cpu_double3 execute_formula<FORMULA_TRANS_QIFS>(cpu_double3 p, constant double* params, double &de) {
    // Parameters (from M3 [OPTIONS]):
    //   .Double Add X = 0           -> params[0]
    //   .Double Add Y = 0           -> params[1]
    //   .Double Add Z = 0           -> params[2]
    //   .3SingleAngles Rotation = 0 -> params[3..5] (3 Euler angles in degrees)
    //   .Double Quadratic X = 2     -> params[12]
    //   .Double Quadratic Y = 2     -> params[13]
    const double Add_X = params[0];
    const double Add_Y = params[1];
    const double Add_Z = params[2];
    // Build rotation matrix from 3 Euler angles (raw .3SingleAngles input)
    RotMatrix3 R = rotMatrix3FromFloats(params, 3);
    const double Quadratic_X = params[12];
    const double Quadratic_Y = params[13];

    // ASM trace:
    // 0000-0051: Standard add + rotation block:
    //   z' = z + Add_Z; y' = y + Add_Y; x' = x + Add_X
    //   Apply 3x3 rotation -> rx, ry, rz
    //   Store rx -> [esi-0x78], ry -> [esi-0x70], rz -> [esi-0x68]
    //
    // 0055-0060: Tent fold on x:
    //   fld1 -> 1.0
    //   fld [esi-0x78] -> rx
    //   fabs -> |rx|
    //   fsubp -> 1 - |rx|
    //   fstp [esi-0x78] -> x = 1 - |rx|
    //
    // 0061-006C: Tent fold on y:
    //   fld1 -> 1.0
    //   fld [esi-0x70] -> ry
    //   fabs -> |ry|
    //   fsubp -> 1 - |ry|
    //   fstp [esi-0x70] -> y = 1 - |ry|
    //
    // 006E-0096: Quadratic on x (if x > 0):
    //   fld [esi-0x78] -> x
    //   fldz -> 0
    //   fcompp -> compare 0 vs x; pop both
    //   fnstsw ax; shr ah,1; jae 0x99 -> skip if x <= 0 (CF=0 means 0 >= x)
    //   Wait: fcompp compares st0 vs st1 = 0 vs x.
    //   If 0 < x: C0=1, shr ah,1 sets CF, jae (CF=0) does NOT jump -> execute block
    //   If 0 >= x: C0=0, jae jumps -> skip
    //   So block executes when x > 0.
    //
    //   Inside block:
    //   fld [edi-0x4c] -> Quadratic_X (=qx)
    //   fld1; fsubp -> a1 = qx - 1
    //   fld st(0) -> dup a1
    //   fld1; fsubrp -> b1 = 1 - a1 = 2 - qx
    //   fmul [esi-0x78] -> b1 * x
    //   fxch st(1) -> a1 on top
    //   fmul [esi-0x78] -> a1 * x
    //   fmul [esi-0x78] -> a1 * x * x = a1*x^2
    //   faddp -> a1*x^2 + b1*x
    //   fstp [esi-0x78] -> store new x
    //
    // 0099-00C4: Quadratic on y (if y > 0):
    //   Same pattern with [edi-0x54] = Quadratic_Y
    //   fld [esi-0x70] -> y
    //   fldz; fcompp -> compare 0 vs y
    //   If y > 0: a2 = qy - 1; b2 = 1 - a2 = 2 - qy
    //   y = a2*y^2 + b2*y

    double z_val = p.z + Add_Z;
    double y_val = p.y + Add_Y;
    double x_val = p.x + Add_X;

    // Step 1: Apply 3x3 rotation matrix
    cpu_double3 rv = R * cpu_double3(x_val, y_val, z_val);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Step 2: Tent fold
    double x = 1.0 - abs(rx);
    double y = 1.0 - abs(ry);

    // Step 3: Quadratic transformation on x (only if x > 0)
    if (x > 0.0) {
        double a1 = Quadratic_X - 1.0;
        double b1 = 1.0 - a1;  // = 2 - Quadratic_X
        x = a1 * x * x + b1 * x;
    }

    // Step 4: Quadratic transformation on y (only if y > 0)
    if (y > 0.0) {
        double a2 = Quadratic_Y - 1.0;
        double b2 = 1.0 - a2;  // = 2 - Quadratic_Y
        y = a2 * y * y + b2 * y;
    }

    return cpu_double3(x, y, rz);
}

// Alternative direct function implementation
cpu_double3 trans_qIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_TRANS_QIFS>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_trans_qIFS = {
    .id = FORMULA_TRANS_QIFS,
    .name = "trans-qIFS",
    .param_count = 8,
    .complexity = 680,
    .uses_transcendental = false
};

/*
Original M3 Description:
Non-conformal quadratic transformation of x and y (plus z if there is a rotation).
Similar to a folding but it's quite different when quadratic term is not = 1!

x = 1 - abs(x);
y = 1 - abs(y);
a1 = quadr_x - 1; b1 = 1 - a1;
a2 = quadr_y - 1; b2 = 1 - a2;

if (x > 0) {
   x = a1 * x * x + b1 * x; } // x = 0 -> newx = 0 and x = 1 -> newx = 1

if (y > 0) {
   y = a2 * y * y + b2 * y; } // y = 0 -> newy = 0 and y = 1 -> newy = 1

Mathematical Formula:
  DEoption=21 (dIFS transform — quadratic folding):
    Step 1: Add offsets + 3x3 rotation
    Step 2: Tent fold: x = 1 - |rx|, y = 1 - |ry|
    Step 3: If x > 0: x = (qx-1)*x^2 + (2-qx)*x
    Step 4: If y > 0: y = (qy-1)*y^2 + (2-qy)*y
    Output: (x, y, rz)

  The quadratic preserves endpoints: f(0)=0, f(1)=1
  When qx=1: a=0, b=1 -> f(x)=x (identity)
  When qx=2: a=1, b=0 -> f(x)=x^2 (parabolic)

Parameters:
  [0] Add_X: double (default: 0)
  [1] Add_Y: double (default: 0)
  [2] Add_Z: double (default: 0)
  [3..11] Rotation: 9 floats (row-major 3x3 matrix)
  [12] Quadratic_X: double (default: 2) — quadratic strength for X
  [13] Quadratic_Y: double (default: 2) — quadratic strength for Y

PVar layout (verified against x87 ASM offsets):
  [PVar - 0x10] = Add_X (double)
  [PVar - 0x18] = Add_Y (double)
  [PVar - 0x20] = Add_Z (double)
  [PVar - 0x24 .. -0x44] = Rotation matrix (9 singles)
  [PVar - 0x4c] = Quadratic_X (double)
  [PVar - 0x54] = Quadratic_Y (double)

Comparison notes:
  fcompp compares st0 vs st1. With fldz then fcompp: compares 0 vs x.
  If 0 < x (i.e. x > 0): C0=1, CF set after shr. jae (CF=0) does NOT jump.
  198 bytes.
*/
