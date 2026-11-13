// Preserve original x87 arithmetic order and Double stores.
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void FoldingVarOriginal_x87(double*, const double*);
#endif
constexpr unsigned int FORMULA__FOLDINGVAR = 7077;


template<>
cpu_double3 execute_formula<FORMULA__FOLDINGVAR>(cpu_double3 p, constant double* params, double &de) {
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {p.x,p.y,p.z};
    FoldingVarOriginal_x87(state,params);
    return cpu_double3(state[0],state[1],state[2]);
#elif defined(MB3D_SOFTWARE_EXT80)
    using namespace mb3d::numeric;
    using E=Extended80;
    ExtendedScope scope;
    const E x=p.x,y=p.y,z=p.z;
    const E fx=((x*params[1]+y*params[2])+z*params[3])+params[0];
    const E fy=((x*params[5]+y*params[6])+z*params[7])+params[4];
    const E fz=(x*params[9]+y*params[10])+(z*params[11]+params[8]);
    return cpu_double3(double((abs(fx+x)-abs(x-fx))-x),
                       double((abs(fy+y)-abs(y-fy))-y),
                       double((abs(fz+z)-abs(z-fz))-z));
#else
    // Portable numerical parity remains unverified.

    const double Fold_X = params[0];   // default: 1.0
    const double xx     = params[1];   // default: 0
    const double xy     = params[2];   // default: 0
    const double xz     = params[3];   // default: 0
    const double Fold_Y = params[4];   // default: 1.0
    const double yx     = params[5];   // default: 0
    const double yy     = params[6];   // default: 0
    const double yz     = params[7];   // default: 0
    const double Fold_Z = params[8];   // default: 1.0
    const double zx     = params[9];   // default: 0
    const double zy     = params[10];  // default: 0
    const double zz     = params[11];  // default: 0

    double x = p.x, y = p.y, z = p.z;

    double Xfold = (x * xx + y * xy) + z * xz + Fold_X;
    double Yfold = (x * yx + y * yy) + z * yz + Fold_Y;
    double Zfold = (x * zx + y * zy) + (z * zz + Fold_Z);

    double new_x = abs(x + Xfold) - abs(x - Xfold) - x;
    double new_y = abs(y + Yfold) - abs(y - Yfold) - y;
    double new_z = abs(z + Zfold) - abs(z - Zfold) - z;

    return cpu_double3(new_x, new_y, new_z);
#endif
}

constant FormulaInfo formula_info__FoldingVar = {
    .id = FORMULA__FOLDINGVAR,
    .name = "_FoldingVar",
    .param_count = 12,
    .complexity = 145,
    .uses_transcendental = false
};
