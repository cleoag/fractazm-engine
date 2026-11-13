// Sierpinski4.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Sierpinski4
// Hand-ported from ASM: 334 bytes, DEoption=6
// 4D IFS formula: uses de parameter as w coordinate input/output
// 4D Sierpinski tetrahedron with conditional swap folding, 4D rotation, linear scale

constexpr unsigned int FORMULA_SIERPINSKI4 = 10344;

// ASM trace:
//   Same fold pattern as MixPinski4 (Sierpinski conditional swap with negation).
//   Same 4D rotation.
//   DIFFERENT scaling: no Menger z-fold. Instead, simple linear scaling:
//     x' = Scale*x - CX*(Scale-1)
//     y' = Scale*y - CY*(Scale-1)
//     z' = Scale*z - CZ*(Scale-1)
//     w' = Scale*w - CW*(Scale-1)
//
// Key difference from Menger4/MixPinski4: after rotation, w' is computed
// as dot product → stored directly to [ecx+8]. Then x,y,z,w are scaled
// with simple linear offset (no Menger cross fold on z).
//
// ASM at 0x00E5-0x014B:
//   0x00E5: fstp [ecx+8] → store w_rot directly (not through Menger fold)
//   0x00E8: add edi, 0x28
//   0x00EB: fld w_rot, fld z, fld y, fld x
//   Stack: [x, y, z, w_rot]  (Wait, 0x00EB loads from [ecx+8], then [ecx], [edx], [ebx])
//     Actually: 0x00EB: fld [ecx+8]=w_rot (just stored)
//              0x00EE: fld [ecx]=z
//              0x00F0: fld [edx]=y
//              0x00F2: fld [ebx]=x
//   Stack: [x, y, z, w_rot]
//   0x00F4: fld [PVar-0x10]=Scale
//   Stack: [Scale, x, y, z, w_rot]
//   0x00FA: dup Scale, fmul [esi+0xE8] → DE_acc *= Scale
//   0x0108-0x010E: st4 *= Scale (w), st3 *= Scale (z), st2 *= Scale (y), st1 *= Scale (x)
//   Stack: [Scale, x*S, y*S, z*S, w*S]
//   0x0110: fld1, fsubp → Scale-1
//   0x0114: fchs → -(Scale-1) = 1-Scale
//   Stack: [1-Scale, x*S, y*S, z*S, w*S]
//
//   Then for each coordinate:
//   0x0116: fld [PVar-0x18]=CX, fmul st1=(1-Scale) → CX*(1-Scale)
//   0x011B: fadd st2=x*S → x*S + CX*(1-S) = x*S - CX*(S-1)
//   0x011D: fstp [ebx] → x_out
//
//   0x011: fld [PVar-0x20]=CY, fmul st1=(1-Scale) → CY*(1-Scale)
//   0x0124: fadd st3=y*S → y*S + CY*(1-S) = y*S - CY*(S-1)
//   0x0126: fstp [edx] → y_out
//
//   0x0128: fld [PVar-0x28]=CZ, fmul st1=(1-Scale)
//   0x012D: fadd st4=z*S → z_out
//   0x012: fstp [ecx] → z_out
//
//   0x0131: fld [PVar-0x30]=CW, fmul st1=(1-Scale)
//   0x0136: fadd st5=w*S → w_out
//   0x0138: fstp [ecx+8] → w_out
//
// So Sierpinski4 uses simple linear scaling for ALL 4 coordinates (no Menger cross fold).
// Also NO cross-mapping: x goes to x, y to y, etc.

template<>
cpu_double3 execute_formula<FORMULA_SIERPINSKI4>(cpu_double3 p, double* params, double &de) {
    // Parameters (doubles)
    const double Scale = params[0];    // [PVar-0x10]
    const double CX = params[1];       // [PVar-0x18]
    const double CY = params[2];       // [PVar-0x20]
    const double CZ = params[3];       // [PVar-0x28]
    const double CW = params[4];       // [PVar-0x30]

    // 4D rotation matrix: 16 pre-computed floats from .6SingleAngles (stored by Pascal engine)
    RotMatrix4 RM = rotMatrix4FromFloats(params, 5);

    double x = p.x, y = p.y, z = p.z, w = de;

    // Step 1: Sierpinski-type fold (conditional swap with negation)
    if (x + y < 0.0) { double t = x; x = -y; y = -t; }
    if (x + z < 0.0) { double t = x; x = -z; z = -t; }
    if (y + z < 0.0) { double t = y; y = -z; z = -t; }
    if (x + w < 0.0) { double t = x; x = -w; w = -t; }
    if (y + w < 0.0) { double t = y; y = -w; w = -t; }
    if (z + w < 0.0) { double t = z; z = -w; w = -t; }

    // Step 2: Apply 4D rotation
    double r0 = x*RM.m[0][0] + y*RM.m[0][1] + z*RM.m[0][2] + w*RM.m[0][3];
    double r1 = x*RM.m[1][0] + y*RM.m[1][1] + z*RM.m[1][2] + w*RM.m[1][3];
    double r2 = x*RM.m[2][0] + y*RM.m[2][1] + z*RM.m[2][2] + w*RM.m[2][3];
    double r3 = x*RM.m[3][0] + y*RM.m[3][1] + z*RM.m[3][2] + w*RM.m[3][3];

    // Step 3: Simple linear scale + offset for all 4 coordinates
    double k = Scale - 1.0;
    double x_out = Scale * r0 - k * CX;
    double y_out = Scale * r1 - k * CY;
    double z_out = Scale * r2 - k * CZ;
    double w_out = Scale * r3 - k * CW;

    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Sierpinski4 = {
    .id = FORMULA_SIERPINSKI4,
    .name = "Sierpinski4",
    .param_count = 11,
    .complexity = 334,
    .uses_transcendental = false
};

/*
Original M3 Description:
4D extension of the 3D Sierpinski KIFS, with a built-in 4D rotation.
If you try CScaleW=0 you get again the normal 3D Sierpinski solid!

Fold pattern (hyper-tetrahedral):
  if(x+y<0) { xy = -yx }
  if(x+z<0) { xz = -zx }
  if(y+z<0) { yz = -zy }
  if(x+w<0) { xw = -wx }
  if(y+w<0) { yw = -wy }
  if(z+w<0) { zw = -wz }

Then 4D rotation, then simple linear scale+offset:
  coord' = Scale * coord - CScale_coord * (Scale-1)

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 1)
  [4] CScale_W: double (default: 1)
  [5..20] 4x4 rotation matrix (16 floats, from .6SingleAngles Rotation)
*/
