#pragma once
#include "scene_abi.h"

// Restricted production CalcAO_HiQ (ATi=2). All Single values cross the ABI as
// integer encodings. Scalar preparation is scene/dimension dependent only.
struct alignas(16) ExactAOScene {
    ExactU32 version, width, height, num_levels;
    ExactI32 w_lo, w_hi, h_lo, h_hi;
    ExactU32 directions[64];
};

struct alignas(16) ExactAOLevel {
    ExactU32 version, index, i_step, step_count;
    ExactI32 max_radius;
    ExactU32 sit, z_threshold, sum_multiplier;
    ExactU32 min_radius, reciprocal_radius[6], reserved;
};

#ifndef __METAL_VERSION__
static_assert(sizeof(ExactAOScene) == 288);
static_assert(sizeof(ExactAOLevel) == 64);
static_assert(offsetof(ExactAOScene, directions) == 32);
static_assert(offsetof(ExactAOLevel, sit) == 20);
#endif
