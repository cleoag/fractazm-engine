#pragma once

namespace mb3d {

// The original numerical DE mixes a binary64 denominator with an extended
// logarithm, numerator and division, narrowing only the final result.
double NumericalDistanceEstimate(double radius, double dxSquared,
                                 double dySquared, double dzSquared,
                                 double scale, double offset,
                                 double* denominatorOut = nullptr);

} // namespace mb3d
