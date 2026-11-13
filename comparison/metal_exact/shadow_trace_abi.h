#pragma once
#include "shadow_abi.h"

// Independent full ray march, before applying the row's open-air shortcut.
// min_remaining covers entered loop starts only. A row shortcut can suppress
// terminal_fault, but can never suppress an invalid input recorded in fault.
struct alignas(16) ExactShadowTrace {
    ExactU32 version, active, fault, terminal_fault;
    ExactU64 min_remaining, final_remaining, last_found;
    ExactU32 has_loop, final_threshold, writes_last_found, reserved[3];
};

#ifndef __METAL_VERSION__
static_assert(sizeof(ExactShadowTrace) == 64);
static_assert(offsetof(ExactShadowTrace, min_remaining) == 16);
static_assert(offsetof(ExactShadowTrace, has_loop) == 40);
static_assert(offsetof(ExactShadowTrace, reserved) == 52);
#endif
