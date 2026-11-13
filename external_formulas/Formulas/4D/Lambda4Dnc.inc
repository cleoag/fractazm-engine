// Lambda4Dnc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Lambda4Dnc
// Hand-ported from ASM: 302 bytes, DEoption=4
// 4D Lambda formula — non-commutative (Hamilton) quaternion variant.
// Computes: z → (-z² + P) * C  where multiplication is Hamilton product.

constexpr unsigned int FORMULA_LAMBDA4DNC = 6449;

// ASM trace (302 bytes, 0 branches):
//
// Phase 1: Modified quaternion squaring (negated Hamilton q²) with offsets
//   qx = -x² + y² + z² + w² + P0
//   qy = -2*x*y + P1
//   qz = -2*x*z + P2
//   qw = -2*x*w + P3
//
// Phase 2: Hamilton (non-commutative) quaternion product q * C
//   x' = qx*Cx - qy*Cy - qz*Cz - qw*Cw
//   y' = qx*Cy + qy*Cx + qz*Cw - qw*Cz
//   z' = qx*Cz - qy*Cw + qz*Cx + qw*Cy
//   w' = qx*Cw + qy*Cz - qz*Cy + qw*Cx
//
// For 3D (w=0, Cw=0):
//   Phase 1: qx=-x²+y²+z²+P0, qy=-2xy+P1, qz=-2xz+P2, qw=P3
//   Phase 2: x'=qx*Cx-qy*Cy-qz*Cz, y'=qx*Cy+qy*Cx-P3*Cz, z'=qx*Cz+qz*Cx+P3*Cy
//
// With defaults P0=1,P1=P2=P3=0 → standard Lambda formula: (1-z²)*C
//
// C handling: Formula handles C internally (DEoption=4).

template<>
cpu_double3 execute_formula<FORMULA_LAMBDA4DNC>(cpu_double3 p, constant double* params, double &de) {
    const double P0 = params[0];   // [edi-0x10] (default: 1.0)
    const double P1 = params[1];   // [edi-0x18] (default: 0.0)
    const double P2 = params[2];   // [edi-0x20] (default: 0.0)
    const double P3 = params[3];   // [edi-0x28] (default: 0.0, 4D qw offset)
    const double Cx = params[4];
    const double Cy = params[5];
    const double Cz = params[6];
    const double Cw = params[7];

    double x = p.x, y = p.y, z = p.z, w = de;

    // Phase 1: full 4D negated Hamilton quaternion squaring + offsets.
    double qx = -x * x + y * y + z * z + w * w + P0;
    double qy = -2.0 * x * y + P1;
    double qz = -2.0 * x * z + P2;
    double qw = -2.0 * x * w + P3;

    // Phase 2: full 4D Hamilton quaternion product q * C.
    double x_out = qx * Cx - qy * Cy - qz * Cz - qw * Cw;
    double y_out = qx * Cy + qy * Cx + qz * Cw - qw * Cz;
    double z_out = qx * Cz - qy * Cw + qz * Cx + qw * Cy;
    double w_out = qx * Cw + qy * Cz - qz * Cy + qw * Cx;

    de = w_out;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Lambda4Dnc = {
    .id = FORMULA_LAMBDA4DNC,
    .name = "Lambda4Dnc",
    .param_count = 7,
    .complexity = 302,
    .uses_transcendental = false
};

/*
Original M3 Description:
4D Lambda Mandelbrot (non-commutative / Hamilton quaternion).

The formula computes z → (-z² + P) * C using Hamilton quaternion
multiplication. With default P=(1,0,0,0), this is the standard
Lambda formula z → (1 - z²) * C.

Full 4D:
  Phase 1 (negated Hamilton squaring + offset):
    qx = -x² + y² + z² + w² + P0
    qy = -2xy + P1
    qz = -2xz + P2
    qw = -2xw + P3
  Phase 2 (Hamilton product q * C):
    x' = qx*Cx - qy*Cy - qz*Cz - qw*Cw
    y' = qx*Cy + qy*Cx + qz*Cw - qw*Cz
    z' = qx*Cz - qy*Cw + qz*Cx + qw*Cy
    w' = qx*Cw + qy*Cz - qz*Cy + qw*Cx

3D projection (w=0, Cw=0):
  qx = -x²+y²+z²+P0, qy = -2xy+P1, qz = -2xz+P2, qw = P3
  x' = qx*Cx - qy*Cy - qz*Cz
  y' = qx*Cy + qy*Cx - P3*Cz
  z' = qx*Cz + qz*Cx + P3*Cy

Parameters:
  [0] P0: double (default: 1.0) - real offset [edi-0x10]
  [1] P1: double (default: 0.0) - i-axis offset [edi-0x18]
  [2] P2: double (default: 0.0) - j-axis offset [edi-0x20]
  [3] P3: double (default: 0.0) - k-axis offset [edi-0x28]
  [4..6] Cx, Cy, Cz: passed by engine from context
*/
