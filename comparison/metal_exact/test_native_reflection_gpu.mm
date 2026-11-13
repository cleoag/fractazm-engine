#include "native_reflection_host.h"
#include "metal_device.h"
#include "color_host.h"
#include "shadow_host.h"
#include "io/m3p_parser.h"
#include <array>
#include <bit>
#include <cstdio>
#include <cstring>
#include <limits>
#include <sys/sysctl.h>

using namespace mb3d;
using namespace mb3d::metal_exact;
namespace gpu = mb3d::metal_exact::gpu;

int main(int argc, char** argv) {
#if !defined(__APPLE__) || !defined(__aarch64__)
    std::fprintf(stderr, "Native reflection GPU guards require native Apple ARM64.\n");
    return 2;
#else
    @autoreleasepool {
        unsigned checks = 0, failures = 0;
        auto check = [&](bool success, const char* description) {
            ++checks;
            if (!success && ++failures <= 20) std::fprintf(stderr, "FAIL: %s\n", description);
        };
        try {
            gpu::require(argc == 3, "usage: test_native_reflection_gpu native.metallib integerpower8-reflections.m3p");
            int translated = 0;
            size_t translated_size = sizeof(translated);
            (void)sysctlbyname("sysctl.proc_translated", &translated, &translated_size, nullptr, 0);
            gpu::require(translated == 0, "GPU guard host must run natively");
            const auto loaded = LoadM3PFile(argv[2]);
            gpu::require(loaded.success, loaded.error);
            const auto prepared = PrepareLoadedExactScene(loaded.header, loaded.addon, 1, 1, "",
                                                          65536, PrecisionMode::Native);
            const auto reflection = PrepareNativeReflection(prepared);
            const auto packing = PrepareSurface(prepared);
            const auto shadow = PrepareShadow(prepared);
            const auto color = PrepareExactColor(prepared);
            gpu::Metal metal(argv[1], prepared.device.formula_kind, prepared.device.bulb.power,
                             prepared.device.capability_id, false, true);
            check(metal.arithmeticProfile == std::array<uint32_t, 3>{2, 4, 32},
                  "actual offline library is Native FP32");

            const std::array<uint32_t, 4> marker_initial{0, 0, 0, 0xc5a92b71};
            auto marker_buffer = metal.buffer(sizeof(marker_initial), marker_initial.data());
            auto marker = metal.pipeline(@"native_reflection_abi");
            gpu::dispatchStage(metal, marker, 1, [&](id<MTLComputeCommandEncoder> encoder) {
                [encoder setBuffer:marker_buffer offset:0 atIndex:0];
            });
            std::array<uint32_t, 4> marker_actual{};
            std::memcpy(marker_actual.data(), marker_buffer.contents, sizeof(marker_actual));
            check(marker_actual == std::array<uint32_t, 4>{1, 640, 64, marker_initial[3]},
                  "reflection ABI marker and trailing canary");

            auto kernel = metal.pipeline(@"native_reflection");
            std::array<ExactPixel, 2> primary{};
            std::array<ExactSurface, 2> surface{};
            std::array<ExactColorPixel, 2> rgb{};
            std::array<NativeReflectionRecord, 2> records{};
            // No CPU per-pixel renderer or numerical oracle: this is a known
            // background no-op and deliberately invalid descriptor admission.
            surface[0].depth_packed = 0x80000000u;
            rgb[0].rgb = 0x00332211u;
            std::memset(&primary[1], 0xa5, sizeof(primary[1]));
            std::memset(&surface[1], 0x96, sizeof(surface[1]));
            std::memset(&rgb[1], 0xc7, sizeof(rgb[1]));
            std::memset(records.data(), 0x5a, sizeof(records));
            auto primary_buffer = metal.buffer(sizeof(primary), primary.data());
            auto surface_buffer = metal.buffer(sizeof(surface), surface.data());
            auto rgb_buffer = metal.buffer(sizeof(rgb), rgb.data());
            auto record_buffer = metal.buffer(sizeof(records), records.data());
            auto color_buffer = metal.buffer(sizeof(color.device), &color.device);

            auto dispatch = [&](const ExactScene& scene, const NativeReflectionParams& params,
                                bool expect_fault, const char* description) {
                std::memcpy(rgb_buffer.contents, rgb.data(), sizeof(rgb));
                std::memcpy(record_buffer.contents, records.data(), sizeof(records));
                // Two physical entries exist for canaries, but only entry0 is
                // admitted. Over-dispatch must leave entry1 untouched even if
                // descriptor dimensions are invalid or disagree with scene.
                const uint32_t launch[]{0, 2, 1};
                gpu::dispatchStage(metal, kernel, 2, [&](id<MTLComputeCommandEncoder> encoder) {
                    [encoder setBytes:&scene length:sizeof(scene) atIndex:0];
                    [encoder setBytes:&packing length:sizeof(packing) atIndex:1];
                    [encoder setBytes:&shadow length:sizeof(shadow) atIndex:2];
                    [encoder setBuffer:color_buffer offset:0 atIndex:3];
                    [encoder setBytes:&params length:sizeof(params) atIndex:4];
                    [encoder setBuffer:primary_buffer offset:0 atIndex:5];
                    [encoder setBuffer:surface_buffer offset:0 atIndex:6];
                    [encoder setBuffer:rgb_buffer offset:0 atIndex:7];
                    [encoder setBuffer:record_buffer offset:0 atIndex:8];
                    [encoder setBytes:launch length:sizeof(launch) atIndex:9];
                });
                NativeReflectionRecord actual{}, expected{};
                expected.fault = expect_fault ? 1 : 0;
                std::memcpy(&actual, record_buffer.contents, sizeof(actual));
                check(std::memcmp(&actual, &expected, sizeof(actual)) == 0, description);
                check(std::memcmp(rgb_buffer.contents, rgb.data(), sizeof(rgb)) == 0,
                      "background/invalid input preserves RGB and trailing canary");
                check(std::memcmp(static_cast<const unsigned char*>(record_buffer.contents) + sizeof(actual),
                                  &records[1], sizeof(records[1])) == 0,
                      "independent launch allocation bound preserves record1 canary");
                check(std::memcmp(primary_buffer.contents, primary.data(), sizeof(primary)) == 0 &&
                      std::memcmp(surface_buffer.contents, surface.data(), sizeof(surface)) == 0,
                      "shader does not modify primary or surface inputs");
            };
            dispatch(prepared.device, reflection, false, "admitted background is exact zero-work no-op");
            dispatch(prepared.device, reflection, false, "admitted background repeats exactly");
            auto invalid = [&](auto&& mutate, const char* description) {
                auto params = reflection;
                mutate(params);
                dispatch(prepared.device, params, true, description);
            };
            invalid([](auto& p) { p.version = 0; }, "invalid reflection version rejected by GPU");
            invalid([](auto& p) { p.max_bounces = 0; }, "zero bounce count rejected by GPU");
            invalid([](auto& p) { p.max_bounces = 3; }, "unsupported bounce count rejected by GPU");
            invalid([](auto& p) { p.max_steps = 0; }, "zero work bound rejected by GPU");
            invalid([](auto& p) { p.max_steps = 65537; }, "oversized work bound rejected by GPU");
            invalid([](auto& p) { p.amount = std::bit_cast<uint32_t>(std::numeric_limits<float>::quiet_NaN()); },
                    "NaN reflection amount rejected by GPU");
            invalid([](auto& p) { p.amount = std::bit_cast<uint32_t>(1.1f); }, "unsupported reflection amount rejected by GPU");
            invalid([](auto& p) { p.first_step_random = 1; }, "exact reflection first-step jitter rejected by GPU");
            invalid([](auto& p) { p.width = 0; }, "zero width rejected before buffer access");
            invalid([](auto& p) { p.width = 2; }, "allocation/descriptor mismatch rejected before buffer access");
            invalid([](auto& p) { p.height = 0; }, "zero height rejected before buffer access");
            invalid([](auto& p) { p.width = 0xffffffffu; }, "oversized dimension rejected before multiplication");
            {
                auto scene = prepared.device; scene.width = 2;
                dispatch(scene, reflection, true, "scene/descriptor dimension mismatch rejected by GPU");
            }
        } catch (const std::exception& error) {
            check(false, error.what());
        }
        std::printf("Native reflection GPU guards: %u checks, %u failures\n", checks, failures);
        return failures ? 1 : 0;
    }
#endif
}
