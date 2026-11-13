#pragma once
#include "soft_double.h"

namespace mb3d_metal_exact {
struct NumericalDEInput { U64 values[6]; };
struct NumericalDEOutput { U64 raw, denominator; };
#ifndef __METAL_VERSION__
static_assert(sizeof(NumericalDEInput) == 48);
static_assert(sizeof(NumericalDEOutput) == 16);
#endif
}
