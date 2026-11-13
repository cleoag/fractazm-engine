// Lambda4Dc.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Lambda4Dc
// Hand-ported from ASM: 330 bytes, DEoption=4
// 4D Lambda formula — commutative (tessarine/bicomplex) variant.
// Computes: z → (-z² + P) * C  where multiplication is tessarine product.

constexpr unsigned int FORMULA_LAMBDA4DC = 3869;

// ASM trace (330 bytes, 0 branches):
//
// Phase 1: Negated tessarine squaring with offsets
//   qx = -x² + y² + z² - w² + P0
//   qy = -2*x*y + 2*z*w + P1
//   qz = -2*x*z + 2*y*w + P2
//   qw = -2*x*w - 2*y*z + P3
//
// Phase 2: Tessarine (commutative) quaternion product q * C
//   x' = qx*Cx - qy*Cy - qz*Cz + qw*Cw
//   y' = qy*Cx + qx*Cy - qw*Cz - qz*Cw
//   z' = qz*Cx - qw*Cy + qx*Cz - qy*Cw
//   w' = qw*Cx + qz*Cy + qy*Cz + qx*Cw
//
// For 3D (w=0, Cw=0):
//   Phase 1: qx=-x²+y²+z²+P0, qy=-2xy+P1, qz=-2xz+P2, qw=-2yz+P3
//   Phase 2: x'=qx*Cx-qy*Cy-qz*Cz
//            y'=qx*Cy+qy*Cx-qw*Cz
//            z'=qx*Cz-qw*Cy+qz*Cx
//
// Note: differs from Lambda4Dnc in cross-coupling (yz terms in 3D).
//
// C handling: Formula handles C internally (DEoption=4).

template<>
cpu_double3 execute_formula<FORMULA_LAMBDA4DC>(cpu_double3 p, constant double* params, double &de) {
    const double P0 = params[0];   // [edi-0x10] (default: 1.0)
    const double P1 = params[1];   // [edi-0x18] (default: 0.0)
    const double P2 = params[2];   // [edi-0x20] (default: 0.0)
    const double P3 = params[3];   // [edi-0x28] (default: 0.0)
    const double Cx = params[4];
    const double Cy = params[5];
    const double Cz = params[6];
    const double Cw = params[7];

    double x = p.x, y = p.y, z = p.z, w = de;

    // Phase 1: full 4D negated tessarine squaring + offsets.
    double qx = -x * x + y * y + z * z - w * w + P0;
    double qy = -2.0 * x * y + 2.0 * z * w + P1;
    double qz = -2.0 * x * z + 2.0 * y * w + P2;
    double qw = -2.0 * x * w - 2.0 * y * z + P3;

    // Phase 2: full 4D tessarine product q * C.
    double x_out = qx * Cx - qy * Cy - qz * Cz + qw * Cw;
    double y_out = qy * Cx + qx * Cy - qw * Cz - qz * Cw;
    double z_out = qz * Cx - qw * Cy + qx * Cz - qy * Cw;
    double w_out = qw * Cx + qz * Cy + qy * Cz + qx * Cw;

    de = w_out;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_Lambda4Dc = {
    .id = FORMULA_LAMBDA4DC,
    .name = "Lambda4Dc",
    .param_count = 7,
    .complexity = 330,
    .uses_transcendental = false
};

/*
Original M3 Description:
4D Lambda Mandelbrot (commutative / tessarine multiplication).

Uses tessarine (bicomplex) algebra where ij=ji=k, i²=-1, j²=-1, k²=+1.
The formula computes z → (-z² + P) * C using tessarine multiplication.
With default P=(1,0,0,0), this is the Lambda formula z → (1 - z²) * C.

Differs from Lambda4Dnc (Hamilton) in:
- Phase 1 squaring: tessarine vs Hamilton (sign flips on w² and cross terms)
- Phase 2 product: commutative vs non-commutative (sign flips on Cw/qw terms)
- In 3D (w=0): qw = -2yz+P3 (non-trivial), vs qw=P3 for nc variant

Full 4D:
  Phase 1 (negated tessarine squaring + offset):
    qx = -x² + y² + z² - w² + P0
    qy = -2xy + 2zw + P1
    qz = -2xz + 2yw + P2
    qw = -2xw - 2yz + P3
  Phase 2 (tessarine product q * C):
    x' = qx*Cx - qy*Cy - qz*Cz + qw*Cw
    y' = qy*Cx + qx*Cy - qw*Cz - qz*Cw
    z' = qz*Cx - qw*Cy + qx*Cz - qy*Cw
    w' = qw*Cx + qz*Cy + qy*Cz + qx*Cw

3D projection (w=0, Cw=0):
  qx = -x²+y²+z²+P0, qy = -2xy+P1, qz = -2xz+P2, qw = -2yz+P3
  x' = qx*Cx - qy*Cy - qz*Cz
  y' = qx*Cy + qy*Cx - qw*Cz
  z' = qx*Cz - qw*Cy + qz*Cx

Parameters:
  [0] P0: double (default: 1.0) - real offset [edi-0x10]
  [1] P1: double (default: 0.0) - i-axis offset [edi-0x18]
  [2] P2: double (default: 0.0) - j-axis offset [edi-0x20]
  [3] P3: double (default: 0.0) - k-axis offset [edi-0x28]
  [4..6] Cx, Cy, Cz: passed by engine from context
*/
