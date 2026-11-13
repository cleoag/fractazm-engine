// Original x87 arithmetic, Double stores and conditional Julia addition.
#include "tglad_original_state.h"
constexpr unsigned int FORMULA_TGLADTETRA = 10269;

template<>
cpu_double3 execute_formula<FORMULA_TGLADTETRA>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    return execute_tglad_original(p, params, TgladTetraOriginal_x87);
#elif defined(MB3D_SOFTWARE_EXT80)
    using E=numeric::Extended80;
    const numeric::ExtendedScope scope;
    const auto store=[](E value) {return E(double(value));};
    const E two(params[7]),limit(params[8]);
    const int outerCount=int(params[0])&7,innerCount=int(params[1])&7;
    const int outerIterations=outerCount==0 ? 1 : outerCount;
    const int innerIterations=innerCount==0 ? 1 : innerCount;
    E x(p.x),y(p.y),z(p.z);
    for(int outer=0;outer<outerIterations;++outer) {
        const E xySquare=x*x+y*y,square=xySquare+z*z;
        if(square>=limit) return cpu_double3(params[8],params[8],params[8]);
        const E radius=sqrt(square);
        E radialPower=store(radius);
        const E cosTheta=store(z/radius);
        const E omega=(sqrt(xySquare)-y)/x,omegaSquare=omega*omega;
        const E omegaStored=store(omega),omegaSquareStored=store(omegaSquare);
        const E invOmega=E(1)/(omegaSquare+E(1));
        const E magnitude=invOmega*sqrt((E(1)-cosTheta)/(E(1)+cosTheta));
        // FST without pop keeps the extended operand; later memory products
        // reload the separately rounded Double value.
        x=store((magnitude+magnitude)*omegaStored);
        y=store(magnitude*(E(1)-omegaSquareStored));
        for(int inner=0;inner<innerIterations;++inner) {
            const E s=x*(x*x)+((y*y)*E(params[3]))*x;
            if(s>=limit) return cpu_double3(params[8],params[8],params[8]);
            const E ss=store(s),t=y*(y*y)+((x*x)*E(params[3]))*y;
            const E b=store(t*E(params[6])),ts=store(t),tSquare=store(ts*ts);
            const E sk=ss+E(params[5]);
            const E a=store(sk*(E(params[4])-ss)-tSquare);
            const E c=store(-E(params[5])/(sk*sk+tSquare));
            const E nextX=store((a*x+-(b*y))*c);
            y=store((a*y+b*x)*c);
            x=nextX;
            radialPower=store(radialPower*radialPower);
        }
        const E mappedSquare=x*x+y*y,mappedRadius=sqrt(mappedSquare);
        const E omega2=(mappedRadius-y)/x,omega2Square=omega2*omega2;
        const E omega2Stored=store(omega2),omega2SquareStored=store(omega2Square);
        const E invOmega2=E(1)/(omega2Square+E(1)),invOmega2Stored=store(invOmega2);
        const E cosOmega=store((E(1)-omega2SquareStored)*invOmega2);
        const E sinOmega=store((omega2Stored+omega2Stored)*invOmega2Stored);
        const E angleFactor=two/(E(1)+store(mappedSquare));
        const E angleFactorStored=store(angleFactor),sinA=store(angleFactor*store(mappedRadius));
        x=store((radialPower*sinOmega)*sinA);
        y=store((radialPower*cosOmega)*sinA);
        z=store((angleFactorStored-E(1))*radialPower);
    }
    // Overflow exits bypass Julia C; the normal exit reloads stored XYZ.
    return cpu_double3(double(x+E(params[9])),double(y+E(params[10])),double(z+E(params[11])));
#else
    // Portable numerical parity remains unverified.

    int Log2Power = (int)params[0];
    int Log2MapPower = (int)params[1];

    const double HALF = params[2];
    const double NEG3 = params[3];
    const double SQRT8 = params[4];
    const double INV_SQRT8 = params[5];
    const double SQRT8_PLUS_INV = params[6];
    const double TWO = params[7];
    const double OVERFLOW = params[8];

    int outerCount = Log2Power & 7;
    int innerCount = Log2MapPower & 7;
    int outerIters = (outerCount == 0) ? 1 : outerCount;
    int innerIters = (innerCount == 0) ? 1 : innerCount;

    double x = p.x;
    double y = p.y;
    double z = p.z;

    for (int oi = 0; oi < outerIters; oi++) {
        double r_xy2 = x * x + y * y;
        double r2 = r_xy2 + z * z;

        if (r2 >= OVERFLOW) {
            return cpu_double3(OVERFLOW, OVERFLOW, OVERFLOW);
        }

        double r_xy = sqrt(r_xy2);
        double r = sqrt(r2);

        double cos_theta = z / r;
        double omega_ratio = (r_xy - y) / x;

        double omega_sq = omega_ratio * omega_ratio;
        double inv_omega_sum = 1.0 / (omega_sq + 1.0);
        double halfTan = sqrt((1.0 - cos_theta) / (1.0 + cos_theta));
        double zMag = halfTan * inv_omega_sum;

        double x_z = 2.0 * zMag * omega_ratio;
        double y_z = zMag * (1.0 - omega_sq);

        double r_sq = r;

        for (int ii = 0; ii < innerIters; ii++) {
            double xx = x_z;
            double yy = y_z;

            double S = xx * xx * xx + NEG3 * xx * yy * yy;
            if (S >= OVERFLOW) {
                return cpu_double3(OVERFLOW, OVERFLOW, OVERFLOW);
            }

            double T = yy * yy * yy + NEG3 * yy * xx * xx;

            double B = SQRT8_PLUS_INV * T;

            double T_sq = T * T;

            double A = (S + INV_SQRT8) * (SQRT8 - S) - T_sq;

            double Sk = S + INV_SQRT8;
            double C = -INV_SQRT8 / (T_sq + Sk * Sk);

            double new_x_z = (A * xx - B * yy) * C;
            double new_y_z = (A * yy + B * xx) * C;

            y_z = new_y_z;
            x_z = new_x_z;

            r_sq = r_sq * r_sq;
        }

        double xy2 = x_z * x_z + y_z * y_z;
        double zMag2 = sqrt(xy2);

        double omega2_ratio = (zMag2 - y_z) / x_z;
        double omega2_sq = omega2_ratio * omega2_ratio;
        double inv_omega2_sum = 1.0 / (omega2_sq + 1.0);

        double angle_factor = TWO / (1.0 + xy2);
        double sinA = angle_factor * zMag2;

        double sinOmega = 2.0 * omega2_ratio * inv_omega2_sum;
        double cosOmega = (1.0 - omega2_sq) * inv_omega2_sum;

        x = r_sq * sinOmega * sinA;
        y = r_sq * cosOmega * sinA;
        z = (angle_factor - 1.0) * r_sq;
    }

    return cpu_double3(x + params[9], y + params[10], z + params[11]);
#endif
}

constant FormulaInfo formula_info_tgladtetra = {
    .id = FORMULA_TGLADTETRA,
    .name = "TgladTetra",
    .param_count = 2,
    .complexity = 0,
    .uses_transcendental = false
};
