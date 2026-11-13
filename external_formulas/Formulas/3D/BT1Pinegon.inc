// BT1Pinegon.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: BT1Pinegon
// Hand-ported from ASM: 430 bytes, DEoption=0 (standard fractal)
// Benesi T1 transform + Pine Tree Mandelbulb with polygon transform.

constexpr unsigned int FORMULA_BT1PINEGON = 10265;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void BT1PinehedronT1_x87(double* state);
extern "C" void BT1PinegonTail_x87(double* state, int flags);
#endif

// ASM trace summary:
//
// Constants: sqrt(2/3), sqrt(1/3), sqrt(1/2), pi
//
// Parameters (PVar offsets):
//   [PVar-0x10] = Scale (2.0), [PVar-0x18] = xOff (2.0)
//   [PVar-0x20] = pixel_x (1.0), [PVar-0x28] = pixel_y (0.0), [PVar-0x30] = pixel_z (0.0)
//   [PVar-0x38] = yz_angle (0.0 radians)
//   [PVar-0x3c] = AngleIter (62), [PVar-0x40] = A_AfterIter (0)
//   [PVar-0x44] = T1_Count (1, byte), [PVar-0x48] = PolyIter (2)
//   [PVar-0x4c] = PolyIterAfter (62), [PVar-0x54] = Sides (4.0)
//
// Algorithm:
//   1. T1 loop (T1_Count times): tetrahedron fold + scale
//   2. Optional yz rotation (if iter==AngleIter or iter>=A_AfterIter)
//   3. Optional polygon transform (if iter==PolyIter or iter>=PolyIterAfter)
//   4. Benesi Pine Tree: xt=x^2, yt=y^2, zt=z^2
//      t = 2*x/sqrt(yt+zt)
//      x_out = xt-yt-zt + Cx*pixel_x
//      y_out = 2*t*y*z + Cy*pixel_z    (ASM: [esi+0x20]*[PVar-0x30])
//      z_out = t*(yt-zt) + Cz*pixel_y  (ASM: [esi+0x28]*[PVar-0x28])

template<>
cpu_double3 execute_formula<FORMULA_BT1PINEGON>(cpu_double3 p, constant double* params, double &de) {
    const double Scale   = params[0];   // default: 2.0
    const double xOff    = params[1];   // default: 2.0
    const double pixel_x = params[2];   // default: 1.0
    const double pixel_y = params[3];   // default: 0.0
    const double pixel_z = params[4];   // default: 0.0
    const double yz_angle = params[5];  // default: 0.0 (radians)
    const int AngleIter     = (int)params[6];   // default: 62
    const int A_AfterIter   = (int)params[7];   // default: 0
    const unsigned countByte = static_cast<unsigned>(static_cast<int>(params[8])) & 255u;
    const unsigned T1_Count = countByte ? countByte : 256u;
    const int PolyIter      = (int)params[9];   // default: 2
    const int PolyIterAfter = (int)params[10];  // default: 62
    const double Sides   = params[11];  // default: 4.0
    const double s23     = params[12];  // [edi+0x00]
    const double s13     = params[13];  // [edi+0x08]
    const double s12     = params[14];  // [edi+0x10]
    const double pi_const= params[15];  // [edi+0x18]
    const double Cx = params[16];
    const double Cy = params[17];
    const double Cz = params[18];
    const unsigned iteration = static_cast<unsigned>(static_cast<int>(params[ITER_COUNT_SLOT])) & 255u;
    auto lowByte = [](int value) { return static_cast<unsigned>(value) & 255u; };
    auto signedByte = [](unsigned value) { return value < 128u ? int(value) : int(value)-256; };
    const bool rotate = iteration == lowByte(AngleIter) || signedByte(iteration) >= signedByte(lowByte(A_AfterIter));
    const bool polygon = iteration == lowByte(PolyIter) || signedByte(iteration) >= signedByte(lowByte(PolyIterAfter));

    double x = p.x, y = p.y, z = p.z;

    // T1 Loop
    for (unsigned i = 0; i < T1_Count; i++) {
#if defined(_MSC_VER) && defined(_M_X64)
        // The two shipped assets have identical 0019..00AD instruction bytes.
        double state[] = {x,y,z,s23,s13,s12,Scale,xOff};
        BT1PinehedronT1_x87(state);
        x=state[0]; y=state[1]; z=state[2];
#elif defined(MB3D_SOFTWARE_EXT80)
        external_pine_t1(x,y,z,s23,s13,s12,Scale,xOff);
#else
        // Step A: Tetrahedron fold
        double tx = (x * s23 - z * s13) * s12;
        z = abs(x * s13 + z * s23);
        x = abs(tx - y * s12);
        y = abs(tx + y * s12);

        // Step B: Inverse rotation
        double tx2 = x * s12 + y * s12;
        y = -x * s12 + y * s12;
        x = tx2 * s23 + z * s13;
        z = -tx2 * s13 + z * s23;

        // Step C: Scale and offset
        x = Scale * x - xOff;
        y = Scale * y;
        z = Scale * z;
#endif
    }

#if defined(_MSC_VER) && defined(_M_X64)
    // Preserve FPATAN argument order, signed-byte branches and Double stores.
    double tail[] = {x,y,z,yz_angle,Sides,pi_const,pixel_x,pixel_y,pixel_z,Cx,Cy,Cz};
    BT1PinegonTail_x87(tail, (rotate ? 1 : 0) | (polygon ? 2 : 0));
    return cpu_double3(tail[0],tail[1],tail[2]);
#else
    // YZ-plane rotation (conditional)
    // ASM: apply if cur_iter==AngleIter OR cur_iter>=A_AfterIter (no ">0" guard!)
    // When A_AfterIter=0, rotation applies to ALL iterations (every iter >= 0).
    if (rotate) {
        double cos_a = cos(yz_angle);
        double sin_a = sin(yz_angle);
        double nz = z * sin_a + y * cos_a;
        double ny = y * sin_a - z * cos_a;
        y = ny;
        z = nz;
    }

    // Polygon transform (conditional)
    // ASM: apply if cur_iter==PolyIter OR cur_iter>=PolyIterAfter (no ">0" guard!)
    if (polygon) {
        double half_angle = pi_const / Sides;
        double double_half = 2.0 * half_angle;
        double theta = abs(atan2(z, y));
        // Reduce theta modulo double_half until <= half_angle
        while (theta >= half_angle) {
            theta -= double_half;
        }
        double factor = cos(theta);
        y *= factor;
        z *= factor;
    }

    // Benesi Pine Tree
    double xt = x * x;
    double yt = y * y;
    double zt = z * z;
    double t = 2.0 * x / sqrt(yt + zt);

    // Per ASM trace (trust ASM over description):
    //   x_out = xt - yt - zt + Cx * pixel_x
    //   y_out = 2*t*y*z + Cy * pixel_z
    //   z_out = t*(yt-zt) + Cz * pixel_y
    double out_x = xt - yt - zt + Cx * pixel_x;
    double out_y = 2.0 * t * y * z + Cy * pixel_z;
    double out_z = t * (yt - zt) + Cz * pixel_y;

    return cpu_double3(out_x, out_y, out_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_BT1Pinegon = {
    .id = FORMULA_BT1PINEGON,
    .name = "BT1Pinegon",
    .param_count = 16,
    .complexity = 430,
    .uses_transcendental = true
};

/*
Original M3 Description:
Transform 1 with Pine Tree Mandelbulb. T1_Count is the number of times you do T1
before the PTM, if you want to increase the amount of details.

AngleIter is the iteration number that the fractal is rotated on -- IN RADIANS!
A_AfterIter is the iteration number that every iteration has a rotation applied after.
The rotation is just the yz plane (around the x axis).

PolyIter and PolyIterAfter add polygonal transforms on specific iterations.

Parameters:
  [0]  Scale: double (default: 2.0)
  [1]  xOff: double (default: 2.0)
  [2]  pixel_x: double (default: 1.0)
  [3]  pixel_y: double (default: 0.0)
  [4]  pixel_z: double (default: 0.0)
  [5]  yz_angle: double (default: 0.0, radians)
  [6]  AngleIter: integer (default: 62)
  [7]  A_AfterIter: integer (default: 0)
  [8]  T1_Count: integer (default: 1)
  [9]  PolyIter: integer (default: 2)
  [10] PolyIterAfter: integer (default: 62)
  [11] Sides: double (default: 4.0)
  [12] Cx, [13] Cy, [14] Cz: from engine context
  [15] cur_iter: current iteration number

C handling: C is multiplied by pixel_x/y/z and added inline per ASM.
Engine should NOT add C again.

NOTE: ASM uses Cy*pixel_z for y-output and Cz*pixel_y for z-output,
which differs from the description. Trust ASM.

M Benesi
*/
