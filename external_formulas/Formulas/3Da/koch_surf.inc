// koch_surf.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: koch_surf
// Hand-ported from ASM: 347 bytes, Capstone-verified
// DEoption: 2 (analytic DE via w tracking)
// Koch surface fractal with polyfold symmetric + rotation

constexpr unsigned int FORMULA_KOCH_SURF = 7227;

template<>
cpu_double3 execute_formula<FORMULA_KOCH_SURF>(cpu_double3 p, constant double* params, double &de) {
    const double PFOrder = params[0];         // polyfold order (sectors)
    const double PFShift_deg = params[1];     // polyfold angle shift (degrees)
    const double PFShift_x = params[2];       // x shift for polyfold center
    const double PFShift_y = params[3];       // y shift for polyfold center
    const double YOff1 = params[4];           // y fold offset (tent fold center)
    const double YOff2 = params[5];           // Koch fold offset (1/3 for standard Koch)
    const double Scale = params[6];
    const double CScale_Z = params[7];        // z offset (added before rot, subtracted after)
    const double CScale_Y = params[8];        // y offset
    const double CScale_X = params[9];        // x offset
    // Rotation matrix from 9 pre-computed float elements (from .3SingleAngles)
    RotMatrix3 R = rotMatrix3FromFloats(params, 10);
    const int Variation = int(params[19]);    // bit 0: Koch variation, bit 1: abs polyfold

    // Constants (from M3 CONSTANTS section)
    const double TWO_PI = 6.283185307;
    const double DEG2RAD = 0.017453293;

    double PFShift_rad = PFShift_deg * DEG2RAD;

    // Step 1: Shift position to polyfold center
    double x = p.x + PFShift_x;
    double y = p.y + PFShift_y;
    double z = p.z;

    // Step 2: Compute polyfold angle and sector index
    // ASM: fpatan(y, x) gives atan2(y, x) (standard azimuthal angle)
    double angle = atan2(y, x) + PFShift_rad;

    // Quantize: n = round(angle / (2*pi) * PFOrder)
    int n = int(rint(angle / TWO_PI * PFOrder));

    // Rotation angle to reference sector
    double rot_angle = -double(n) / PFOrder * TWO_PI + PFShift_rad;

    // Step 3: 2D rotation by polyfold angle
    double cs = cos(rot_angle);
    double ss = sin(rot_angle);
    // ASM computes: comp1 = y*ss - x*cs, comp2 = x*ss + y*cs
    double comp1 = y * ss - x * cs;    // rotated x (negated convention)
    double comp2 = x * ss + y * cs;    // rotated y

    // Step 4: Polyfold symmetric adjustment
    if (Variation & 2) {
        // Absolute mode (bit 1 set)
        comp2 = abs(comp2);
    } else if ((n & 1) == 0) {
        // Mirror symmetry: negate for even sectors
        comp2 = -comp2;
    }

    // Step 5: Y fold — tent fold centered at (PFShift_y + YOff1)
    // ASM: comp2 -= PFShift_y; comp2 -= YOff1; comp2 = |comp2|; comp2 = -comp2 + YOff1
    y = YOff1 - abs(comp2 - PFShift_y - YOff1);

    // Step 6: X adjust — remove polyfold shift
    x = comp1 - PFShift_x;

    // Step 7: First Koch fold
    // x += YOff2; if z > x: swap(x, z); x -= YOff2
    x += YOff2;
    if (z > x) { double tmp = x; x = z; z = tmp; }
    x -= YOff2;

    // Step 8: Second Koch fold (depends on Variation bit 0)
    if (Variation & 1) {
        // Variation: same direction as first fold
        x += YOff2;
        if (z > x) { double tmp = x; x = z; z = tmp; }
        x -= YOff2;
    } else {
        // Normal: reverse direction
        x -= YOff2;
        if (z > x) { double tmp = x; x = z; z = tmp; }
        x += YOff2;
    }

    // Step 9: Add offset before rotation
    double vx = x + CScale_X;
    double vy = y + CScale_Y;
    double vz = z + CScale_Z;

    // Step 10: Apply rotation matrix
    cpu_double3 rv = R * cpu_double3(vx, vy, vz);
    double rx = rv.x;
    double ry = rv.y;
    double rz = rv.z;

    // Step 11: Scale and subtract offset
    de = de * Scale;  // w tracking

    cpu_double3 result;
    result.x = rx * Scale - CScale_X;
    result.y = ry * Scale - CScale_Y;
    result.z = rz * Scale - CScale_Z;
    return result;
}

// Alternative direct function implementation
cpu_double3 koch_surf_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_KOCH_SURF>(p, params, de);
}

// Formula metadata
constant FormulaInfo formula_info_koch_surf = {
    .id = FORMULA_KOCH_SURF,
    .name = "koch_surf",
    .param_count = 14,
    .complexity = 780,
    .uses_transcendental = true
};

/*
Original M3 Description:
From a Knighty's script, modified a bit;

  // Fold
  z.xy = polyfoldsym(z.xy, shiftx, shifty, angleshift);
  z.y = YOff - abs(z.y - YOff);
  z.x += 1./3.; if(z.z > z.x) z.xz = z.zx; z.x -= 1./3.;
  // variation is
  // z.x += 1./3.; if(z.z > z.x) z.xz = z.zx; z.x -= 1./3.;
  z.x -= 1./3.; if(z.z > z.x) z.xz = z.zx; z.x += 1./3.; // normal formula
  z = Scale * (z + Offset);
  z = rot * z;
  z = z - Offset;

FORMULA AND IMPLEMENTATION; Luca G.N. 2013

Mathematical Formula:
  DEoption=2 (analytic DE via w tracking, w *= Scale each iteration):

  Step 1: Position shift to polyfold center:
    x += PFShift_x;  y += PFShift_y

  Step 2: Polyfold symmetric (PolyFoldSym):
    angle = atan2(y, x) + PFShift_deg * DEG2RAD
    n = round(angle / (2*pi) * PFOrder)
    rot_angle = -n / PFOrder * 2*pi + PFShift_rad
    Rotate (x, y) by rot_angle:
      comp1 = y*sin(rot) - x*cos(rot)
      comp2 = x*sin(rot) + y*cos(rot)
    If Variation & 2: comp2 = |comp2|
    Else if n is even: comp2 = -comp2

  Step 3: Y tent fold:
    y = YOff1 - |comp2 - PFShift_y - YOff1|

  Step 4: X adjust:
    x = comp1 - PFShift_x

  Step 5: First Koch fold:
    x += YOff2; if z > x: swap(x,z); x -= YOff2

  Step 6: Second Koch fold:
    If Variation & 1:
      x += YOff2; if z > x: swap(x,z); x -= YOff2
    Else (normal):
      x -= YOff2; if z > x: swap(x,z); x += YOff2

  Step 7: Rotate + Scale + Offset:
    v = (x, y, z) + CScale
    v_rot = R * v
    result = v_rot * Scale - CScale
    w *= Scale

Parameters:
  [0] PFOrder: double (default: 4) — polyfold sectors
  [1] PFShift (deg): double (default: 0) — polyfold angle shift
  [2] PFShift x: double (default: 0)
  [3] PFShift y: double (default: 0)
  [4] YOff1: double (default: 0.333333) — tent fold offset
  [5] YOff2: double (default: 0.333333) — Koch fold offset (1/3)
  [6] Scale: double (default: 2)
  [7] CScale Z: double (default: 1)
  [8] CScale Y: double (default: 0)
  [9] CScale X: double (default: 0.333333)
  [10..18] Rotation: 9 floats (from .3SingleAngles, row-major 3x3 matrix)
  [19] Variation: int (default: 0) — bit 0: Koch variant, bit 1: abs polyfold

PVar layout (verified against x87 ASM offsets):
  Constants (positive offsets):
  [PVar + 0x00] = 6.283185307 (2*pi)
  [PVar + 0x08] = 0.017453293 (pi/180, deg-to-rad)

  Options (negative offsets):
  [PVar - 0x10] = PFOrder (double)
  [PVar - 0x18] = PFShift_deg (double)
  [PVar - 0x20] = PFShift_x (double)
  [PVar - 0x28] = PFShift_y (double)
  [PVar - 0x30] = YOff1 (double)
  [PVar - 0x38] = YOff2 (double)
  [PVar - 0x40] = Scale (double)
  [PVar - 0x48] = CScale_Z (double)
  [PVar - 0x50] = CScale_Y (double)
  [PVar - 0x58] = CScale_X (double)
  [PVar - 0x5C .. -0x7C] = Rotation matrix (9 singles, row-major)
    R[0][0] at -0x5C, R[0][1] at -0x60, R[0][2] at -0x64
    R[1][0] at -0x68, R[1][1] at -0x6C, R[1][2] at -0x70
    R[2][0] at -0x74, R[2][1] at -0x78, R[2][2] at -0x7C
  [PVar - 0x80] = Variation (integer)

Standard formula register mapping:
  eax=@x (saved to ebx), edx=@y, ecx=@z
  [ebp + 8] = PIteration3D, [esi + 0x30] = PVar
  [ecx + 8] = w (DE tracking)

ASM rotation matrix application order:
  Row 0 (-> new x): [0x5C, 0x60, 0x64] = R[0][0..2]
  Row 1 (-> new y): [0x68, 0x6C, 0x70] = R[1][0..2]
  Row 2 (-> new z): [0x74, 0x78, 0x7C] = R[2][0..2]
  Computed as: row_C first, then row_A, then row_B (interleaved for stack efficiency)
  Stored as: x first (fstp [ebx]), y second (fstp [edx]), z third (fstp [ecx])

Key notes:
  - PolyFoldSym: quantize azimuthal angle to sectors, mirror alternate sectors
  - Koch fold: conditional swap of x and z around +/-YOff2 offset
  - Two Koch folds per iteration create the Koch surface fractal
  - Variation bit 0 controls Koch fold direction (normal vs same)
  - Variation bit 1 controls polyfold symmetry (abs vs sector parity)
  - YOff2 = 1/3 gives the classic Koch snowflake geometry
  - Rotation matrix from .3SingleAngles (3 Euler angles -> 9 singles via BuildRotMatrix)
  - Scale applied after rotation: result = Scale * R * (fold + CScale) - CScale
*/
