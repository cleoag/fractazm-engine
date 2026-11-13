#pragma once

namespace mb3d {

// Camera arithmetic follows the active render precision policy. Compatible
// mode preserves the original extended operations and binary64 vector stores;
// native mode evaluates the same operation order with hardware double math.
void NumericalBuildViewVectorDFOV(double cafy, double cafx, double* result);
void NumericalBuildViewVectorDSphereFOV(double cafy, double cafx, double* result);
void NumericalRotateVectorReverse(double* vector, const double* matrix9);
void NumericalBuildRotateViewVectorDFOV(double cafy, double cafx,
                                       const double* matrix9, double* result);

} // namespace mb3d
