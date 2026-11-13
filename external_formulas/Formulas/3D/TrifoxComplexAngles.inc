// TrifoxComplexAngles.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: TrifoxComplexAngles
// Hand-ported from ASM: 203 bytes, DEoption=0
// Trifox variant using complex-valued angle raised to integer power.
// Spherical→complex angle→complex power→modified spherical output.

constexpr unsigned int FORMULA_TRIFOXCOMPLEXANGLES = 4225;

template<> struct ExternalFormulaReadsRout<FORMULA_TRIFOXCOMPLEXANGLES> { static constexpr bool value = true; };
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void TrifoxComplexAnglesOriginal_x87(double*, const unsigned char*);
#endif

// ASM trace (203 bytes):
//
// Phase 1: R = r^power_R via binary exponentiation
//   r = sqrt([esi+0x38])  (r² from engine)
//   R = r ^ power_R (integer exponent, binary exp loop)
//
// Phase 2: Spherical angles
//   phi = atan2(z, sqrt(x²+y²))   (elevation from XY plane)
//   theta = atan2(y, x)            (azimuthal)
//   Both scaled by Angle_scale
//
// Phase 3: Optional flip theta↔phi
//   if Flip_Theta_Phi != 0: swap angles
//
// Phase 4: Optional reciprocal on complex angle
//   if Reciprocal_R_angle != 0:
//     inv = 1/(angle_a² + angle_b² + Reciprocal_fixing)
//     angle_a *= inv; angle_b *= inv
//
// Phase 5: Complex power (angle_a + angle_b*i)^power_Angle
//   via binary exponentiation of complex number
//
// Phase 6: Scale by Angle_scale2, then spherical output:
//   a = real * Angle_scale2
//   b = imag * Angle_scale2
//   x = R * cos(a) * cos(b) + Cx
//   y = R * cos(a) * sin(b) + Cy
//   z = R * sin(a) * Z_multiplier + Cz
//
// C handling: Formula handles C internally (DEoption=0).

template<>
cpu_double3 execute_formula<FORMULA_TRIFOXCOMPLEXANGLES>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    // Zero powers never terminate in the original. Retain the bounded fallback
    // below for that unsupported input; never enter its original shift loop.
    if (int(params[0]) != 0 && int(params[4]) != 0) {
        unsigned char packed[48] = {};
        for (int i = 0; i < 8; ++i) {
            if (i == 0 || i == 3 || i == 4 || i == 5) {
                const int value = int(params[i]);
                memcpy(packed + 36 - 4*i, &value, 4);
            } else {
                const float value = float(params[i]);
                memcpy(packed + 36 - 4*i, &value, 4);
            }
        }
        double state[] = {p.x,p.y,p.z,params[FORMULA_ROUT_SLOT],params[8],params[9],params[10]};
        TrifoxComplexAnglesOriginal_x87(state, packed + 48);
        return cpu_double3(state[0],state[1],state[2]);
    }
#endif
#if defined(MB3D_SOFTWARE_EXT80)
    if(int(params[0])!=0 && int(params[4])!=0) {
        const numeric::ExtendedScope scope;
        using E=numeric::Extended80;
        // Both original shift loops use unsigned SHR, including negative
        // prepared INTEGERs. Rout is caller state, not a recomputed radius.
        E radius(1),base=sqrt(E(params[FORMULA_ROUT_SLOT]));
        uint32_t exponent=static_cast<uint32_t>(int(params[0]));
        while(exponent) {
            if(exponent&1) radius*=base;
            exponent>>=1;
            if(exponent) base*=base;
        }
        const E x(p.x),y(p.y),z(p.z),angleScale(static_cast<float>(params[1]));
        E a=numeric::extendedAtan2(y,x)*angleScale;
        E b=numeric::extendedAtan2(z,sqrt(x*x+y*y))*angleScale;
        if(int(params[3])!=0) {const E temporary=a;a=b;b=temporary;}
        if(int(params[5])!=0) {
            const E inverse=E(1)/(a*a+b*b+E(static_cast<float>(params[6])));
            a*=inverse;b*=inverse;
        }
        E real(1),imaginary(0);
        exponent=static_cast<uint32_t>(int(params[4]));
        while(exponent) {
            if(exponent&1) {
                const E nextReal=real*a-imaginary*b;
                imaginary=imaginary*a+real*b;real=nextReal;
            }
            exponent>>=1;
            if(exponent) {
                const E product=b*a;
                a=a*a-b*b;b=product+product;
            }
        }
        const E outputScale(static_cast<float>(params[2]));
        real*=outputScale;imaginary*=outputScale;
        // An out-of-range FSINCOS does not push. The subsequent fixed stack
        // accesses in this formula then underflow in all three output lanes.
        if(abs(real)>=E(0x1p63) || abs(imaginary)>=E(0x1p63))
            return cpu_double3(static_cast<double>(numeric::extendedNaN()));
        const auto first=numeric::extendedSinCos(real),second=numeric::extendedSinCos(imaginary);
        return cpu_double3(static_cast<double>(second.cos*first.cos*radius+E(params[8])),
                           static_cast<double>((first.cos*second.sin)*radius+E(params[9])),
                           static_cast<double>(first.sin*radius*E(static_cast<float>(params[7]))+E(params[10])));
    }
#endif
    // Portable/zero-power fallback is not certified against original CODE.
    const int   power_R            = int(params[0]);   // [edi-0x0c] (default: 2)
    const double Angle_scale        = params[1];         // [edi-0x10] (default: 0.75)
    const double Angle_scale2       = params[2];         // [edi-0x14] (default: 1.0)
    const int   Flip_Theta_Phi     = int(params[3]);    // [edi-0x18] (default: 0)
    const int   power_Angle        = int(params[4]);    // [edi-0x1c] (default: 2)
    const int   Reciprocal_R_angle = int(params[5]);    // [edi-0x20] (default: 0)
    const double Reciprocal_fixing  = params[6];          // [edi-0x24] (default: 0.1)
    const double Z_multiplier       = params[7];          // [edi-0x28] (default: 1.0)
    const double Cx                 = params[8];
    const double Cy                 = params[9];
    const double Cz                 = params[10];

    double x = p.x, y = p.y, z = p.z;

    // R = r ^ power_R
    double r_sq = x * x + y * y + z * z;
    double r = sqrt(r_sq);
    double R = pow(r, double(power_R));

    // Spherical angles
    double phi   = atan2(z, sqrt(x * x + y * y));  // elevation
    double theta = atan2(y, x);                      // azimuthal

    // Scale angles
    phi   *= Angle_scale;
    theta *= Angle_scale;

    // Optionally flip theta and phi
    double angle_a = (Flip_Theta_Phi != 0) ? phi   : theta;
    double angle_b = (Flip_Theta_Phi != 0) ? theta : phi;

    // Optional reciprocal transform on complex angle
    if (Reciprocal_R_angle != 0) {
        double inv = 1.0 / (angle_a * angle_a + angle_b * angle_b + Reciprocal_fixing);
        angle_a *= inv;
        angle_b *= inv;
    }

    // Complex power: (angle_a + angle_b*i) ^ power_Angle
    // Using polar form: c = m*e^(i*arg), c^n = m^n * e^(i*n*arg)
    double m   = sqrt(angle_a * angle_a + angle_b * angle_b);
    double arg = atan2(angle_b, angle_a);
    double n   = double(power_Angle);
    double mn  = pow(m, n);
    double final_re = mn * cos(n * arg);
    double final_im = mn * sin(n * arg);

    // Scale by Angle_scale2
    double a = final_re * Angle_scale2;
    double b = final_im * Angle_scale2;

    // Output in modified spherical coordinates
    double cos_a = cos(a);
    double sin_a = sin(a);
    double cos_b = cos(b);
    double sin_b = sin(b);

    double x_out = R * cos_a * cos_b + Cx;
    double y_out = R * cos_a * sin_b + Cy;
    double z_out = R * sin_a * Z_multiplier + Cz;

    return cpu_double3(x_out, y_out, z_out);
}

// Formula metadata
constant FormulaInfo formula_info_TrifoxComplexAngles = {
    .id = FORMULA_TRIFOXCOMPLEXANGLES,
    .name = "TrifoxComplexAngles",
    .param_count = 11,
    .complexity = 203,
    .uses_transcendental = true
};

/*
Original M3 Description:
Trifox variant using complex-valued angle raised to integer power.
Converts to spherical, forms a complex number from the angles,
raises it to an integer power, then converts back.

Mathematical Formula:
  R = r ^ power_R
  phi = atan2(z, sqrt(x²+y²))
  theta = atan2(y, x)
  phi *= Angle_scale; theta *= Angle_scale
  if Flip: swap(theta, phi)

  c = angle_a + angle_b*i
  if Reciprocal_R_angle:
    c *= 1/(|c|² + fix)
  c = c ^ power_Angle
  c *= Angle_scale2

  a = Re(c), b = Im(c)
  x = R * cos(a) * cos(b) + Cx
  y = R * cos(a) * sin(b) + Cy
  z = R * sin(a) * Z_multiplier + Cz

C handling: Standard addition of Ci per axis.
Engine should NOT add C; template handles it.

Parameters:
  [0] power_R: int (default: 2) - radial power exponent
  [1] Angle_scale: double (default: 0.75) - pre-power angle scaling
  [2] Angle_scale2: double (default: 1.0) - post-power angle scaling
  [3] Flip_Theta_Phi: int (default: 0) - swap theta and phi
  [4] power_Angle: int (default: 2) - complex angle power
  [5] Reciprocal_R_angle: int (default: 0) - enable reciprocal transform
  [6] Reciprocal_fixing: double (default: 0.1) - reciprocal stabilizer
  [7] Z_multiplier: double (default: 1.0) - z output scaling
  [8..10] Cx, Cy, Cz: passed by engine from context
*/
