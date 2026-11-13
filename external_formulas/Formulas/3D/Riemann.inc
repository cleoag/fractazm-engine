// Riemann.inc — Canonical double-precision formula
// This is the SOURCE OF TRUTH. Metal float version is auto-generated from this file.

// Metal Template for Formula: Riemann
// Hand-ported from ASM: 195 bytes, DEoption=0

constexpr unsigned int FORMULA_RIEMANN = 7100;

// ASM trace:
//   r = sqrt(r²)  where r² read from [esi+0x38]
//   inv_r = 1/r
//   xn = x*inv_r, yn = y*inv_r, zn = z*inv_r  (normalize)
//   inv_ym1 = 1/(yn - 1)
//   a = xn * inv_ym1, b = zn * inv_ym1  (stereographic projection)
//   angle1 = atan2(2*a, a²-1)   (= 2*atan(a), double-angle identity)
//   angle2 = atan2(2*b, b²-1)
//   Pa1 = Power * angle1, Pa2 = Power * angle2
//   C1 = (cos(Pa1)+1)/sin(Pa1)  (= cot(Pa1/2))
//   C2 = (cos(Pa2)+1)/sin(Pa2)
//   D = C1² + C2²
//   R = r^(2*Power*RadScale)   (radius power)
//   inv_dp1 = 1/(D+1)
//   new_x = 2*R*C1*inv_dp1 + Cx
//   new_y = R*(D-1)*inv_dp1 + Cy
//   new_z = 2*R*C2*inv_dp1 + Cz
//
// Riemann sphere power mapping via stereographic projection.
// C is added inside the original body before final Double stores.

template<> struct ExternalFormulaReadsRout<FORMULA_RIEMANN> { static constexpr bool value = true; };
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void RiemannOriginal_x87(double*,const double*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_RIEMANN>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z,params[FORMULA_ROUT_SLOT]};
    RiemannOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    const ExtendedScope scope;
    const E invRadius=E(1)/sqrt(E(params[FORMULA_ROUT_SLOT]));
    const E x=E(p.x)*invRadius,y=E(p.y)*invRadius,z=E(p.z)*invRadius;
    const E projection=E(1)/(y-E(1)),a=x*projection,b=z*projection;
    const E angleA=extendedAtan2(a+a,a*a-E(1))*E(params[0]);
    const E angleB=extendedAtan2(b+b,b*b-E(1))*E(params[0]);
    const auto missingPush=[](E angle) {
        return (angle.signExponent()&0x7fff)!=0x7fff && abs(angle)>=E(0x1p63);
    };
    // A finite out-of-range FSINCOS leaves the angle without pushing. The
    // following fixed stack indices then underflow and propagate NaN to XYZ.
    if(missingPush(angleA) || missingPush(angleB))
        return cpu_double3(double(extendedNaN()),double(extendedNaN()),double(extendedNaN()));
    const auto trigA=extendedSinCos(angleA),trigB=extendedSinCos(angleB);
    const E c1=trigA.sin/(trigA.cos+E(1));
    const E c2=trigB.sin/(trigB.cos+E(1));
    const E d=c1*c1+c2*c2,invD=E(1)/(d+E(1));
    // Original normalization precedes the radial power. Its FLDLN2/FYL2X,
    // exponent multiply and FLDL2E sequence consumes stored Rout directly.
    const E radius=extendedLogPower(E(params[FORMULA_ROUT_SLOT]),E(params[0])*E(params[1]));
    const E nx=(c1*invD)*radius,ny=((d-E(1))*invD)*radius,nz=(c2*invD)*radius;
    return cpu_double3(double((nx+nx)+E(params[2])),double(ny+E(params[3])),
                       double((nz+nz)+E(params[4])));
#else
    // Portable arithmetic is not certified against original x87 CODE.

    const double Power    = params[0];  // angular power (default: 8)
    // RadScale is a hidden PVar constant at [edi-0x08] = 1/SIpower = 0.5
    // Not exposed as a user-configurable option in the .m3f.
    // The engine pre-computes 1/SIpower and stores it in the PVar buffer.
    const double RadScale = params[1];

    double x = p.x, y = p.y, z = p.z;
    if (x == 0.0 && y == 0.0 && z == 0.0) {
        return cpu_double3(NAN, NAN, NAN);
    }
    if (!isfinite(x) || !isfinite(y) || !isfinite(z)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    // Compute radius and normalize
    double r2 = params[FORMULA_ROUT_SLOT];
    double r = sqrt(r2);
    if (!isfinite(r) || r == 0.0) return cpu_double3(NAN, NAN, NAN);

    double inv_r = 1.0 / r;
    double xn = x * inv_r;
    double yn = y * inv_r;
    double zn = z * inv_r;

    // Stereographic projection from unit sphere (pole at yn=1)
    double ym1 = yn - 1.0;
    if (!isfinite(ym1) || ym1 == 0.0) return cpu_double3(NAN, NAN, NAN);
    double inv_ym1 = 1.0 / ym1;

    double a = xn * inv_ym1;
    double b = zn * inv_ym1;

    // Double-angle formula: atan2(2a, a²-1) = 2*atan(a)
    double angle1 = atan2(2.0 * a, a * a - 1.0);
    double angle2 = atan2(2.0 * b, b * b - 1.0);

    // Power-multiply the angles
    double Pa1 = Power * angle1;
    double Pa2 = Power * angle2;

    // Convert back to stereographic coords: tan(Pa/2) = sin(Pa)/(cos(Pa)+1)
    // ASM uses fdivp st(1): st(1) = sin / (cos+1) = tan(Pa/2)
    double sin_Pa2 = sin(Pa2);
    double cos_Pa2 = cos(Pa2);
    double denom2 = cos_Pa2 + 1.0;
    if (!isfinite(denom2) || denom2 == 0.0) return cpu_double3(NAN, NAN, NAN);
    double C2 = sin_Pa2 / denom2;

    double sin_Pa1 = sin(Pa1);
    double cos_Pa1 = cos(Pa1);
    double denom1 = cos_Pa1 + 1.0;
    if (!isfinite(denom1) || denom1 == 0.0) return cpu_double3(NAN, NAN, NAN);
    double C1 = sin_Pa1 / denom1;

    // Inverse stereographic projection
    double D = C1 * C1 + C2 * C2;
    double inv_dp1 = 1.0 / (D + 1.0);  // D+1 >= 1, safe

    // Radius power: r^(2*Power*RadScale)
    // With RadScale=0.5: r^Power (standard)
    double R = pow(r, 2.0 * Power * RadScale);
    if (!isfinite(R)) return cpu_double3(NAN, NAN, NAN);

    // Output: inverse stereographic coords scaled by R
    double new_x = 2.0 * R * C1 * inv_dp1;
    double new_y = R * (D - 1.0) * inv_dp1;
    double new_z = 2.0 * R * C2 * inv_dp1;
    if (!isfinite(new_x) || !isfinite(new_y) || !isfinite(new_z)) {
        return cpu_double3(NAN, NAN, NAN);
    }

    return cpu_double3(new_x+params[2], new_y+params[3], new_z+params[4]);
#endif
}

// Formula metadata
constant FormulaInfo formula_info_Riemann = {
    .id = FORMULA_RIEMANN,
    .name = "Riemann",
    .param_count = 2, // Power and the loader normalization coefficient at PVar-8.
    .complexity = 195,
    .uses_transcendental = true
};

/*
Original M3 Description:
Riemann sphere power mapping formula.

Uses stereographic projection to map 3D points onto a sphere,
applies angular power multiplication, and maps back. This creates
fractal structures based on the Riemann sphere topology.

Algorithm:
1. Normalize to unit sphere: (x,y,z)/r
2. Stereographic projection: a = x/(y-1), b = z/(y-1)
3. Double-angle: angle = atan2(2a, a²-1) = 2*atan(a)
4. Power: Pa = Power * angle
5. Back to stereographic: C = cot(Pa/2) = (cos(Pa)+1)/sin(Pa)
6. Inverse stereographic + radius power:
   new = R * (2C1/(D+1), (D-1)/(D+1), 2C2/(D+1))
   where D = C1²+C2², R = r^(2*Power*RadScale)

Parameters:
  [0] Power: double (default: 8)
  [1] RadScale: double (default: 0.5, gives r^Power)
*/
