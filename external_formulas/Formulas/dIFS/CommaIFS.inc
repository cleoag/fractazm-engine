// Local comma-distance transform is always evaluated; coordinates and VaryScale
// are committed only when Apply scale/add is nonzero (original 0021..0040).
constexpr unsigned int FORMULA_COMMAIFS = 10206;
#if defined(_MSC_VER) && defined(_M_X64)
extern "C" void CommaIFSDistance_x87(double* state, const double* params, int apply);
#endif

template<>
cpu_double3 execute_formula<FORMULA_COMMAIFS>(cpu_double3 p, constant double* params, double &de) {
    const int apply = int(params[6]);
    // Original SSE2 preamble rounds each multiply and add to Double.
    const cpu_double3 transformed(p.x * params[1] + params[4],
                                  p.y * params[1] + params[3],
                                  p.z * params[1] + params[2]);
    if (apply != 0) params[DIFS_VARYSCALE_SLOT] *= params[1];
    double x = transformed.x, y = transformed.y, z = transformed.z;
    if (int(params[9]) != 0) { const double t = x; x = z; z = t; }
    if (int(params[10]) != 0) { const double t = y; y = z; z = t; }
    const int signs = int(params[11]);
    if (!(signs & 1)) x = -x;
    if (!(signs & 2)) y = -y;
    if (!(signs & 4)) z = -z;
#if defined(_MSC_VER) && defined(_M_X64)
    double state[] = {x, y, z, 0., 0., .159155};
    CommaIFSDistance_x87(state, params, apply);
    de = state[4];
#elif defined(MB3D_SOFTWARE_EXT80)
    using E = mb3d::numeric::Extended80;
    mb3d::numeric::ExtendedScope scope;
    const E sx=x, sy=y, sz=z;
    const E r=sqrt(sx*sx+sy*sy);
    const E theta=mb3d::numeric::extendedAtan2(sy,sx)*double(.159155);
    const E t2=theta*theta;
    const double radius=double((theta+(-t2*theta+t2))*params[0]+params[5]);
    const E a=r-params[7], b=sx+params[7];
    const E torus=sqrt(a*a+sz*sz)-radius;
    const E sphere=sqrt((b*b+sy*sy)+sz*sz)-params[8];
    E distance=!(sphere >= torus) ? sphere : torus;
    if (apply == 0) distance/=params[1];
    de=double(distance);
#else
    const long double sx = x, sy = y, sz = z;
    const long double r = sqrtl(sx*sx + sy*sy);
    const long double theta = atan2l(sy, sx) * (double).159155;
    const long double t2 = theta*theta;
    const double radius = double(((t2 - t2*theta) + theta)*params[0] + params[5]);
    const long double a = r - params[7], b = sx + params[7];
    const long double torus = sqrtl(a*a + sz*sz) - radius;
    const long double sphere = sqrtl(b*b + sy*sy + sz*sz) - params[8];
    long double distance = !(sphere >= torus) ? sphere : torus;
    if (apply == 0) distance /= params[1];
    de = double(distance);
#endif
    return apply != 0 ? transformed : p;
}

constant FormulaInfo formula_info_CommaIFS = {
    .id = FORMULA_COMMAIFS,
    .name = "CommaIFS",
    .param_count = 12,
    .complexity = 300,
    .uses_transcendental = true
};
