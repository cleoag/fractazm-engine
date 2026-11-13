// Menger3.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Menger3
// Hand-ported from ASM: 249 bytes, Capstone-verified
// DEoption: 2 (analytic DE via w tracking)
// Two rotation matrices, sort network, conditional z-subtraction

constexpr unsigned int FORMULA_MENGER3 = 9526;

template<>
cpu_double3 execute_formula<FORMULA_MENGER3>(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    // Pre-computed rotation matrices from .3SingleAngles (9 floats each in PVar)
    RotMatrix3 RM1 = rotMatrix3FromFloats(params, 4);
    RotMatrix3 RM2 = rotMatrix3FromFloats(params, 13);

    double R1_00 = RM1.m[0][0]; double R1_01 = RM1.m[0][1]; double R1_02 = RM1.m[0][2];
    double R1_10 = RM1.m[1][0]; double R1_11 = RM1.m[1][1]; double R1_12 = RM1.m[1][2];
    double R1_20 = RM1.m[2][0]; double R1_21 = RM1.m[2][1]; double R1_22 = RM1.m[2][2];

    double R2_00 = RM2.m[0][0]; double R2_01 = RM2.m[0][1]; double R2_02 = RM2.m[0][2];
    double R2_10 = RM2.m[1][0]; double R2_11 = RM2.m[1][1]; double R2_12 = RM2.m[1][2];
    double R2_20 = RM2.m[2][0]; double R2_21 = RM2.m[2][1]; double R2_22 = RM2.m[2][2];

    // Step 1: Absolute values (ASM clears sign bit via AND 0x7FFFFFFF)
    double ax = abs(p.x);
    double ay = abs(p.y);
    double az = abs(p.z);

    // Step 2: Apply first rotation R1 to (|x|, |y|, |z|)
    // Standard: result = R1 * [ax, ay, az]^T
    double rx = ax * R1_00 + ay * R1_01 + az * R1_02;
    double ry = ax * R1_10 + ay * R1_11 + az * R1_12;
    double rz = ax * R1_20 + ay * R1_21 + az * R1_22;

    // Step 3: Sort network (3-element comparator)
    // Result: max_val >= mid_val >= min_val
    double max_val = rx;
    double mid_val = ry;
    double min_val = rz;
    if (max_val < mid_val) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (max_val < min_val) { double t = max_val; max_val = min_val; min_val = t; }
    if (mid_val < min_val) { double t = mid_val; mid_val = min_val; min_val = t; }

    // Step 4: Apply second rotation R2 to (max, mid, min).
    // The x87 block feeds the second matrix with the natural sorted order
    // (max, mid, min). Row groups map directly as:
    //   row0 -> params[13..15]
    //   row1 -> params[16..18]
    //   row2 -> params[19..21]
    double r0_R2 = max_val * params[13] + mid_val * params[14] + min_val * params[15];
    double r1_R2 = max_val * params[16] + mid_val * params[17] + min_val * params[18];
    double r2_R2 = max_val * params[19] + mid_val * params[20] + min_val * params[21];

    // Step 5: Scale + DE tracking
    // ASM: all three are scaled by Scale
    de = de * Scale;
    double k = Scale - 1.0;

    // Step 6: Final stores use the same row order: x <- row0, y <- row1, z <- row2.
    double x_out = r0_R2 * Scale - k * CScaleX;
    double y_out = r1_R2 * Scale - k * CScaleY;
    double z_scaled = r2_R2 * Scale;

    // Step 7: Conditional z-subtraction
    double threshold = 0.5 * k * CScaleZ;
    double z_out;
    if (z_scaled > threshold) {
        z_out = z_scaled - k * CScaleZ;
    } else {
        z_out = z_scaled;
    }

    cpu_double3 result;
    result.x = x_out;
    result.y = y_out;
    result.z = z_out;
    return result;
}

// Alternative direct function implementation
cpu_double3 Menger3_formula(cpu_double3 p, constant double* params, double &de) {
    const double Scale = params[0];
    const double CScaleX = params[1];
    const double CScaleY = params[2];
    const double CScaleZ = params[3];

    double a1 = params[4] * 3.14159265358979 / 180.0;
    double a2 = params[5] * 3.14159265358979 / 180.0;
    double a3 = params[6] * 3.14159265358979 / 180.0;
    double b1 = params[7] * 3.14159265358979 / 180.0;
    double b2 = params[8] * 3.14159265358979 / 180.0;
    double b3 = params[9] * 3.14159265358979 / 180.0;

    double s1 = sin(a1); double c1 = cos(a1);
    double s2 = sin(a2); double c2 = cos(a2);
    double s3 = sin(a3); double c3 = cos(a3);
    double R1_00 = c2*c3;            double R1_01 = -c2*s3;           double R1_02 = s2;
    double R1_10 = s1*s2*c3+c1*s3;   double R1_11 = c1*c3-s1*s2*s3;   double R1_12 = -s1*c2;
    double R1_20 = s1*s3-c1*s2*c3;   double R1_21 = c1*s2*s3+s1*c3;   double R1_22 = c1*c2;

    double t1 = sin(b1); double u1 = cos(b1);
    double t2 = sin(b2); double u2 = cos(b2);
    double t3 = sin(b3); double u3 = cos(b3);
    double R2_00 = u2*u3;            double R2_01 = -u2*t3;           double R2_02 = t2;
    double R2_10 = t1*t2*u3+u1*t3;   double R2_11 = u1*u3-t1*t2*t3;   double R2_12 = -t1*u2;
    double R2_20 = t1*t3-u1*t2*u3;   double R2_21 = u1*t2*t3+t1*u3;   double R2_22 = u1*u2;

    double ax = abs(p.x); double ay = abs(p.y); double az = abs(p.z);

    double rx = ax * R1_00 + ay * R1_01 + az * R1_02;
    double ry = ax * R1_10 + ay * R1_11 + az * R1_12;
    double rz = ax * R1_20 + ay * R1_21 + az * R1_22;

    double max_val = rx; double mid_val = ry; double min_val = rz;
    if (max_val < mid_val) { double t = max_val; max_val = mid_val; mid_val = t; }
    if (max_val < min_val) { double t = max_val; max_val = min_val; min_val = t; }
    if (mid_val < min_val) { double t = mid_val; mid_val = min_val; min_val = t; }

    // ASM row mapping: r0=params[19..21], r1=params[13..15], r2=params[16..18]
    double r0_R2 = max_val * R2_20 + mid_val * R2_21 + min_val * R2_22;  // params[19..21]
    double r1_R2 = max_val * R2_00 + mid_val * R2_01 + min_val * R2_02;  // params[13..15]
    double r2_R2 = max_val * R2_10 + mid_val * R2_11 + min_val * R2_12;  // params[16..18]

    de = de * Scale;
    double k = Scale - 1.0;
    double threshold = 0.5 * k * CScaleZ;

    cpu_double3 result;
    result.x = r1_R2 * Scale - k * CScaleX;
    result.y = r0_R2 * Scale - k * CScaleY;
    double z_scaled = r2_R2 * Scale;
    result.z = (z_scaled > threshold) ? z_scaled - k * CScaleZ : z_scaled;
    return result;
}

// Formula metadata
constant FormulaInfo formula_info_Menger3 = {
    .id = FORMULA_MENGER3,
    .name = "Menger3",
    .param_count = 10,
    .complexity = 860,
    .uses_transcendental = true
};

/*
Original M3 Description:
Menger sponge variant with two rotation matrices.

Mathematical Formula:
  1. Absolute values: x=|x|, y=|y|, z=|z|
  2. First rotation: [rx,ry,rz] = R1 * [|x|,|y|,|z|]
  3. Sort: arrange (rx,ry,rz) into (max, mid, min)
  4. Second rotation: [r2x,r2y,r2z] = R2 * [max,mid,min]
  5. Scale + DE: de *= Scale
  6. Offsets:
     x_out = r2x * Scale - (Scale-1) * CScaleX
     y_out = r2y * Scale - (Scale-1) * CScaleY
  7. Conditional z-subtraction (NOT symmetric fold):
     threshold = 0.5 * (Scale-1) * CScaleZ
     if r2z * Scale >= threshold:
       z_out = r2z * Scale - (Scale-1) * CScaleZ
     else:
       z_out = r2z * Scale

Parameters:
  [0] Scale: double (default: 3)
  [1] CScale_X: double (default: 1)
  [2] CScale_Y: double (default: 1)
  [3] CScale_Z: double (default: 1)
  [4] Rotation1_X: double (default: 0) - angle in degrees
  [5] Rotation1_Y: double (default: 0) - angle in degrees
  [6] Rotation1_Z: double (default: 0) - angle in degrees
  [7] Rotation2_X: double (default: 0) - angle in degrees
  [8] Rotation2_Y: double (default: 0) - angle in degrees
  [9] Rotation2_Z: double (default: 0) - angle in degrees

PVar layout (verified against ASM offsets):
  [PVar - 0x08] = 0.5 (marker constant, used in z-fold threshold)
  [PVar - 0x10] = Scale (double)
  [PVar - 0x18] = CScale_X (double)
  [PVar - 0x20] = CScale_Y (double)
  [PVar - 0x28] = CScale_Z (double)
  Rotation1 matrix (9 singles):
  [PVar - 0x2C] = R1[0,0]  [PVar - 0x30] = R1[0,1]  [PVar - 0x34] = R1[0,2]
  [PVar - 0x38] = R1[1,0]  [PVar - 0x3C] = R1[1,1]  [PVar - 0x40] = R1[1,2]
  [PVar - 0x44] = R1[2,0]  [PVar - 0x48] = R1[2,1]  [PVar - 0x4C] = R1[2,2]
  Rotation2 matrix (9 singles):
  [PVar - 0x50] = R2[0,0]  [PVar - 0x54] = R2[0,1]  [PVar - 0x58] = R2[0,2]
  [PVar - 0x5C] = R2[1,0]  [PVar - 0x60] = R2[1,1]  [PVar - 0x64] = R2[1,2]
  [PVar - 0x68] = R2[2,0]  [PVar - 0x6C] = R2[2,1]  [PVar - 0x70] = R2[2,2]

ASM register mapping:
  eax/@x -> ebx (saved), edx/@y, ecx/@z
  [ecx + 8] = w (DE tracking)
  [esi] = PVar (via [PIteration3D + 0x30])

Key differences from MengerIFS:
  - Two rotations (R1 before sort, R2 after sort) vs one (after sort)
  - R1 input: (|x|, |y|, |z|) standard
  - R2 input: (max, mid, min) — note: different column order than MengerIFS (mid,max,min)
  - Straight x/y mapping: row0->x, row1->y (MengerIFS cross-maps: row1->x, row0->y)
  - Conditional z-subtraction instead of symmetric abs fold
*/
