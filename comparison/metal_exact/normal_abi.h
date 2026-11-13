#pragma once
#include "scene_abi.h"

// Six independent DE results in production probe order Z+, Z-, X+, X-, Y+, Y-.
// Each GPU task owns one distance word; only Y- writes trap and reserved.
// Integer encodings preserve every binary64 bit between dispatches.
struct alignas(16) ExactNormalSamples {
    ExactU64 distance[6];
    ExactU64 orbit_trap, reserved;
};

#ifndef __METAL_VERSION__
static_assert(sizeof(ExactNormalSamples) == 64);
static_assert(offsetof(ExactNormalSamples, distance) == 0);
static_assert(offsetof(ExactNormalSamples, orbit_trap) == 48);
static_assert(offsetof(ExactNormalSamples, reserved) == 56);
#endif
