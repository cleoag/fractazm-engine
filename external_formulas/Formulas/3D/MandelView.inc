// MandelView.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: MandelView
// Hand-ported from ASM: 204 bytes, DEoption=0

constexpr unsigned int FORMULA_MANDELVIEW = 7098;

// ASM trace:
//   Three angle pairs, each Power-scaled:
//     phi1 = Power * atan2(y, x)   → cos_p1, sin_p1
//     phi2 = Power * atan2(z, y)   → cos_p2, sin_p2
//     phi3 = Power * atan2(z, x)   → cos_p3, sin_p3
//   r_P = |r|^Power   where r = sqrt(x²+y²+z²)
//   Output:
//     new_x = r_P * (cos_p1*cos_p3 - sin_p1*sin_p2*sin_p3) + Cx
//     new_y = r_P * sin_p1*cos_p2 + Cy
//     new_z = r_P * (sin_p1*sin_p2*cos_p3 + cos_p1*sin_p3) * zMul + Cz
//
// C is added inside the original body before final Double stores.
// (z output has extra zMul multiplication)

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void MandelViewOriginal_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_MANDELVIEW>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[11] = {p.x,p.y,p.z};
    MandelViewOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E=numeric::Extended80;
    const E x(p.x),y(p.y),z(p.z),power(params[0]);
    auto pair=[&](E a,E b) {return numeric::extendedSinCos(numeric::extendedAtan2(a,b)*power);};
    const auto first=pair(y,x),second=pair(z,y),third=pair(z,x);
    // All six trig lanes and the radial power have original Double stores.
    const E c1(static_cast<double>(first.cos)),s1(static_cast<double>(first.sin));
    const E c2(static_cast<double>(second.cos)),s2(static_cast<double>(second.sin));
    const E c3(static_cast<double>(third.cos)),s3(static_cast<double>(third.sin));
    const E radius(static_cast<double>(numeric::extendedExp2(power*numeric::extendedLog2(sqrt(z*z+y*y+x*x)),false)));
    return cpu_double3(static_cast<double>((c1*c3-(s2*s3)*s1)*radius+E(params[2])),
                       static_cast<double>(s1*c2*radius+E(params[3])),
                       static_cast<double>(((s2*c3)*s1+c1*s3)*radius*E(params[1])+E(params[4])));
#else
    // Portable arithmetic is not certified against original x87 CODE.

    const double Power = params[0];  // exponent (default: 8)
    const double zMul  = params[1];  // z multiplier (default: -1)

    double x = p.x, y = p.y, z = p.z;
    if (x == 0.0 && y == 0.0 && z == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }
    if (!isfinite(x) || !isfinite(y) || !isfinite(z)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    // Three Power-scaled angle pairs
    double phi1 = Power * atan2(y, x);
    double cos_p1 = cos(phi1);
    double sin_p1 = sin(phi1);

    double phi2 = Power * atan2(z, y);
    double cos_p2 = cos(phi2);
    double sin_p2 = sin(phi2);

    double phi3 = Power * atan2(z, x);
    double cos_p3 = cos(phi3);
    double sin_p3 = sin(phi3);

    // Radius power
    double r = sqrt(x * x + y * y + z * z);
    double r_P = pow(abs(r), Power);
    if (!isfinite(r) || !isfinite(r_P)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    // Output: matrix-like combination of sin/cos products
    double new_x = r_P * (cos_p1 * cos_p3 - sin_p1 * sin_p2 * sin_p3);
    double new_y = r_P * sin_p1 * cos_p2;
    double new_z = r_P * (sin_p1 * sin_p2 * cos_p3 + cos_p1 * sin_p3) * zMul;
    if (!isfinite(new_x) || !isfinite(new_y) || !isfinite(new_z)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    return cpu_double3(new_x+params[2], new_y+params[3], new_z+params[4]);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_MandelView = {
    .id = FORMULA_MANDELVIEW,
    .name = "MandelView",
    .param_count = 2,
    .complexity = 204,
    .uses_transcendental = true
};

/*
Original M3 Description:
A complicated Mandelbulb-like formula that uses three "atan2() angles",
and does a particular multiplication of them.
Was proposed by David Makin on nov. 2009.
I added a z-Mul for your convenience.

See http://www.fractalforums.com/index.php?topic=2035.msg9238#msg9238

Implemented by Luca G.N. 2011

The three angle pairs are (y,x), (z,y), (z,x) — note the different
order from Makin-NSR-tri which uses (y,x), (z,y), (x,z).
The output is a specific matrix-like combination of the 6 trig values.

Parameters:
  [0] Power: double (default: 8)
  [1] z_Mul: double (default: -1)
*/
