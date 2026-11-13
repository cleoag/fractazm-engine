// hyperd_1.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: hyperd_1
// Hand-ported from ASM: 257 bytes, DEoption=4
// Hypercomplex power formula: q^N + C with configurable cross-coupling.

constexpr unsigned int FORMULA_HYPERD_1 = 9986;

// ASM trace (257 bytes, loop with 2 branches for power clamping):
//
// Power N is a 4-byte integer at [edi-0x14].
// Valid range is [1, 16]; values outside that range default to 2.
// Effective minimum is q^2 (one multiply).
//
// Setup:
//   x0=x, y0=y, z0=z, w0=w+W_add   where w is the incoming 4th coordinate
//   counter = N - 2
//
// Loop body (N-1 multiplications, result = q^N):
//   Multiply accumulated (x0,y0,z0,w0) by original (x,y,z,w):
//
//   new_x = x*x0 - y*y0 - z*z0 - w*w0 + (z*w0 + w*z0)*P1
//   new_y = x*y0 + y*x0
//   new_z = x*z0 + z*x0 + (y*w0 + w*y0)*P2
//   new_w = x*w0 + w*x0 + (y*z0 + z*y0)*P3
//
//   dec counter; if counter > 0, update (x0,y0,z0,w0) = (new_x,...) and repeat
//
// After loop: add C
//   w_out = new_w + Cw  (4D only; CPU validation path currently carries w in `de`)
//   z_out = new_z + Cz
//   y_out = new_y + Cy
//   x_out = new_x + Cx
//
// With P1=P2=P3=0, w=0, W_add=0:
//   new_x = x*x0 - y*y0 - z*z0  (triplex real)
//   new_y = x*y0 + y*x0          (triplex i)
//   new_z = x*z0 + z*x0          (triplex j)
//   new_w = 0                     (stays zero)
//
// C handling: Formula adds C internally after the loop.

template<>
cpu_double3 execute_formula<FORMULA_HYPERD_1>(cpu_double3 p, constant double* params, double &de) {
    const double W_add  = params[0];   // [edi-0x10] (default: 0.0)
    int power           = int(params[1]);   // [edi-0x14] (default: 8)
    const double P1     = params[2];   // [edi-0x1C] ZW_mul (default: 0.0)
    const double P2     = params[3];   // [edi-0x24] YW_mul (default: 1.0)
    const double P3     = params[4];   // [edi-0x2C] YZ_mul (default: 1.0)
    const double Cx     = params[5];
    const double Cy     = params[6];
    const double Cz     = params[7];

    if (power < 1 || power > 16) {
        power = 2;
    }

    // Original point (constant throughout loop)
    const double x = p.x, y = p.y, z = p.z;
    const double w = de;  // DEoption=4: w is carried through de

    // Accumulated result, initialized to original point
    double x0 = x, y0 = y, z0 = z;
    double w0 = w + W_add;

    // Power loop: multiply N-1 times to get q^N
    // ASM does dec twice before first body, then dec once per loop iteration.
    // Effective: N-1 multiplications for power N (minimum 1 multiplication).
    int count = power - 1;
    if (count < 1) count = 1;

    for (int i = 0; i < count; i++) {
        // Hypercomplex multiplication: (x,y,z,w) × (x0,y0,z0,w0)
        double nx = x * x0 - y * y0 - z * z0 - w * w0
                 + (z * w0 + w * z0) * P1;
        double ny = x * y0 + y * x0;
        double nz = x * z0 + z * x0
                 + (y * w0 + w * y0) * P2;
        double nw = x * w0 + w * x0
                 + (y * z0 + z * y0) * P3;

        x0 = nx; y0 = ny; z0 = nz; w0 = nw;
    }

    // Add C
    double x_out = x0 + Cx;
    double y_out = y0 + Cy;
    double z_out = z0 + Cz;
    de = w0;  // Cw is zero in current direct-validation contract.

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_hyperd_1 = {
    .id = FORMULA_HYPERD_1,
    .name = "hyperd_1",
    .param_count = 8,
    .complexity = 257,
    .uses_transcendental = false
};

/*
Original M3 Description:
Hypercomplex power formula: computes q^N + C using iterative multiplication.

The multiplication has configurable cross-coupling terms P1, P2, P3 that
control how the 4D components interact. With all coupling params = 0,
reduces to a triplex-like multiplication (no y-z or y-w cross terms in
the real part).

Full 4D:
  w0 = w + W_add
  Multiply N-1 times: (x,y,z,w) × accumulated
  new_x = x*x0 - y*y0 - z*z0 - w*w0 + (z*w0+w*z0)*P1
  new_y = x*y0 + y*x0
  new_z = x*z0 + z*x0 + (y*w0+w*y0)*P2
  new_w = x*w0 + w*x0 + (y*z0+z*y0)*P3
  Finally: result += C

3D projection (w=0, W_add=0, P1=P2=P3=0):
  new_x = x*x0 - y*y0 - z*z0
  new_y = x*y0 + y*x0
  new_z = x*z0 + z*x0
  (standard triplex multiplication)

Parameters:
  [0] W_add: double (default: 0.0) - w offset [edi-0x10]
  [1] P1: double (default: 0.0) - x coupling [edi-0x1C]
  [2] P2: double (default: 0.0) - z coupling [edi-0x24]
  [3] P3: double (default: 0.0) - w coupling [edi-0x2C]
  [4] power: int (default: 2, range [1,16]) - exponent [edi-0x14]
  [5..7] Cx, Cy, Cz: passed by engine from context
*/
