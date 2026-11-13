#include "ifs_original_state.h"
// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_DINIIFS = 10226;

#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void DiniDistance_x87(double*, const double*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_DINIIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[6]);
    const cpu_double3 transformed(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[1];
    double state[8]={transformed.x,transformed.y,transformed.z};
    const int flags[]={apply,int(params[9]),int(params[10])};
    DiniDistance_x87(state,params,flags);
    de=state[7];
    return apply!=0 ? transformed : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    Arm64ApplyIFSState native(p,params);
    using E=Arm64ApplyIFSState::E;
    E x=native.transformed.x,y=native.transformed.y,z=native.transformed.z;
    if(int(params[9])==1) { const E t=x; x=z; z=t; }
    if(int(params[9])==-1) { const E t=y; y=z; z=t; }
    if(int(params[10])!=0) z=-z;
    E minimum=params[14];
    for(int i=0;i<3;++i) {
        const E nx=-x,ny=-y;
        const E angle=double(extendedAtan2(y-x,y+x)*params[11]);
        const E square=x*x+y*y,radius=double(sqrt(square));
        const E delta=E(params[0])*params[0]-E(double(square));
        const E root=sqrt(delta+abs(delta)),argument=(root+params[0])/radius;
        const E infinity=std::numeric_limits<double>::infinity();
        const E logarithm=argument==E(0) ? -infinity : argument==infinity ? infinity : extendedLog2(argument);
        E distance=(abs((((extendedLn2()*logarithm)*params[0]+(-root))-angle)-z)-params[7])
                   *abs(radius*params[12]+params[13]);
        const E outer=radius-params[5],inner=E(params[8])-radius;
        if(outer>=distance) distance=outer;
        if(inner>=distance) distance=inner;
        if(ny>=distance) distance=ny;
        if(!(distance>=minimum)) minimum=double(distance);
        x=nx; y=ny; z=double(z+E(params[11])*x87Pi());
    }
    return native.finish(minimum,de);
#else
    // Portable arithmetic remains uncertified.
    const double Pseudo_Radius = params[0];   // [PVar-0x10] = edi-0x10 after correction
    const double Scale = params[1];
    const double Z_add = params[2];
    const double Y_add = params[3];
    const double X_add = params[4];
    const double Max_Radius = params[5];      // [edi-0x5c] = edi-0xa4 corrected? No.
    const int Apply_scale_add = int(params[6]);
    const double Thickness = params[7];       // [edi-0x54] = Thickness
    const double Min_Radius = params[8];      // [edi-0x48] = Min_Radius
    const int Flip_XZ_YZ = int(params[9]);   // [edi-0x60]
    const int Upside_down = int(params[10]); // [edi-0x64]
    const double Spiral_tightness = params[11]; // [edi-0x6c]

    const double const_a = params[12]; // 0.3
    const double const_b = params[13]; // 0.02
    const double const_c = params[14]; // 1e200

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    double out_x, out_y, out_z;
    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
        out_x = x;
        out_y = y;
        out_z = z;
    } else {
        out_x = p.x;
        out_y = p.y;
        out_z = p.z;
    }

    double running_min = const_c;

    if (Flip_XZ_YZ == 1) {
        double tmp = x;
        x = z;
        z = tmp;
    }
    if (Flip_XZ_YZ == -1) {
        double tmp = y;
        y = z;
        z = tmp;
    }
    if (Upside_down != 0) {
        z = -z;
    }

    for (int iter = 0; iter < 3; iter++) {
        double neg_y = -y;
        double neg_x = -x;

        double t = atan2(y - x, y + x) * Spiral_tightness;

        double r2 = x * x + y * y;
        double r = sqrt(r2);

        double a = Pseudo_Radius * Pseudo_Radius - r2;
        double b = a + fabs(a);
        double c = sqrt(b);
        double d = (c + Pseudo_Radius) / r;

        double e = log(d) * Pseudo_Radius;

        double f = fabs((e - c) - t - z);

        double shape_de = f - Thickness;

        double v = fabs(r * const_a + const_b);

        shape_de *= v;

        double w = r - Max_Radius;

        double result = max(w, shape_de);

        double u = Min_Radius - r;

        result = max(u, result);

        result = max(neg_y, result);

        running_min = min(result, running_min);

        x = neg_x;
        y = neg_y;
        z = z + Spiral_tightness * M_PI;
    }

    double shape_de = running_min;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(out_x, out_y, out_z);
#endif
}

cpu_double3 diniIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_DINIIFS>(p, params, de);
}

constant FormulaInfo formula_info_diniIFS = {
    .id = FORMULA_DINIIFS,
    .name = "diniIFS",
    .param_count = 15,
    .complexity = 413,
    .uses_transcendental = true
};

