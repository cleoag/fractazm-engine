// Original M3F arithmetic; portable fallback remains uncertified.
constexpr unsigned int FORMULA_DUCKSIFS = 10209;

#include "ifs_original_state.h"

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void DucksEvenOriginal_x87(double*, const double*, const float*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_DUCKSIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    if ((int(params[8])&1)==0) {
        const int apply=int(params[6]);
        const cpu_double3 transformed(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
        if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[1];
        double state[]={transformed.x,transformed.y,transformed.z,0.,params[DIFS_ROLD_SLOT],params[MAX_FORMULA_PARAMS-1],0.};
        const float coefficients[]={float(params[11]),float(params[12])};
        const int flags[]={apply,int(params[7])};
        DucksEvenOriginal_x87(state,params,coefficients,flags);
        params[MAX_FORMULA_PARAMS-1]=state[5];
        de=state[6];
        return apply!=0 ? transformed : p;
    }
    // Odd Flip XZ retains the approximation until original caller state is known.
#elif defined(MB3D_SOFTWARE_EXT80)
    if ((int(params[8])&1)==0) {
        Arm64ApplyIFSState state(p,params);
        using E=Arm64ApplyIFSState::E;
        const E x(state.transformed.x), y(state.transformed.y), z(state.transformed.z);
        const E threshold=(abs(E(params[5]))+E(params[16]))*E(params[15])
            + E(params[DIFS_ROLD_SLOT]);
        // FCOMPP compares threshold with |z|; unordered also takes the exit.
        if (!(threshold>=abs(z))) return state.finish(-z,de);
        auto logarithm=[](E v) {
            const E infinity(std::numeric_limits<double>::infinity());
            if (v==E(0)) return -infinity;
            if (v==infinity) return infinity;
            return numeric::extendedLn2()*numeric::extendedLog2(v);
        };
        E duckP=x, duckQ=y, accum(0);
        const E sine(static_cast<float>(params[12]));
        const E cosine(static_cast<float>(params[11]));
        const E heightDecay(params[0]), seedX(params[9]), seedY(params[10]);
        const E logScale(params[14]);
        E radiusSquared=duckQ*duckQ+duckP*duckP;
        const int32_t iters=static_cast<int32_t>(params[7]);
        int64_t remaining=iters;
        FormulaWorkBudget work("ducksIFS");
        do {
            work.beforeStep();
            const E angle=logarithm(radiusSquared)*logScale+seedX;
            const E angle2=numeric::extendedAtan2(abs(duckQ),duckP)+seedY;
            const E nextP=angle*sine+(-(angle2*cosine));
            const E nextQ=angle*cosine+angle2*sine;
            duckP=nextP; duckQ=nextQ;
            // The next body's logarithm consumes this identical extended
            // sum, with no intervening Double store or coordinate update.
            radiusSquared=duckQ*duckQ+duckP*duckP;
            accum=accum*heightDecay+sqrt(radiusSquared);
            // DEC INT_MIN sets OF; JG is false because SF != OF. A wider
            // signed counter preserves that one-body outcome without C++ UB.
        } while (--remaining>0);
        const E result=logarithm(accum+E(1));
        E trap=result;
        if (E(0)>=E(params[13])) trap*=trap;
        params[MAX_FORMULA_PARAMS-1]=static_cast<double>(abs(trap*E(params[13])));
        return state.finish((result/E(iters))*E(params[5])-z,de);
    }
#endif

    const double PerIterHeight_mul = params[0]; // [PVar - 0x10] default 0.7
    const double Scale = params[1];             // [PVar - 0x20] .2Doubles
    const double Z_add = params[2];             // [PVar - 0x30] .2Doubles
    const double Y_add = params[3];             // [PVar - 0x38]
    const double X_add = params[4];             // [PVar - 0x40]
    const double FinalHeight_mul = params[5];   // [PVar - 0x48]
    const int Apply_scale_add = int(params[6]); // [PVar - 0x4c]
    const int Iters = int(params[7]);          // [PVar - 0x50] integer
    const int Flip_XZ = int(params[8]);        // [PVar - 0x54] integer
    const double Seed_X = params[9];            // [PVar - 0x5c] (double at -0x5c, skipping int at -0x54)
    const double Seed_Y = params[10];           // [PVar - 0x64]
    const double rot_sin = params[12];          // [PVar - 0x6c] (dword at edi-0x6c)
    const double rot_cos = params[11];          // [PVar - 0x68] (dword at edi-0x68)
    const double OTrapClrMul = params[13];      // [PVar - 0x74]

    const double K_HALF = 0.5;                  // [PVar + 0x00] = 0.5
    const double K_CONST2 = 0.03;               // [PVar + 0x08] = 0.03
    const double K_CONST3 = 0.1;               // [PVar + 0x10] = 0.1
    const double Rold = params[DIFS_ROLD_SLOT];

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    double h_axis = (Flip_XZ & 1) != 0 ? x : z;  // height comparison axis
    double d_axis0 = (Flip_XZ & 1) != 0 ? z : x;  // duck iteration variable 0

    double threshold = (fabs(FinalHeight_mul) + K_CONST3) * K_CONST2 + Rold;

    if (fabs(h_axis) > threshold) {
        double shape_de = -h_axis;

        if (Apply_scale_add == 0) {
            shape_de /= Scale;
        }
        de = shape_de;
        return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
    }

    double duck_p = d_axis0;
    double duck_q = y;
    double accum = 0.0;
    int iters = std::max(1, Iters); // body precedes DEC/JG

    for (int i = 0; i < iters; i++) {
        double r2 = duck_q * duck_q + duck_p * duck_p;
        double angle = log(r2) * K_HALF;

        angle += Seed_X;

        double angle2 = atan2(fabs(duck_q), duck_p);

        angle2 += Seed_Y;



        double new_p = angle * rot_sin - angle2 * rot_cos;
        double new_q = angle * rot_cos + angle2 * rot_sin;

        duck_p = new_p;
        duck_q = new_q;

        double r = sqrt(duck_p * duck_p + duck_q * duck_q);


        accum = accum * PerIterHeight_mul + r;
    }


    double result = log(1.0 + accum);


    double otrap_result;
    if (OTrapClrMul > 0.0) {
        otrap_result = result * OTrapClrMul;
    } else {
        otrap_result = result * result * OTrapClrMul;
    }
    params[MAX_FORMULA_PARAMS - 1] = fabs(otrap_result);

    result = result / double(Iters);

    result *= FinalHeight_mul;

    double shape_de = result - h_axis;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return Apply_scale_add != 0 ? cpu_double3(x, y, z) : p;
}

cpu_double3 ducksIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_DUCKSIFS>(p, params, de);
}

constant FormulaInfo formula_info_ducksIFS = {
    .id = FORMULA_DUCKSIFS,
    .name = "ducksIFS",
    .param_count = 14,
    .complexity = 315,
    .uses_transcendental = true
};

