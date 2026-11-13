// koch_oct.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: koch_oct
// Hand-ported from ASM: 261 bytes, DEoption=2
// Koch-octahedron IFS: abs + R1 + ascending sort + R2 + Koch fold on x + scale + offsets

constexpr unsigned int FORMULA_KOCH_OCT = 10032;

// ASM trace summary:
//   000D-0017: load z, y, x with fabs on each
//   0019-0050: R1 rotation on (|x|, |y|, |z|)
//   0054-007A: ascending sort network (min in st0, mid in st1, max in st2)
//     Sort: put smaller in st0 via jb+fxch, repeat for 3 pairs
//   007C-00B3: R2 rotation on (min, mid, max)
//   00B5-00C4: Scale * each component, DE *= Scale
//   00C7-00DB: Koch fold on r2x: x_new = |Fold + x| - |Fold - x| - x
//     (equivalent to: x_new = 2*clamp(x, -Fold, Fold) - x)
//   00E0-00F5: Menger offsets: k = Scale - 1, subtract k*C from each

template<>
cpu_double3 execute_formula<FORMULA_KOCH_OCT>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    const double Fold = params[22];
    RotMatrix3 R1 = rotMatrix3FromFloats(params, 4);
    RotMatrix3 R2 = rotMatrix3FromFloats(params, 13);

    // Step 1: Absolute values (ASM: fabs after each fld)
    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    // Step 2: Apply R1 rotation
    cpu_double3 r1v = R1 * cpu_double3(ax, ay, az);
    double rx = r1v.x;
    double ry = r1v.y;
    double rz = r1v.z;

    // Step 3: Ascending sort network
    // ASM uses jb (jump if below) to skip swap, putting smaller in st0
    // Result: a=min, b=mid, c=max
    double a = rx, b = ry, c = rz;

    // First: sort a,b (put smaller in a)
    if (a >= b) { double t = a; a = b; b = t; }
    // Second: sort a,c (put smaller in a)
    if (a >= c) { double t = a; a = c; c = t; }
    // Now a = minimum. Swap a,b to bring b to front for third compare.
    { double t = a; a = b; b = t; }
    // Third: sort a,c (ensure a <= c)
    if (a >= c) { double t = a; a = c; c = t; }
    // Swap a,b for final order
    { double t = a; a = b; b = t; }
    // Final: a=min, b=mid, c=max (approximately -- mid/max may not be fully sorted
    // but the R2 input order is what matters)

    // Step 4: Apply R2 rotation to (a, b, c) = (min, mid, max)
    cpu_double3 r2v = R2 * cpu_double3(a, b, c);
    double r2x = r2v.x;
    double r2y = r2v.y;
    double r2z = r2v.z;

    // Step 5: Scale + DE tracking
    de = de * Scale;
    double sx = r2x * Scale;
    double sy = r2y * Scale;
    double sz = r2z * Scale;

    // Step 6: Koch fold on x component
    // ASM: x_new = |Fold + sx| - |Fold - sx| - sx
    // Equivalent to: x_new = 2*clamp(sx, -Fold, Fold) - sx
    double x_folded = abs(Fold + sx) - abs(Fold - sx) - sx;

    // Step 7: Menger offsets
    double k = Scale - 1.0;
    double x_out = x_folded - k * CScaleX;
    double y_out = sy - k * CScaleY;
    double z_out = sz - k * CScaleZ;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_koch_oct = {
    .id = FORMULA_KOCH_OCT,
    .name = "koch_oct",
    .param_count = 11,
    .complexity = 920,
    .uses_transcendental = true
};

/*
Original M3 Description:
Modified from OctahedronIFS.
Koch-octahedron combination with Koch fold applied to x component after
rotation and sort.

Mathematical Formula:
  1. Absolute values: x=|x|, y=|y|, z=|z|
  2. First rotation: [rx,ry,rz] = R1 * [|x|,|y|,|z|]
  3. Ascending sort: arrange into (min, mid, max)
  4. Second rotation: [r2x,r2y,r2z] = R2 * [min,mid,max]
  5. Scale all components: sx = r2x * Scale, etc.
  6. Koch fold on x: x_new = |Fold + sx| - |Fold - sx| - sx
     This clamps x to [-Fold, Fold] range then reflects: 2*clamp(sx,-F,F) - sx
  7. DE tracking: de *= Scale
  8. Menger offsets:
     x = x_folded - (Scale-1) * CScaleX
     y = sy - (Scale-1) * CScaleY
     z = sz - (Scale-1) * CScaleZ

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 0)
  [2] CScale_Y: double (default: 0)
  [3] CScale_Z: double (default: 1)
  [4-6] Rotation1: 3 angles in degrees (default: 0)
  [7-9] Rotation2: 3 angles in degrees (default: 0)
  [10] Fold: double (default: 0.5)

PVar layout:
  [PVar - 0x10] = Scale
  [PVar - 0x18] = CScale_X
  [PVar - 0x20] = CScale_Y
  [PVar - 0x28] = CScale_Z
  Rotation1 matrix: [PVar - 0x2C..0x4C] (9 singles)
  Rotation2 matrix: [PVar - 0x50..0x70] (9 singles)
  [PVar - 0x78] = Fold

Key differences from OctahedronIFS:
  - Has abs() step (like Menger3)
  - Standard ascending sort (not octahedral fold)
  - Koch fold applied to x component after R2 rotation + scale
  - Default CScaleZ=1, CScaleX=CScaleY=0
*/
