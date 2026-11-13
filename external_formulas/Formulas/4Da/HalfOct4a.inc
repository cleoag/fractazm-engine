// HalfOct4a.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: HalfOct4a
// Hand-ported from ASM: 341 bytes, DEoption=6
// 4D IFS formula: uses de parameter as w coordinate input/output
// 4D half-octahedral KIFS with a specific 7-comparison fold + 4D rotation + linear scale

constexpr unsigned int FORMULA_HALFOCT4A = 10345;

// ASM trace summary:
//
// Initial load (0x000E-0x0015):
//   fld [ecx+8]=w, fld [ecx]=z, fld [edx]=y, fld [ebx]=x
//   Stack: ST0=x, ST1=y, ST2=z, ST3=w
//
// Fold section (0x0018-0x007C): 7-comparison fold on all 4 values
//   Let (a,b,c,d) = (ST0,ST1,ST2,ST3) = (x,y,z,w) initially.
//
//   Phase 1: (x,y) half-oct-style fold (0x0018-0x0030)
//     fchs: a = -a
//     fcom st(1), jb: if a < b → jump (skip fxch); else fxch(a,b)
//     fchs: a = -a
//     fcom st(1), jae: if a >= b → jump (skip fxch); else fxch(a,b)
//     fchs: a = -a
//
//   Phase 2: (a,z) half-oct-style fold (0x0031-0x0048)
//     fcom st(2), jb: if a < c → skip; else fxch(a,c)
//     fchs: a = -a
//     fcom st(2), jb: if a < c → skip; else fxch(a,c)
//
//   Phase 3: HalfOct4a-specific w-folds (0x004C-0x007C)
//     fcom st(3), jb: if a < d → skip; else fxch(a,d)   [0x004C]
//     fxch st(1): unconditional swap(a,b)                [0x0058]
//     fcom st(3), jb: if a < d → skip; else fxch(a,d)   [0x005C]
//     fxch st(1): unconditional swap(a,b)                [0x0068]
//     fxch st(2): unconditional swap(a,c)                [0x006A]
//     fcom st(3), jae: if a >= d → skip; else fxch(a,d) [0x006E]  ← NOTE: jae (not jb)
//     fxch st(2): unconditional swap(a,c)                [0x007A]
//
// Rotation section (0x007D-0x00EC): apply 4x4 double rotation to (a,b,c,d)
//   sub edi, 0x28  — shift PVar ref for matrix access
//   x_out = a*R[0][0] + b*R[0][1] + c*R[0][2] + d*R[0][3] → [ebx]
//   y_out = a*R[1][0] + b*R[1][1] + c*R[1][2] + d*R[1][3] → [edx]
//   z_out = a*R[2][0] + b*R[2][1] + c*R[2][2] + d*R[2][3] → [ecx]
//   w_out = a*R[3][0] + b*R[3][1] + c*R[3][2] + d*R[3][3] → [ecx+8]
//   add edi, 0x28  — restore PVar ref
//
// Scale section (0x00F2-0x013):
//   Reload (w_out, z_out, y_out, x_out, Scale)
//   de *= Scale  (via [esi+0xe8] — engine DE accumulator)
//   Each coord *= Scale
//   k = 1 - Scale  (via fsubp/fchs)
//   coord_final = Scale*coord + k*C = Scale*coord - (Scale-1)*C
//   Store: x→[ebx], y→[edx], z→[ecx], w→[ecx+8]
//
// Matrix parameter layout (PVar offsets, singles):
//   params[5..8]   = R[0][0..3] at PVar-0x34,-0x38,-0x3c,-0x40
//   params[9..12]  = R[1][0..3] at PVar-0x44,-0x48,-0x4c,-0x50
//   params[13..16] = R[2][0..3] at PVar-0x54,-0x58,-0x5c,-0x60
//   params[17..20] = R[3][0..3] at PVar-0x64,-0x68,-0x6c,-0x70

template<>
cpu_double3 execute_formula<FORMULA_HALFOCT4A>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];   // [PVar-0x10] double
    const double CX    = params[1];   // [PVar-0x18]
    const double CY    = params[2];   // [PVar-0x20]
    const double CZ    = params[3];   // [PVar-0x28]
    const double CW    = params[4];   // [PVar-0x30]

    // .6SingleAngles → 6 raw angles in degrees, buildRotMatrix4d computes the 4x4 matrix
    // 4D rotation matrix: 16 pre-computed floats at params[5..20]

    // Fold: (a,b,c,d) starts as (x,y,z,w)
    double a = p.x, b = p.y, c = p.z, d = de;

    // Phase 1: (x,y) half-oct-style fold (0x0018-0x0030)
    // fchs, fcom st(1) jb, [fxch], fchs, fcom st(1) jae, [fxch], fchs
    a = -a;
    if (a >= b) { double t = a; a = b; b = t; }
    a = -a;
    if (a < b)  { double t = a; a = b; b = t; }
    a = -a;

    // Phase 2: (a,z) half-oct-style fold (0x0033-0x0048)
    // fcom st(2) jb, [fxch st(2)], fchs, fcom st(2) jb, [fxch st(2)]
    if (a >= c) { double t = a; a = c; c = t; }
    a = -a;
    if (a >= c) { double t = a; a = c; c = t; }

    // Phase 3: HalfOct4a w-folds (0x004C-0x007C)
    // 0x004C fcom st(3) jb → fxch(a,d) if a>=d
    if (a >= d) { double t = a; a = d; d = t; }
    // 0x0058 unconditional fxch(a,b)
    { double t = a; a = b; b = t; }
    // 0x005C fcom st(3) jb → fxch(a,d) if a>=d
    if (a >= d) { double t = a; a = d; d = t; }
    // 0x0068 unconditional fxch(a,b)
    { double t = a; a = b; b = t; }
    // 0x006A unconditional fxch(a,c)
    { double t = a; a = c; c = t; }
    // 0x006E fcom st(3) jae → fxch(a,d) if a<d  (jae skips when a>=d)
    if (a < d)  { double t = a; a = d; d = t; }
    // 0x007A unconditional fxch(a,c)
    { double t = a; a = c; c = t; }

    // 4D rotation using pre-computed matrix at params[5..20]
    double r0 = a*params[5]  + b*params[6]  + c*params[7]  + d*params[8];
    double r1 = a*params[9]  + b*params[10] + c*params[11] + d*params[12];
    double r2 = a*params[13] + b*params[14] + c*params[15] + d*params[16];
    double r3 = a*params[17] + b*params[18] + c*params[19] + d*params[20];

    // Linear scale + offset: coord' = Scale*coord - (Scale-1)*C
    double k = Scale - 1.0;
    double x_out = Scale * r0 - k * CX;
    double y_out = Scale * r1 - k * CY;
    double z_out = Scale * r2 - k * CZ;
    double w_out = Scale * r3 - k * CW;

    // de is the w coordinate for 4D iteration; engine handles DE accumulator separately
    de = w_out;
    return cpu_double3(x_out, y_out, z_out);
}

constant FormulaInfo formula_info_HalfOct4a = {
    .id = FORMULA_HALFOCT4A,
    .name = "HalfOct4a",
    .param_count = 11,
    .complexity = 341,
    .uses_transcendental = false
};

/*
Original M3 Description:
An attempt to make a 4D KIFS with half octahedral planes. w is folded like x,y,z.

Parameters:
  [0] Scale: double (default: 2)
  [1] CScale_X: double (default: 0)
  [2] CScale_Y: double (default: 0.5)
  [3] CScale_Z: double (default: 1.5)
  [4] CScale_W: double (default: 0.5)
  [5..20] 4x4 rotation matrix (16 floats, from .6SingleAngles Rotation)

PVar layout:
  [PVar-0x10] = Scale (double)
  [PVar-0x18] = CScale_X (double)
  [PVar-0x20] = CScale_Y (double)
  [PVar-0x28] = CScale_Z (double)
  [PVar-0x30] = CScale_W (double)
  [PVar-0x34..-0x40] = R[0][0..3] (singles, 4 bytes each)
  [PVar-0x44..-0x50] = R[1][0..3]
  [PVar-0x54..-0x60] = R[2][0..3]
  [PVar-0x64..-0x70] = R[3][0..3]
*/
