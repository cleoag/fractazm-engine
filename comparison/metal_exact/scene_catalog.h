#pragma once
#include "scene_abi.h"
#include "soft_double.h"

namespace mb3d_metal_exact {

enum : ExactU32 {
    kRuntimeSphereCapability = 101,
    kRuntimeBoxCapability = 102,
    kRuntimeBulbCapability = 103,
};

inline bool finite_formula_value(ExactU64 value) {
    return (value & 0x7ff0000000000000ULL) != 0x7ff0000000000000ULL;
}
inline bool positive_formula_value(ExactU64 value) {
    return finite_formula_value(value) && value != 0 && !(value >> 63);
}
inline bool boolean_formula_value(ExactU64 value) {
    return value == 0 || value == 0x3ff0000000000000ULL;
}
inline bool trap_formula_value(ExactU64 value) {
    return boolean_formula_value(value) || value == 0x4000000000000000ULL ||
           value == 0x4008000000000000ULL;
}
inline bool single_formula_value(ExactU64 value) {
    const F64 input = F64::from_bits(value);
    return finite_formula_value(value) &&
           F64::from_f32_bits(input.to_f32_bits()).to_bits() == value;
}

// Runtime capabilities describe implemented numerical branches. Source hashes
// remain evidence identities; they are not the admission rule for these IDs.
inline bool valid_runtime_formula_scene(ExactScene scene) {
    if (scene.max_iterations < 1 || scene.max_iterations > 256 ||
        scene.slot_count < 1 || scene.slot_count > 65535) return false;
    for (unsigned index = 0; index < 13; ++index)
        if (!finite_formula_value(scene.params[index])) return false;
    if (scene.capability_id == kRuntimeBulbCapability && scene.formula_kind == 3) {
        const ExactU64 fixed[7] = {
            0x2b2bff2ee48e0530ULL, 0x3ff0000000000000ULL, 0x4018000000000000ULL,
            0x401c000000000000ULL, 0x4020000000000000ULL, 0x403c000000000000ULL,
            0x4051800000000000ULL
        };
        if ((scene.params[0] & 0x7fffffffffffffffULL) == 0) return false;
        for (unsigned index = 0; index < 7; ++index)
            if (scene.params[index + 1] != fixed[index]) return false;
        for (unsigned index = 8; index < 13; ++index)
            if (scene.params[index] != 0) return false;
        const auto bulb = scene.bulb;
        return bulb.power >= 2 && bulb.power <= 8 &&
               !bulb.reserved && positive_formula_value(bulb.bailout) &&
               bulb.bailout > 0x3ff0000000000000ULL &&
               single_formula_value(bulb.bailout) &&
               positive_formula_value(bulb.gradient_bailout) &&
               single_formula_value(bulb.gradient_bailout) &&
               positive_formula_value(bulb.de_offset) &&
               positive_formula_value(bulb.de_offset006) &&
               (bulb.smooth_hln & 0x7f800000U) != 0x7f800000U &&
               bulb.smooth_hln != 0 && !(bulb.smooth_hln >> 31) &&
               (bulb.smooth_lnrstop & 0x7f800000U) != 0x7f800000U;
    }
    if (scene.capability_id == kRuntimeSphereCapability && scene.formula_kind == 1) {
        if (!positive_formula_value(scene.params[0]) ||
            !positive_formula_value(scene.params[1])) return false;
        for (unsigned index = 5; index <= 7; ++index)
            if (!positive_formula_value(scene.params[index])) return false;
        return boolean_formula_value(scene.params[8]) &&
               trap_formula_value(scene.params[9]) &&
               single_formula_value(scene.params[10]) &&
               single_formula_value(scene.params[11]);
    }
    if (scene.capability_id == kRuntimeBoxCapability && scene.formula_kind == 2) {
        for (unsigned index = 0; index <= 3; ++index)
            if (!positive_formula_value(scene.params[index])) return false;
        return scene.params[8] == 0 &&
               boolean_formula_value(scene.params[9]) &&
               trap_formula_value(scene.params[10]) &&
               single_formula_value(scene.params[11]) &&
               single_formula_value(scene.params[12]);
    }
    return false;
}

// Only the declared formula, coefficients and iteration profile belong to a
// capability. A known formula kind alone does not admit arbitrary parameters.
inline bool valid_formula_scene(ExactScene scene) {
    if (scene.color_on_it || scene.color_radius_stop ||
        scene.min_iterations != 1 || scene.refinement_steps != 8) return false;
    if (scene.formula_kind != 3 && (scene.bulb.bailout || scene.bulb.gradient_bailout ||
        scene.bulb.de_offset || scene.bulb.de_offset006 || scene.bulb.smooth_hln ||
        scene.bulb.smooth_lnrstop || scene.bulb.power || scene.bulb.reserved)) return false;
    switch (scene.capability_id) {
#define MB3D_EXACT_SCENE(capId, sceneId, pathText, hashText, kindValue, nameText, maxItValue, slotValue, shadowValue, ...) \
        case capId: { \
            if (scene.formula_kind != kindValue || scene.max_iterations != maxItValue || \
                scene.slot_count != slotValue) return false; \
            const ExactU64 expected[13] = {__VA_ARGS__}; \
            for (unsigned index = 0; index < 13; ++index) \
                if (scene.params[index] != expected[index]) return false; \
            return true; \
        }
#include "scene_manifest.inc"
#undef MB3D_EXACT_SCENE
        default: return valid_runtime_formula_scene(scene);
    }
}

// Standalone shadow parameters must use a catalogued iteration limit. A
// complete dispatch must additionally pair that limit with its actual scene.
inline bool valid_formula_iteration_limit(ExactU32 limit) {
    return (limit >= 1 && limit <= 256)
#define MB3D_EXACT_SCENE(capId, sceneId, pathText, hashText, kindValue, nameText, maxItValue, slotValue, shadowValue, ...) \
        || limit == maxItValue
#include "scene_manifest.inc"
#undef MB3D_EXACT_SCENE
        ;
}
} // namespace mb3d_metal_exact

#ifndef __METAL_VERSION__
#include <string_view>

namespace mb3d::metal_exact {
struct ExactSceneCapability {
    ExactU32 capability_id;
    const char* scene_id;
    const char* source_path;
    const char* source_sha256;
    ExactU32 formula_kind;
    const char* formula_name;
    ExactU32 max_iterations, slot_count, original_shadow_bits;
    ExactU64 params[13];
};

inline constexpr ExactSceneCapability kExactSceneCapabilities[] = {
#define MB3D_EXACT_SCENE(capId, sceneId, pathText, hashText, kindValue, nameText, maxItValue, slotValue, shadowValue, ...) \
    {capId, sceneId, pathText, hashText, kindValue, nameText, maxItValue, slotValue, shadowValue, {__VA_ARGS__}},
#include "scene_manifest.inc"
#undef MB3D_EXACT_SCENE
};

constexpr bool ExactCatalogHasSupportedBoxOptions() {
    for (const auto& capability : kExactSceneCapabilities) {
        if (capability.formula_kind != 2) continue;
        if (capability.params[8] != 0 ||
            capability.params[9] != 0x3ff0000000000000ULL ||
            capability.params[10] != 0 ||
            capability.params[11] != 0x3ff0000000000000ULL ||
            capability.params[12] != 0x3ff0000000000000ULL) return false;
    }
    return true;
}
static_assert(ExactCatalogHasSupportedBoxOptions(),
              "Exact boxIFS scenes require Twist=+0, Apply=1 and RStopD OTrap with unit scale/add");

constexpr bool ExactCatalogHasSupportedSphereOptions() {
    // SphereIFSFormula keeps an optimized literal step, including exact 2^i.
    constexpr ExactU64 expected[13] = {
        0x3ff0000000000000ULL, 0x4000000000000000ULL, 0, 0,
        0x4008000000000000ULL, 0x3ff0000000000000ULL, 0x3ff0000000000000ULL,
        0x3ff0000000000000ULL, 0x3ff0000000000000ULL, 0,
        0x3ff0000000000000ULL, 0x3ff0000000000000ULL, 0xbff0000000000000ULL
    };
    for (const auto& capability : kExactSceneCapabilities) {
        if (capability.formula_kind != 1) continue;
        if (capability.max_iterations > 60) return false;
        for (unsigned i = 0; i < 13; ++i)
            if (capability.params[i] != expected[i]) return false;
    }
    return true;
}
static_assert(ExactCatalogHasSupportedSphereOptions(),
              "Exact SphereIFS scenes require the qualified literal coefficient profile");

inline const ExactSceneCapability* FindExactSceneCapability(std::string_view sha256) {
    for (const auto& capability : kExactSceneCapabilities)
        if (sha256 == capability.source_sha256) return &capability;
    return nullptr;
}

inline const ExactSceneCapability* FindExactSceneCapabilityById(ExactU32 id) {
    for (const auto& capability : kExactSceneCapabilities)
        if (id == capability.capability_id) return &capability;
    return nullptr;
}
} // namespace mb3d::metal_exact
#endif
