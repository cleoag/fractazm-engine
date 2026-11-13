#include "core/numerical_camera.h"
#include "core/numeric_precision.h"
#include "core/extended_transcendentals.h"

#include <cmath>

namespace mb3d {
namespace {

template<class Real> struct SinCos { Real sin, cos; };
SinCos<double> sineCosine(double angle) {
    return {std::sin(angle), std::cos(angle)};
}

#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
SinCos<numeric::Extended80> sineCosine(numeric::Extended80 angle) {
    const auto value = numeric::extendedSinCos(angle);
    return {value.sin, value.cos};
}
#endif

// Math3D.pas BuildViewVectorDFOV keeps both FSINCOS results extended,
// normalizes in (x*x + z*z) + y*y order, then stores x/z/y as Double.
template<class Real>
void buildPerspective(double cafy, double cafx, double* result) {
    using std::sqrt;
    const auto a = sineCosine(Real(cafy));
    const auto b = sineCosine(Real(cafx));
    const Real x = -b.sin, y = a.sin, z = a.cos * b.cos;
    const Real inverseLength = Real(1) / sqrt((x * x + z * z) + y * y);
    result[0] = double(x * inverseLength);
    result[2] = double(z * inverseLength);
    result[1] = double(y * inverseLength);
}

// The spherical routine has no normalization and no intermediate Double
// trigonometric stores. Its output stores occur before matrix rotation.
template<class Real>
void buildSpherical(double cafy, double cafx, double* result) {
    const auto b = sineCosine(Real(cafx));
    const auto a = sineCosine(Real(cafy));
    result[1] = double(a.sin);
    result[2] = double(a.cos * b.cos);
    result[0] = double(-(b.sin * a.cos));
}

template<class Real>
void rotateReverse(double* vector, const double* matrix) {
    const Real x(vector[0]), y(vector[1]), z(vector[2]);
    const Real outX = (Real(matrix[0]) * x + Real(matrix[3]) * y) + Real(matrix[6]) * z;
    const Real outY = (Real(matrix[1]) * x + Real(matrix[4]) * y) + Real(matrix[7]) * z;
    const Real outZ = (Real(matrix[2]) * x + Real(matrix[5]) * y) + Real(matrix[8]) * z;
    vector[2] = double(outZ);
    vector[1] = double(outY);
    vector[0] = double(outX);
}

} // namespace

void NumericalBuildViewVectorDFOV(double cafy, double cafx, double* result) {
#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
    if (!NativePrecisionEnabled()) {
        const numeric::ExtendedScope scope;
        buildPerspective<numeric::Extended80>(cafy, cafx, result);
        return;
    }
#endif
    buildPerspective<double>(cafy, cafx, result);
}

void NumericalBuildViewVectorDSphereFOV(double cafy, double cafx, double* result) {
#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
    if (!NativePrecisionEnabled()) {
        const numeric::ExtendedScope scope;
        buildSpherical<numeric::Extended80>(cafy, cafx, result);
        return;
    }
#endif
    buildSpherical<double>(cafy, cafx, result);
}

void NumericalRotateVectorReverse(double* vector, const double* matrix9) {
#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
    if (!NativePrecisionEnabled()) {
        const numeric::ExtendedScope scope;
        rotateReverse<numeric::Extended80>(vector, matrix9);
        return;
    }
#endif
    rotateReverse<double>(vector, matrix9);
}

void NumericalBuildRotateViewVectorDFOV(double cafy, double cafx,
                                       const double* matrix9, double* result) {
#if defined(MB3D_SOFTWARE_EXT80) || defined(MB3D_HAVE_SOFT_EXT80)
    if (!NativePrecisionEnabled()) {
        const numeric::ExtendedScope scope;
        buildPerspective<numeric::Extended80>(cafy, cafx, result);
        // The original build stores binary64 before RotateVectorReverse loads.
        rotateReverse<numeric::Extended80>(result, matrix9);
        return;
    }
#endif
    buildPerspective<double>(cafy, cafx, result);
    rotateReverse<double>(result, matrix9);
}

} // namespace mb3d
