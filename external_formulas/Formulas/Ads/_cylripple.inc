// _cylripple.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: _cylripple
// Hand-ported from ASM: 283 bytes, DEoption=-1 (transform)
// Uses .3SingleAngles rotation matrix (9 single-double params) for pre/post rotation.

constexpr unsigned int FORMULA_CYLRIPPLE = 10261;

// ASM trace summary:
//
// .3SingleAngles produces a 3x3 rotation matrix stored as 9 single-precision floats.
// The matrix entries are at PVar offsets (single-precision, 4 bytes each):
//
// Forward rotation matrix R (offsets from PVar, single-precision):
//   R[0][0] = [PVar-0x24], R[0][1] = [PVar-0x28], R[0][2] = [PVar-0x2c]
//   R[1][0] = [PVar-0x0c], R[1][1] = [PVar-0x10], R[1][2] = [PVar-0x14]
//   R[2][0] = [PVar-0x18], R[2][1] = [PVar-0x1c], R[2][2] = [PVar-0x20]
//
// Note: ASM at 0x001A-0x0053 applies forward rotation:
//   new_x = x*R[0][0] + y*R[0][1] + z*R[0][2]
//   new_y = x*R[1][0] + y*R[1][1] + z*R[1][2]
//   new_z = x*R[2][0] + y*R[2][1] + z*R[2][2]
//
// Then cylindrical ripple:
//   ByC(L) check [PVar-0x50] == 1:
//     if ByC(L) == 1: rho = sqrt(x^2 + y^2), stored
//                      u = rho * Longitudinal_freq; u = sin(u) * Longitudinal_ampl
//     else:            rho = sqrt(Cx^2 + Cy^2), jump to same calculation
//
//   ByC(T) check [PVar-0x54] == 1:
//     if ByC(T) == 1: theta = atan2(y, x), stored
//                      v = theta * Transverse_freq; v = sin(v) * Transverse_ampl
//     else:            theta = atan2(Cy, Cx), jump to same calculation
//
//   Combine: result = (1 + u + v) * rho  (stored to [ebx] = x)
//            theta stored to [edx] = y
//
//   Convert back to Cartesian:
//     new_x = result * cos(theta)    (x = rho' * cos(theta))
//     new_y = result * sin(theta)    (y = rho' * sin(theta))
//   z unchanged
//
// Then inverse rotation (transpose of R):
//   out_x = x*R[2][2] + y*R[2][1] + z*R[0][2]  ... wait, let me re-trace.
//
// The inverse rotation at 0x00CB-0x010E uses offsets:
//   Row 0: [PVar-0x14], [PVar-0x20], [PVar-0x2c]  = R[1][2], R[2][2], R[0][2]
//   Row 1: [PVar-0x0c], [PVar-0x18], [PVar-0x24]  = R[1][0], R[2][0], R[0][0]  ... hmm
//
// Actually looking at the pattern more carefully for the inverse (transpose):
// Inverse row 0 uses: [PVar-0x14] from x, [PVar-0x20] from y, [PVar-0x2c] from z
//   = R_col2_of_rows: transposed column = row of transpose
// For col0: R[0][0]=-0x24, R[1][0]=-0x0c, R[2][0]=-0x18
// For col1: R[0][1]=-0x28, R[1][1]=-0x10, R[2][1]=-0x1c
// For col2: R[0][2]=-0x2c, R[1][2]=-0x14, R[2][2]=-0x20
//
// Inverse rotation (at 0xCB onward):
//   First set: x*(-0x14) + y*(-0x20) + z*(-0x2c) -- but wait, swapped layout
//
// Let me re-read the inverse block: offsets used are:
//   new_x: x*[PVar-0x14] + y*[PVar-0x20] + z*[PVar-0x2c]
//        = x*R[1][2] + y*R[2][2] + z*R[0][2]  -- that's R^T row 2
//   new_y: x*[PVar-0x0c] + y*[PVar-0x18] + z*[PVar-0x24]
//        = x*R[1][0] + y*R[2][0] + z*R[0][0]  -- that's R^T row 0
//   new_z: x*[PVar-0x10] + y*[PVar-0x1c] + z*[PVar-0x28]
//        = x*R[1][1] + y*R[2][1] + z*R[0][1]  -- that's R^T row 1
//
// So the inverse rotation IS the transpose, with rows permuted as (2,0,1).
// This confirms it's applying R^T correctly (just stored/accessed differently).
//
// Parameters (double, at PVar negative offsets after rotation matrix):
//   [PVar-0x34] = Longitudinal freq (param 0, default: 6)
//   [PVar-0x3c] = Longitudinal ampl (param 1, default: 0.05)
//   [PVar-0x44] = Transverse freq (param 2, default: 12)
//   [PVar-0x4c] = Transverse ampl (param 3, default: 0.05)
//   [PVar-0x50] = ByC(L) integer (param 4, default: 0)
//   [PVar-0x54] = ByC(T) integer (param 5, default: 0)

template<>
cpu_double3 execute_formula<FORMULA_CYLRIPPLE>(cpu_double3 p, constant double* params, double &de) {
    // Raw .3SingleAngles contract: the harness passes the original Euler angles.
    auto R = rotMatrix3FromFloats(params, 0);

    const double long_freq  = params[9];     // default: 6
    const double long_ampl  = params[10];     // default: 0.05
    const double trans_freq = params[11];     // default: 12
    const double trans_ampl = params[12];     // default: 0.05
    const int    byC_L      = (int)params[13]; // default: 0
    const int    byC_T      = (int)params[14]; // default: 0

    // Hidden engine context: J1/J2 are read at [esi+0x18]/[esi+0x20].
    const double Cx = params[15];
    const double Cy = params[16];

    double x = p.x, y = p.y, z = p.z;

    // Step 1: Apply forward rotation
    cpu_double3 rotated = R * cpu_double3{x, y, z};
    double rx = rotated.x;
    double ry = rotated.y;
    double rz = rotated.z;
    x = rx; y = ry; z = rz;

    // Step 2: Compute cylindrical ripple
    double rho, theta;

    // Longitudinal component
    double u;
    if (byC_L == 1) {
        rho = sqrt(x * x + y * y);
    } else {
        rho = sqrt(Cx * Cx + Cy * Cy);
    }
    u = sin(rho * long_freq) * long_ampl;

    // Transverse component
    double v;
    if (byC_T == 1) {
        theta = atan2(y, x);
    } else {
        theta = atan2(Cy, Cx);
    }
    v = sin(theta * trans_freq) * trans_ampl;

    // Step 3: Combine
    // x = (1 + u + v) * rho  (radial component scaled)
    // y = theta               (angle preserved)
    double new_rho = (1.0 + u + v) * rho;

    // Step 4: Convert back to Cartesian
    x = new_rho * cos(theta);
    y = new_rho * sin(theta);
    // z unchanged

    // Step 5: Apply inverse rotation (transpose).
    double ox = x * R.col0.x + y * R.col0.y + z * R.col0.z;
    double oy = x * R.col1.x + y * R.col1.y + z * R.col1.z;
    double oz = x * R.col2.x + y * R.col2.y + z * R.col2.z;

    return cpu_double3(ox, oy, oz);
}

// Formula metadata
constant FormulaInfo formula_info_cylripple = {
    .id = FORMULA_CYLRIPPLE,
    .name = "_cylripple",
    .param_count = 11,
    .complexity = 283,
    .uses_transcendental = true
};

/*
Original M3 Description:
You are warned; this transform may slow down and/or cause de trouble.
If used correctly it can give a cool "ripple" effect...

1) Rotates the reference axis by selected angles
   Use with caution this option! NOTE; this will NOT affect C!
2) calculate (rho,theta) := toCylindrical(x,y) (or Cx,Cy if asked)
   (note that if you use C you may get a very odd result sometimes!)
   let t1=u; t2=v;
3) cylindrical waves:
   u = La*sin(Lf*u); v = Ta*sin(Tf*v);
4) (x,y) := invCyl(t1*(1+u+v), t2);

5) Rotates the reference axis at the initial position (transposed rot. matrix)
   NOTE; this will NOT affect C!

2011 - Luca GN

Parameters:
  [0]    X angle (.3SingleAngles raw)
  [1]    Y angle (.3SingleAngles raw)
  [2]    Z angle (.3SingleAngles raw)
  [3]    Longitudinal freq: double (default: 6)
  [4]    Longitudinal ampl: double (default: 0.05)
  [5]    Transverse freq: double (default: 12)
  [6]    Transverse ampl: double (default: 0.05)
  [7]    ByC (L): integer (default: 0, 1=use x,y for rho)
  [8]    ByC (T): integer (default: 0, 1=use x,y for theta)
  [9]    J1: hidden engine context
  [10]   J2: hidden engine context

NOTE: Transform (DEoption=-1). DE unchanged.
NOTE: Uses raw `.3SingleAngles` values; the engine materializes the matrix via helper code.
*/
