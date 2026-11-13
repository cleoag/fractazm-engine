#pragma once
#include "shadow_abi.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <vector>

// Included after the shared Metal driver, dispatchStage and readBuffer helpers.
// These trials reuse device-produced primary/surface/ray inputs. The host reads
// results only for comparison; it never supplies per-pixel rendering results.
template<class T>
static void checkShadowDispatchRecords(id<MTLBuffer> buffer,
                                       const std::vector<T>& expected,
                                       uint64_t& differences, uint64_t& faults) {
    const auto actual = readBuffer<T>(buffer);
    require(actual.size() == expected.size(), "shadow dispatch trial record count mismatch");
    for (size_t i = 0; i < actual.size(); ++i) {
        differences += std::memcmp(&actual[i], &expected[i], sizeof(T)) != 0;
        faults += actual[i].fault != 0;
    }
}

static NSDictionary* shadowDispatchTrials(
    Metal& m, const Options& o, ExactScene scene, ExactShadowParams shadow,
    id<MTLBuffer> primary, id<MTLBuffer> surface, id<MTLBuffer> rays,
    const std::vector<ExactShadowRay>& expectedRays,
    const std::vector<ExactSurface>& expectedShadow,
    id<MTLComputePipelineState> prepare, id<MTLComputePipelineState> march) {
    (void)o;
    require(shadow.enabled == 1, "shadow dispatch trials require enabled hard shadows");
    require(scene.width > 0 && scene.height > 0 &&
            scene.width <= 4096 && scene.height <= 4096,
            "invalid shadow dispatch trial dimensions");
    const uint32_t count = scene.width * scene.height;
    require(expectedRays.size() == count && expectedShadow.size() == count,
            "shadow dispatch trial reference dimensions mismatch");
    require(primary.length == size_t(count) * sizeof(ExactPixel) &&
            surface.length == size_t(count) * sizeof(ExactSurface) &&
            rays.length == size_t(count) * sizeof(ExactShadowRay),
            "shadow dispatch trial input buffer size mismatch");
    require(primary != surface && primary != rays && surface != rays,
            "shadow dispatch trial input buffers must be distinct");
    const unsigned prepareWidth = unsigned(prepare.threadExecutionWidth);
    const unsigned prepareMaximum = unsigned(prepare.maxTotalThreadsPerThreadgroup);
    const unsigned marchWidth = unsigned(march.threadExecutionWidth);
    require(prepareWidth > 0 && marchWidth > 0 &&
            marchWidth <= march.maxTotalThreadsPerThreadgroup,
            "invalid shadow dispatch trial pipeline limits");

    constexpr unsigned warmupRuns = 1, timedRuns = 3;
    constexpr unsigned totalRuns = warmupRuns + timedRuns;
    const uint64_t checkedRecords = uint64_t(count) * totalRuns;
    NSMutableArray* preparationTrials = [NSMutableArray array];
    NSMutableArray* marchTrials = [NSMutableArray array];
    bool pass = true;

    for (unsigned group : {32u, 64u, 128u, 256u}) {
        if (group % prepareWidth != 0 || group > prepareMaximum) continue;
        std::vector<double> samples;
        NSMutableArray* gpuTimes = [NSMutableArray array];
        uint64_t differences = 0, faults = 0;
        double warmup = 0;
        for (unsigned run = 0; run < totalRuns; ++run) {
            // Diagnostic poison is not a render result. Clearing before
            // timestamps makes missing or partial writes fail exact comparison.
            std::memset(rays.contents, 0xa5, rays.length);
            const double ms = dispatchStage(m, prepare, count,
                [&](id<MTLComputeCommandEncoder> encoder) {
                    [encoder setBuffer:primary offset:0 atIndex:0];
                    [encoder setBuffer:surface offset:0 atIndex:1];
                    [encoder setBuffer:rays offset:0 atIndex:2];
                    [encoder setBytes:&scene length:sizeof(scene) atIndex:3];
                    [encoder setBytes:&shadow length:sizeof(shadow) atIndex:4];
                    [encoder setBytes:&count length:sizeof(count) atIndex:5];
                }, 1, group);
            require(std::isfinite(ms) && ms > 0, "invalid shadow preparation trial timestamp");
            checkShadowDispatchRecords(rays, expectedRays, differences, faults);
            if (run == 0) warmup = ms;
            else {
                samples.push_back(ms);
                [gpuTimes addObject:@(ms)];
            }
        }
        pass &= differences == 0 && faults == 0;
        [preparationTrials addObject:@{
            @"threads_per_threadgroup":@(group),
            @"warmup_gpu_ms":@(warmup),
            @"gpu_ms":gpuTimes,
            @"median_gpu_ms":@(median(samples)),
            @"different_records":@(differences),
            @"fault_records":@(faults),
            @"checked_records":@(checkedRecords),
            @"record_bytes":@(sizeof(ExactShadowRay))
        }];
    }
    require(preparationTrials.count != 0, "no legal shadow preparation trial threadgroup");

    // The only new device allocation is a separate output surface. In
    // particular, every row-batch trial starts from the immutable packed
    // surface and the GPU-prepared rays, not a prior trial's shadow output.
    auto output = m.buffer(size_t(count) * sizeof(ExactSurface));
    std::vector<unsigned> batches{128u, 256u, 512u, scene.height};
    std::sort(batches.begin(), batches.end());
    batches.erase(std::unique(batches.begin(), batches.end()), batches.end());
    for (unsigned batch : batches) {
        std::vector<double> samples;
        NSMutableArray* gpuTimes = [NSMutableArray array];
        uint64_t differences = 0, faults = 0;
        double warmup = 0, maxCommand = 0;
        for (unsigned run = 0; run < totalRuns; ++run) {
            std::memset(output.contents, 0xa5, output.length);
            double total = 0;
            for (uint32_t y = 0; y < scene.height; y += batch) {
                const uint32_t launch[]{y, scene.width, scene.height};
                const double ms = dispatchStage(m, march, std::min(batch, scene.height - y),
                    [&](id<MTLComputeCommandEncoder> encoder) {
                        [encoder setBuffer:rays offset:0 atIndex:0];
                        [encoder setBuffer:surface offset:0 atIndex:1];
                        [encoder setBuffer:output offset:0 atIndex:2];
                        [encoder setBytes:&scene length:sizeof(scene) atIndex:3];
                        [encoder setBytes:&shadow length:sizeof(shadow) atIndex:4];
                        [encoder setBytes:launch length:sizeof(launch) atIndex:5];
                    }, 1, marchWidth);
                require(std::isfinite(ms) && ms > 0, "invalid shadow march trial timestamp");
                total += ms;
                maxCommand = std::max(maxCommand, ms);
            }
            checkShadowDispatchRecords(output, expectedShadow, differences, faults);
            if (run == 0) warmup = total;
            else {
                samples.push_back(total);
                [gpuTimes addObject:@(total)];
            }
        }
        pass &= differences == 0 && faults == 0;
        [marchTrials addObject:@{
            @"rows_per_command":@(batch),
            @"threads_per_threadgroup":@(marchWidth),
            @"warmup_gpu_ms":@(warmup),
            @"gpu_ms":gpuTimes,
            @"median_gpu_ms":@(median(samples)),
            @"max_command_gpu_ms":@(maxCommand),
            @"different_records":@(differences),
            @"fault_records":@(faults),
            @"checked_records":@(checkedRecords),
            @"record_bytes":@(sizeof(ExactSurface))
        }];
    }
    return @{
        @"pass":@(pass),
        @"records_per_run":@(count),
        @"warmup_runs":@(warmupRuns),
        @"timed_runs":@(timedRuns),
        @"prepare_thread_execution_width":@(prepareWidth),
        @"prepare_max_total_threads_per_threadgroup":@(prepareMaximum),
        @"march_thread_execution_width":@(marchWidth),
        @"preparation":preparationTrials,
        @"march":marchTrials,
        @"note":@"One warmup and three timed runs per configuration. Every run, including warmup, compares all 96 ray bytes or all 48 surface bytes and checks faults. Counts sum all four runs. Medians exclude warmup; GPU timestamps exclude host comparisons. These stage trials are outside complete-render timings. Inputs are GPU-generated; no CPU per-pixel results are uploaded."
    };
}
