// Ruckerbulb.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Ruckerbulb
// Hand-ported from ASM: 287 bytes, DEoption=0
// Rudy Rucker's 3D Mandelbrot formula (1991) - generalized with arbitrary power
// Uses atan-based angle computation (first "Mandelbulb" formula ever)

constexpr unsigned int FORMULA_RUCKERBULB = 5516;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void Ruckerbulb_x87(double* state, const double* params);
#endif

// ASM trace:
//   r = sqrt(x² + y² + z²)
//   r_pow = r^Power (via fyl2x/f2xm1/fscale pattern)
//   Mode == 0: r_z = r_pow (same radius power for z)
//   Mode != 0: r_z = r^z_AnglePow (separate power for z component)
//
//   zangle_raw = atan2(-x, z)   [fpatan with ST(0)=z, ST(1)=-x]
//   Atan2Mode == 0 corrections (convert atan2 to atan):
//     if (x >= 0): no correction
//     if (x < 0):  zangle_raw -= pi * AngleScale
//     Note: original ASM has dead branch for z>0 case (JL after AND ah,0x41
//     never triggers), so the correction always subtracts pi regardless of z sign.
//   zangle = zangle_raw * z_AnglePow
//
//   theta = atan2(y, x) * Power
//
//   x_out = -r_pow * cos(zangle) * cos(theta) + Cx
//   y_out = -r_pow * cos(zangle) * sin(theta) + Cy
//   z_out = r_z * sin(zangle) * z_Mul + Cz
//
// C handling: DEoption=0, formula reads and adds C internally.

template<>
cpu_double3 execute_formula<FORMULA_RUCKERBULB>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z};
    Ruckerbulb_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    const ExtendedScope scope;
    const E x(p.x),y(p.y),z(p.z);
    const E radius=sqrt(x*x+(y*y+z*z));
    // CODE 0028 spills the radius without popping it. The first power still
    // consumes the extended radius; the optional second power reloads Double.
    const E radiusPower=double(extendedExp2(E(params[0])*extendedLog2(abs(radius)),false));
    const E radiusZ=int(params[3])==0 ? radiusPower :
        E(double(extendedExp2(E(params[1])*extendedLog2(abs(E(double(radius)))),false)));
    E polar=extendedAtan2(-x,z);
    // AND AH,41h clears SF/OF: the z-dependent JL branch is unreachable.
    if(int(params[4])==0 && x<E(0)) polar-=x87Pi()*E(params[5]);
    const auto za=extendedSinCos(polar*E(params[1]));
    const auto xy=extendedSinCos(extendedAtan2(y,x)*E(params[0]));
    // Both FSINCOS pairs are explicitly stored as Double before products.
    const E cosZ=double(za.cos),sinZ=double(za.sin);
    const E cosXY=double(xy.cos),sinXY=double(xy.sin);
    return cpu_double3(double(-(radiusPower*cosZ*cosXY)+E(params[6])),
                       double(-(radiusPower*cosZ*sinXY)+E(params[7])),
                       double(radiusZ*sinZ*E(params[2])+E(params[8])));
#else
    const double Power      = params[0];  // [PVar-0x10] (default: 2)
    const double z_AnglePow = params[1];  // [PVar-0x18] (default: 2)
    const double z_Mul      = params[2];  // [PVar-0x20] (default: -1)
    const double Mode       = params[3];  // [PVar-0x24] int (default: 0)
    const double Atan2Mode  = params[4];  // [PVar-0x28] int (default: 0)
    // [CONSTANTS] embedded as compile-time values:
    const double AngleScale = params[5];  // [PVar+0x00]
    // C values passed by engine from context
    const double Cx         = params[6];  // [esi+0x18]
    const double Cy         = params[7];  // [esi+0x20]
    const double Cz         = params[8];  // [esi+0x28]

    double x = p.x, y = p.y, z = p.z;

    // r = sqrt(x² + y² + z²)
    double r = sqrt(x * x + y * y + z * z);

    // r_pow = r^Power (power via fyl2x/f2xm1/fscale = pow(|r|, Power))
    double r_pow = x87_log_power(fabs(r), Power);

    // r_z depends on Mode
    double r_z;
    if (Mode == 0.0) {
        r_z = r_pow;
    } else {
        // Mode != 0: separate power for z radius
        r_z = x87_log_power(fabs(r), z_AnglePow);
    }

    // zangle_raw = atan2(-x, z)
    double zangle_raw = atan2(-x, z);

    // Atan2Mode == 0: the executable's z<0 add-pi branch is dead after
    // `and ah, 0x41`, so the only live correction is subtracting pi when x<0.
    if (Atan2Mode == 0.0 && x < 0.0) {
        zangle_raw -= M_PI * AngleScale;
    }

    // Multiply angle by z_AnglePow
    double zangle = zangle_raw * z_AnglePow;

    // theta = atan2(y, x) * Power
    double theta = atan2(y, x) * Power;

    // Compute trig values
    double cos_za = cos(zangle);
    double sin_za = sin(zangle);
    double cos_th = cos(theta);
    double sin_th = sin(theta);

    // Output with C addition
    double new_x = -r_pow * cos_za * cos_th + Cx;
    double new_y = -r_pow * cos_za * sin_th + Cy;
    double new_z = r_z * sin_za * z_Mul + Cz;

    return cpu_double3(new_x, new_y, new_z);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_Ruckerbulb = {
    .id = FORMULA_RUCKERBULB,
    .name = "Ruckerbulb",
    .param_count = 5,
    .complexity = 287,
    .uses_transcendental = true
};

/*
Original M3 Description:
The Rudy Rucker's formula for 3D Mandelbrot.
Was invented in 1991, the first "Mandelbulb" formula ever; used "atan" instead
of "asin" and a different kind of angle for z. Results are nice.

Complete mode 0 formula:
  r = sqrt(x*x+y*y+z*z) ^ Power
  theta  = atan2(y,x) * Power
  zangle = atan(z/(-x)) * z_AnglePow

  x := -cos(zangle)*cos(theta)*r + Cx
  y := -cos(zangle)*sin(theta)*r + Cy
  z :=  sin(zangle)*r*z_Mul + Cz

MODE 0 (recommended): standard, one radius power, separate angle multipliers.
MODE 2 (any nonzero): separate r_z = r^z_AnglePow for z component.
ATAN2MODE 0: uses atan with corrections (default).
ATAN2MODE != 0: uses atan2 directly.

Negative z mode (z_Mul=-1) is activated by default.
Formula by Luca G.N. 2011.

Parameters:
  [0] Power: double (default: 2) - main radius/angle exponent
  [1] z_AnglePow: double (default: 2) - z angle exponent
  [2] z_Mul: double (default: -1) - z multiplier
  [3] Mode: int (default: 0) - 0=shared r, nonzero=separate r_z
  [4] Atan2Mode: int (default: 0) - 0=atan+correction, nonzero=atan2
  [5] AngleScale: constant (value: 1) - pi multiplier for atan correction
  [6] Cx: passed by engine from context
  [7] Cy: passed by engine from context
  [8] Cz: passed by engine from context
*/
