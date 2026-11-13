#pragma once
#include "scene_abi.h"

// Fixed scene-only palette and lighting constants. No CPU per-pixel data.
struct alignas(16) ExactColorScene {
    ExactU32 version, width, height, reserved;
    ExactU64 z_corr, z_mul, z_mid_difference, step_width, back_distance;
    ExactU32 depth, shadow_start, shadow_z_mul, shadow_gradient;
    ExactU32 ambient_amplitude, palette_start, palette_mul, palette_z_mul;
    ExactU32 diffuse, specular, indirect_reflect, diffuse_shadowing;
    ExactU32 roughness, dynamic_fog_mul, light_pow, light_function;
    ExactU32 hs_mask, hs_calculated, hs_enabled, reserved2;
    ExactU32 depth_colors[2][4], ambient_colors[2][4], fog_colors[2][4];
    ExactU32 light_direction[4], light_color[4];
    ExactI32 palette_positions[10];
    ExactU32 palette_divisors[10], palette_diffuse[10][4], palette_specular[10][4];
    ExactU32 diffuse_table[8][128];
};
// Packed final RGB bytes: low byte red, next green, next blue. This matches
// the final image buffer, independently of production CalcPixelColor2 DIB order.
struct alignas(16) ExactColorPixel {
    ExactU32 rgb, fault, reserved0, reserved1;
};
#ifndef __METAL_VERSION__
static_assert(sizeof(ExactColorPixel)==16);
static_assert(sizeof(ExactColorScene)%16==0);
static_assert(offsetof(ExactColorScene, z_corr)==16);
#endif
