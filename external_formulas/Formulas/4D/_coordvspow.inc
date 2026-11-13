// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_COORDVSPOW = 10336;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CoordVsPowOriginal_x87(double*, const double*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_COORDVSPOW>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[12]={p.x,p.y,p.z};
    const int weights[]={int(params[0]),int(params[2]),int(params[3]),int(params[4]),int(params[5]),int(params[6])};
    CoordVsPowOriginal_x87(state,params,weights);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E x=p.x,y=p.y,z=p.z,scale=params[7];
    const int powerPct=int(params[0]),cylPct=int(params[2]),sphPct=int(params[3]);
    const int icylPct=int(params[4]),isphPct=int(params[5]),expPct=int(params[6]);
    const E power=params[1];
    const auto outsideTrigRange=[](E v) {
        const unsigned exponent=v.signExponent()&0x7fff;
        return exponent>=16383+63 && exponent!=0x7fff;
    };
    // FSIN/FCOS retain their finite input at |v| >= 2^63. FSINCOS
    // additionally omits its push; the two coupled sequences below retain
    // the resulting lane/empty-slot behavior, not merely a NaN sin value.
    const auto scalarSin=[&](E v) {
        return outsideTrigRange(v) ? v : extendedSinCos(v).sin;
    };
    const auto scalarCos=[&](E v) {
        return outsideTrigRange(v) ? v : extendedSinCos(v).cos;
    };
    const auto accumulate=[&](E value,int weight,E previous) {
        return E(double((value*E(weight))*scale+previous));
    };
    E rx(0),ry(0),rz(0);
    if(powerPct!=0) {
        if(power==E(1) || power==E(0)) {
            rx=double((x*E(powerPct))*scale);
            ry=double((y*E(powerPct))*scale);
            rz=double((z*E(powerPct))*scale);
        } else {
            const E radius=sqrt(x*x+(y*y+z*z));
            const E savedRadius=double(radius); // FST at 006E; FYL2X keeps radius.
            const E radiusPower=double(extendedExp2(power*extendedLog2(abs(radius)),false));
            const E theta=extendedAtan2(y,x)*power;
            const E zr=z/savedRadius;
            const E polarNumerator=sqrt(-(zr*zr)+E(1))+E(1);
            // Retain the extended reduction quotient before the wider atan
            // evaluation. A second rounded quadrant subtraction changes one
            // angle ulp that negative powers can amplify near a cosine zero.
            const E halfPhi=extendedAtan2Wide(E(1),zr/polarNumerator);
            const E phi=(halfPhi+halfPhi)*power;
            const auto polar=extendedSinCos(phi),azimuth=extendedSinCos(theta);
            const bool missingPolarPush=outsideTrigRange(phi);
            const E pc=double(polar.cos);
            const E ps=double(missingPolarPush ? theta : polar.sin);
            const E ac=double(missingPolarPush ? extendedNaN() : azimuth.cos);
            const E as=double(missingPolarPush ? extendedNaN() : azimuth.sin);
            rx=double((((radiusPower*ac)*pc)*E(powerPct))*scale);
            ry=double((((radiusPower*as)*pc)*E(powerPct))*scale);
            rz=double(((radiusPower*ps)*E(powerPct))*scale);
        }
    }
    if(cylPct!=0) {
        const E radius=sqrt(x*x+y*y);
        ry=accumulate(extendedAtan2(y,x),cylPct,ry);
        rx=accumulate(radius,cylPct,rx);
    }
    if(sphPct!=0) {
        const E square=x*x+y*y;
        const E radius=sqrt(z*z+square);
        const E polar=extendedAtan2(sqrt(square),z);
        // 0178 forms the polar angle; 017E forms azimuth. They accumulate
        // into Y and Z respectively, unlike the earlier portable mapping.
        rz=accumulate(extendedAtan2(y,x),sphPct,rz);
        ry=accumulate(polar,sphPct,ry);
        rx=accumulate(radius,sphPct,rx);
    }
    if(icylPct!=0) {
        const E cosine=x*scalarCos(y),sine=x*scalarSin(y);
        ry=accumulate(sine,icylPct,ry);
        rx=accumulate(cosine,icylPct,rx);
    }
    if(isphPct!=0) {
        const auto polar=extendedSinCos(y),azimuth=extendedSinCos(z);
        const bool missingY=outsideTrigRange(y),missingZ=outsideTrigRange(z);
        const E zLane=missingY ? (missingZ ? extendedNaN() : y)
                              : (missingZ ? polar.sin : polar.cos);
        rz=accumulate(zLane*x,isphPct,rz);
        if(missingY || missingZ) {
            // After the first store, FXCH ST(2) reaches an empty slot.
            ry=accumulate(extendedNaN(),isphPct,ry);
            rx=accumulate(extendedNaN(),isphPct,rx);
        } else {
            const E xLane=(azimuth.cos*polar.sin)*x;
            const E yLane=azimuth.sin*(polar.sin*x);
            ry=accumulate(yLane,isphPct,ry);
            rx=accumulate(xLane,isphPct,rx);
        }
    }
    if(expPct!=0) {
        const E log2e=E::fromBits(0x3fff,0xb8aa3b295c17f0bcULL);
        const E radius=sqrt(x*x+y*y),ratio=z/radius;
        const E savedRatio=double(ratio); // 0235 leaves the original in ST(0).
        const E exponential=double(extendedExp2((-(ratio*y)+x)*log2e,false)*params[8]);
        const auto first=extendedSinCos(savedRatio*x+y);
        const auto second=extendedSinCos(savedRatio*x-y);
        const E c1=double(first.cos),s1=double(first.sin);
        const E c2=double(second.cos),s2=double(second.sin);
        const E product=savedRatio*y;
        const E twice=double(extendedExp2((product+product)*log2e,false));
        rx=accumulate((twice*c2+c1)*exponential,expPct,rx);
        ry=accumulate((-(twice*s2)+s1)*exponential,expPct,ry);
        rz=accumulate(extendedExp2(radius*log2e,false)*scalarSin(z),expPct,rz);
    }
    return cpu_double3(double(rx),double(ry),double(rz));
#else
    // Portable arithmetic remains uncertified.
    const int   powerPct   = int(params[0]);  // [PVar-0x0C] Power %
    const double floatPow   = params[1];       // [PVar-0x10] Float pow
    const int   cylPct     = int(params[2]);  // [PVar-0x18] Cylindric %
    const int   sphPct     = int(params[3]);  // [PVar-0x1C] Spheric %
    const int   icylPct    = int(params[4]);  // [PVar-0x20] I.Cylindric %
    const int   isphPct    = int(params[5]);  // [PVar-0x24] I.Spheric %
    const int   expPct     = int(params[6]);  // [PVar-0x28] Exp3D %
    const double scale      = 0.01;           // [PVar+0x00]
    const double half_val   = 0.5;            // [PVar+0x08]

    double x = p.x, y = p.y, z = p.z;
    double rx = 0.0, ry = 0.0, rz = 0.0;

    if (powerPct != 0) {
        if (floatPow == 1.0 || floatPow == 0.0) {
            rx = x * double(powerPct) * scale;
            ry = y * double(powerPct) * scale;
            rz = z * double(powerPct) * scale;
        } else {
            double r = sqrt(x*x + y*y + z*z);
            double r_pow = pow(fabs(r), floatPow);

            double theta = atan2(y, x) * floatPow;

            double zr = z / r;
            double phi = 2.0 * atan2(zr, sqrt(1.0 - zr*zr) + 1.0) * floatPow;

            double cos_phi, sin_phi, cos_theta, sin_theta;
            cos_phi = cos(phi);
            sin_phi = sin(phi);
            cos_theta = cos(theta);
            sin_theta = sin(theta);

            double pscale = double(powerPct) * scale;
            rx = r_pow * cos_phi * cos_theta * pscale;
            ry = r_pow * sin_phi * cos_theta * pscale;
            rz = r_pow * sin_theta * pscale;
        }
    }

    if (cylPct != 0) {
        double r_xy = sqrt(x*x + y*y);
        double angle = atan2(y, x);
        double cscale = double(cylPct) * scale;
        ry += angle * cscale;
        rx += r_xy * cscale;
    }

    if (sphPct != 0) {
        double r_xy = sqrt(x*x + y*y);
        double r_xyz = sqrt(x*x + y*y + z*z);
        double angle_xy = atan2(z, r_xy);   // elevation angle
        double angle_xz = atan2(y, x);      // azimuth angle
        double sscale = double(sphPct) * scale;
        rz += angle_xy * sscale;
        ry += angle_xz * sscale;
        rx += r_xyz * sscale;
    }

    if (icylPct != 0) {
        double iscale = double(icylPct) * scale;
        ry += x * sin(y) * iscale;
        rx += x * cos(y) * iscale;
    }

    if (isphPct != 0) {
        double iscale = double(isphPct) * scale;
        double cos_y = cos(y), sin_y = sin(y);
        double cos_z = cos(z), sin_z = sin(z);
        rz += x * cos_y * sin_z * iscale;
        ry += sin_y * cos_z * x * iscale;
        rx += cos_y * cos_z * x * iscale;
    }

    if (expPct != 0) {
        double escale = double(expPct) * scale;
        double r_xy = sqrt(x*x + y*y);
        double ratio = z / r_xy;  // stored to [ebp-0x20]

        double arg1 = -ratio * y + x;
        double exp1 = exp(arg1);
        double exp1_scaled = exp1 * half_val;  // [edi+0x08] = 0.5

        double a1 = ratio * x + y;
        double a2 = ratio * x - y;
        double cos_a1 = cos(a1), sin_a1 = sin(a1);
        double cos_a2 = cos(a2), sin_a2 = sin(a2);

        double exp2 = exp(2.0 * ratio * y);  // [ebp-0x20] stored

        rx += (exp2 * cos_a2 + cos_a1) * exp1_scaled * escale;
        ry += (-exp2 * sin_a2 + sin_a1) * exp1_scaled * escale;

        double exp_rxy = exp(r_xy);
        rz += exp_rxy * sin(z) * escale;
    }

    return cpu_double3(rx, ry, rz);
#endif
}

constant FormulaInfo formula_info_coordvspow = {
    .id = FORMULA_COORDVSPOW,
    .name = "_coordvspow",
    .param_count = 7,
    .complexity = 790,
    .uses_transcendental = true
};

