// Numerical reference: immutable original M3F CODE.
constexpr unsigned int FORMULA_QUADRICSIFS = 10066;


#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void QuadricsDistance_x87(double*, const double*, const int*);
#endif

template<>
cpu_double3 execute_formula<FORMULA_QUADRICSIFS>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    const int apply=int(params[6]);
    const cpu_double3 transformed(p.x*params[1]+params[4],p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if(apply!=0) params[DIFS_VARYSCALE_SLOT]*=params[1];
    double state[4]={transformed.x,transformed.y,transformed.z};
    const int flags[]={apply,int(params[9]),int(params[10])};
    QuadricsDistance_x87(state,params,flags);
    de=state[3];
    return apply!=0 ? transformed : p;
#elif defined(MB3D_SOFTWARE_EXT80)
    const numeric::ExtendedScope scope;
    using E = numeric::Extended80;
    const int apply = int(params[6]);
    // The SSE preamble stores transformed coordinates as Double. The later
    // X/Z swap is local to distance evaluation; it is not a coordinate commit.
    const cpu_double3 transformed(p.x*params[1]+params[4],
                                  p.y*params[1]+params[3],p.z*params[1]+params[2]);
    if (apply) params[DIFS_VARYSCALE_SLOT] *= params[1];
    E x(transformed.x), y(transformed.y), z(transformed.z);
    if (int(params[10])) std::swap(x,z);
    const E quadric = sqrt(abs((z*z+(y*y+(x*x)*E(params[7])))-E(params[8])))-E(params[5]);
    // 0088 jumps past BOTH fabs and subtraction when half-cut is enabled.
    E best = int(params[9]) ? x : abs(x)-E(params[0]);
    const E yCut = abs(y)-E(params[0]), zCut = abs(z)-E(params[0]);
    if (yCut >= best) best = yCut;
    if (zCut >= best) best = zCut;
    E distance = best >= quadric ? best : quadric;
    if (!apply) distance /= E(params[1]);
    de = double(distance);
    return apply ? transformed : p;
#else
    // Portable arithmetic remains uncertified.
    const double CutBoxSz   = params[0];    // .Double CutBoxSz = 2
    const double Scale       = params[1];    // .2Doubles Scale = 2
    const double Z_add       = params[2];    // .2Doubles Z add = 0
    const double Y_add       = params[3];    // .Double Y add = 0
    const double X_add       = params[4];    // .Double X add = 0
    const double Thickness   = params[5];    // .Double Thickness = .5
    const int Apply_scale_add = int(params[6]); // .Integer Apply scale+add = 1
    const double zMul        = params[7];    // .Double zMul = -.7
    const double HypOffset   = params[8];    // .Double HypOffset = -.2
    const int HalfCut       = int(params[9]);   // .Integer Half cut = 0
    const int Flip_XZ       = int(params[10]);  // .Integer Flip X-Z = 0

    double x = p.x * Scale + X_add;
    double y = p.y * Scale + Y_add;
    double z = p.z * Scale + Z_add;

    if (Apply_scale_add != 0) {
        params[DIFS_VARYSCALE_SLOT] *= Scale;
    }

    if (Flip_XZ != 0) {
        double tmp = x;
        x = z;
        z = tmp;
    }

    double quadric = x * x * zMul + y * y + z * z - HypOffset;
    double quadric_de = sqrt(fabs(quadric)) - Thickness;

    double x_de;
    if (HalfCut == 0) {
        x_de = fabs(x) - CutBoxSz;
    } else {
        x_de = x - CutBoxSz;  // no abs → half-space cut
    }

    double y_de = fabs(y) - CutBoxSz;

    double best = (y_de < x_de) ? x_de : y_de;

    double z_de = fabs(z) - CutBoxSz;

    best = (z_de < best) ? best : z_de;

    double shape_de = (best < quadric_de) ? quadric_de : best;

    if (Apply_scale_add == 0) {
        shape_de /= Scale;
    }

    de = shape_de;
    return cpu_double3(x, y, z);
#endif
}

cpu_double3 QuadricsIFS_formula(cpu_double3 p, constant double* params, double &de) {
    return execute_formula<FORMULA_QUADRICSIFS>(p, params, de);
}

constant FormulaInfo formula_info_QuadricsIFS = {
    .id = FORMULA_QUADRICSIFS,
    .name = "QuadricsIFS",
    .param_count = 11,
    .complexity = 222,
    .uses_transcendental = false
};

