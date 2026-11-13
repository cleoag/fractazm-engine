// Riemann2.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Riemann2
// Hand-ported from ASM: 371 bytes, DEoption=0 (standard fractal)
// Uses raw .3SingleAngles Euler angles via buildRotMatrix3().

constexpr unsigned int FORMULA_RIEMANN2 = 10263;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Riemann2_x87(double* state, const double* params);
#endif

// ASM trace summary:
//
// Parameters (double):
//   [0..2]  Rotation angles (.3SingleAngles, default: 0,0,0)
//   [3]     Scale       (default: 1)
//   [4]     RPow        (default: 2)
//   [5]     r-Shift     (default: -0.25)
//   [6]     s-Shift     (default: 0)
//   [7]     t-Shift     (default: 0)
//   [8]     x-Shift     (default: 1)
//   [9]     Sin-AddFold (default: 0)
//   [10]    Smallest    (default: 1e-5)
//
// Constants: [PVar+0x00] = pi, [PVar+0x08] = 2.0
//
// Algorithm from ASM:
//   1. Rotate (x,y,z) by forward matrix
//   2. r = sqrt(x^2+y^2+z^2); store r
//   3. scale_factor = Scale / r; x*=sf; y*=sf; z*=sf
//   4. denom = 1 - x
//      if |denom| >= smallest: s = z/denom, t = y/denom
//      else: s = z, t = y (discontinuity)
//   5. w_power = 1 + s^2 + t^2    (for exponent, uses raw s,t)
//   6. s = |abs(sin(pi*s + s_shift)) + sin_addfold|
//      t = |abs(sin(pi*t + t_shift)) + sin_addfold|
//   7. r_new = r_shift + pow(r, RPow * w_power)
//   8. w_new = 2 / (1 + s^2 + t^2)  (uses folded s,t)
//   9. x = r_new * (1 - w_new) + x_shift
//      y = r_new * t * w_new
//      z = r_new * s * w_new
//
// Original stores xyz without a Julia-C addition.

template<>
cpu_double3 execute_formula<FORMULA_RIEMANN2>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z};
    Riemann2_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#else
    RotMatrix3 Rot = rotMatrix3FromFloats(params, 0);
    const double R00 = Rot.col0.x, R01 = Rot.col1.x, R02 = Rot.col2.x;
    const double R10 = Rot.col0.y, R11 = Rot.col1.y, R12 = Rot.col2.y;
    const double R20 = Rot.col0.z, R21 = Rot.col1.z, R22 = Rot.col2.z;

    const double Scale       = params[9];   // default: 1
    const double RPow        = params[10];  // default: 2
    const double r_shift     = params[11];  // default: -0.25
    const double s_shift     = params[12];  // default: 0
    const double t_shift     = params[13];  // default: 0
    const double x_shift     = params[14];  // default: 1
    const double sin_addfold = params[15];  // default: 0
    const double smallest    = params[16];  // default: 1e-5

    double x = p.x, y = p.y, z = p.z;

    // Step 1: Forward rotation
    double rx = x * R00 + y * R01 + z * R02;
    double ry = x * R10 + y * R11 + z * R12;
    double rz = x * R20 + y * R21 + z * R22;
    x = rx; y = ry; z = rz;

    // Step 2: r = sqrt(x^2 + y^2 + z^2)
    double r = sqrt(x * x + y * y + z * z);

    // Step 3: Scale by Scale/r
    double sf = Scale / r;
    x *= sf; y *= sf; z *= sf;

    // Step 4: Riemann stereographic projection
    double denom = 1.0 - x;
    double s_raw, t_raw;
    if (denom > smallest) {
        double inv_denom = 1.0 / denom;
        s_raw = z * inv_denom;
        t_raw = y * inv_denom;
    } else {
        s_raw = z;
        t_raw = y;
    }

    // Step 5: w for power exponent (pre-fold)
    double w_power = 1.0 + s_raw * s_raw + t_raw * t_raw;

    // Step 6: Sin folding
    double s_fold = abs(abs(sin(params[17] * s_raw + s_shift)) + sin_addfold);
    double t_fold = abs(abs(sin(params[17] * t_raw + t_shift)) + sin_addfold);

    // Step 7: r_new = r_shift + pow(r, RPow * w_power)
    double r_new = r_shift + x87_log_power(abs(r), RPow * w_power);

    // Step 8: w_new = 2 / (1 + s_fold^2 + t_fold^2)
    double w_new = params[18] / (1.0 + s_fold * s_fold + t_fold * t_fold);

    // Step 9: Assemble output
    double out_x = r_new * (1.0 - w_new) + x_shift;
    double out_y = r_new * t_fold * w_new;
    double out_z = r_new * s_fold * w_new;

    return cpu_double3(out_x, out_y, out_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_Riemann2 = {
    .id = FORMULA_RIEMANN2,
    .name = "Riemann2",
    .param_count = 11,
    .complexity = 371,
    .uses_transcendental = true
};

/*
Original M3 Description:
Generalized from a msltoe formula.
http://www.fractalforums.com/3d-fractal-generation/riemann-fractals/msg33500/#msg33500
Use bailout=1.5 and miniter=2 to fix some weird things. This formula
is very complicated, so renders may be slow.

Algorithm:
    (rotate xyz; can give cool effects, but also strange things)
    r = sqrt(x*x+y*y+z*z);
    r' = scale/r;     // ASM confirms Scale/r, NOT Scale/sqrt(r) as description implies
    x=x*r'; z=z*r'; y=y*r';
    if (1-x) > smallest {
        s = z/(1-x);
        t = y/(1-x);
    } else {
        s = z; t = y;
    }

    w = 1+s*s+t*t;

    s = fabs(fabs(sin(pi*s+ssh)+sinaf));
    t = fabs(fabs(sin(pi*t+tsh)+sinaf));

    r = rsh+pow(r,rp*w);
    w = 2/(1+s*s+t*t);
    x = r*(1-w) + xsh;
    y = r*t*w;
    z = r*s*w;

Parameters:
  [0]     RotX: double (default: 0)
  [1]     RotY: double (default: 0)
  [2]     RotZ: double (default: 0)
  [3]     Scale: double (default: 1)
  [4]     RPow: double (default: 2)
  [5]     r-Shift: double (default: -0.25)
  [6]     s-Shift: double (default: 0)
  [7]     t-Shift: double (default: 0)
  [8]     x-Shift: double (default: 1)
  [9]     Sin-AddFold: double (default: 0)
  [10]    Smallest value: double (default: 1e-5)

C handling: No C added by formula. Engine adds C.
*/
