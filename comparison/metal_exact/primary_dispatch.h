#pragma once
#include "scene_abi.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <functional>
#include <utility>
#include <vector>

#include "metal_device.h"
using mb3d::metal_exact::gpu::runScene;

static void checkPrimaryDispatchRecords(id<MTLBuffer> buffer,
                                        const std::vector<ExactPixel>& expected,
                                        uint64_t& differences, uint64_t& faults) {
    require(buffer.length == expected.size() * sizeof(ExactPixel),
            "primary dispatch comparison record count mismatch");
    const auto* actual = static_cast<const ExactPixel*>(buffer.contents);
    for (size_t i = 0; i < expected.size(); ++i) {
        differences += std::memcmp(&actual[i], &expected[i], sizeof(ExactPixel)) != 0;
        faults += actual[i].fault != 0;
    }
}

// Same-kernel baseline and dispatch trials are additional GPU consistency
// checks. Independent production CPU geometry and final-image gates remain
// mandatory. All per-pixel inputs here originate on GPU; the host only reads.
static NSDictionary* primaryDispatchChecks(
    Metal& m, id<MTLComputePipelineState> pipeline, const ExactScene& scene,
    id<MTLBuffer> cols, id<MTLBuffer> rows, id<MTLBuffer> currentOutput,
    const std::vector<ExactPixel>& expected, unsigned selectedRows,
    unsigned selectedThreads, bool sweep, double maxMeasuredCommandMs,
    std::vector<ExactPixel>& baseline, unsigned selectedTileWidth = 8,
    const std::function<void(id<MTLBuffer>, const ExactScene&)>& complete = {}) {
    require(cols != nil && rows != nil && currentOutput != nil && pipeline != nil,
            "primary dispatch diagnostics require allocated buffers and pipeline");
    require(cols.length % sizeof(ExactTrig) == 0 &&
            rows.length % sizeof(ExactTrig) == 0,
            "primary dispatch diagnostic camera allocation mismatch");
    const size_t width = cols.length / sizeof(ExactTrig);
    const size_t height = rows.length / sizeof(ExactTrig);
    require(width > 0 && width <= 4096 && height > 0 && height <= 4096 &&
            scene.width == width && scene.height == height,
            "primary dispatch diagnostic dimensions mismatch");
    const size_t count = width * height;
    require(expected.size() == count &&
            currentOutput.length == count * sizeof(ExactPixel),
            "primary dispatch diagnostic output dimensions mismatch");
    require(currentOutput != cols && currentOutput != rows,
            "primary dispatch diagnostic output aliases camera allocations");
    const NSUInteger simdWidth = pipeline.threadExecutionWidth;
    const NSUInteger maxThreads = pipeline.maxTotalThreadsPerThreadgroup;
    require(selectedRows > 0 && selectedRows <= 4096 && simdWidth > 0 &&
            selectedThreads > 0 && selectedThreads % simdWidth == 0 &&
            selectedThreads <= maxThreads,
            "invalid selected primary dispatch settings");
    const unsigned selectedTile = selectedTileWidth == 0 ? selectedThreads : selectedTileWidth;
    require(selectedTile > 0 && selectedTile <= selectedThreads &&
            selectedThreads % selectedTile == 0,
            "invalid selected primary tile width");
    const unsigned selectedTileHeight = selectedThreads / selectedTile;
    require(std::isfinite(maxMeasuredCommandMs) && maxMeasuredCommandMs > 0,
            "missing measured primary command timestamp");

    auto output = m.buffer(count * sizeof(ExactPixel));
    std::memset(output.contents, 0xa5, output.length);
    runScene(m, pipeline, output, cols, rows, scene, 8, 32, nullptr, 0);
    if (complete) complete(output, scene);
    baseline.resize(count);
    std::memcpy(baseline.data(), output.contents, output.length);
    uint64_t baselineDifferences = 0, baselineFaults = 0;
    checkPrimaryDispatchRecords(output, expected, baselineDifferences, baselineFaults);
    for (size_t i = 0; i < count; ++i)
        baselineFaults += expected[i].fault != 0 && baseline[i].fault == 0;
    bool pass = baselineDifferences == 0 && baselineFaults == 0;

    uint64_t negativeFailures = 0;
    ExactPixel invalid{};
    invalid.fault = 1;
    for (unsigned control = 0; control < 4; ++control) {
        ExactScene bad = scene;
        if (control == 0) ++bad.width;
        else if (control == 1) ++bad.height;
        else if (control == 2) --bad.width;
        else --bad.height;
        std::memset(output.contents, 0xa5, output.length);
        runScene(m, pipeline, output, cols, rows, bad, selectedRows, selectedThreads,
                 nullptr, selectedTileWidth);
        if (complete) complete(output, bad);
        const auto* actual = static_cast<const ExactPixel*>(output.contents);
        for (size_t i = 0; i < count; ++i)
            negativeFailures += std::memcmp(&actual[i], &invalid, sizeof(invalid)) != 0;
    }
    pass &= negativeFailures == 0;

    NSMutableDictionary* report = [@{
        @"rows_per_command":@(selectedRows),
        @"threads_per_threadgroup":@(selectedThreads),
        @"tile_width":@(selectedTile),
        @"tile_height":@(selectedTileHeight),
        @"commands_per_frame":@((height + selectedRows - 1) / selectedRows),
        @"grid_width":@(width),
        @"grid_height":@(std::min<size_t>(selectedRows, height)),
        @"thread_execution_width":@(simdWidth),
        @"max_total_threads_per_threadgroup":@(maxThreads),
        @"max_command_gpu_ms":@(maxMeasuredCommandMs),
        @"baseline":@{
            @"rows_per_command":@8,
            @"threads_per_threadgroup":@32,
            @"tile_width":@32,
            @"tile_height":@1,
            @"pixels":@(count),
            @"different_pixels":@(baselineDifferences),
            @"fault_pixels":@(baselineFaults),
            @"gate":@"all 176 bytes exact (same-kernel eight-row GPU baseline)"
        },
        @"negative_controls":@{
            @"cases":@4,
            @"checks":@(uint64_t(count) * 4),
            @"failures":@(negativeFailures),
            @"pass":@(negativeFailures == 0)
        }
    } mutableCopy];

    if (sweep) {
        constexpr unsigned warmupRuns = 1, timedRuns = 3;
        constexpr unsigned totalRuns = warmupRuns + timedRuns;
        std::vector<std::array<unsigned, 3>> configurations;
        auto append = [&](unsigned batch, unsigned group, unsigned tile) {
            if (group % simdWidth != 0 || group > maxThreads) return;
            if (tile == 0 || tile > group || group % tile != 0) return;
            const std::array<unsigned, 3> config{batch, group, tile};
            if (std::find(configurations.begin(), configurations.end(), config) ==
                configurations.end()) configurations.push_back(config);
        };
        for (unsigned batch : {8u, 32u, 128u, 256u, unsigned(height)})
            append(batch, 32, 32);
        for (unsigned group : {64u, 128u, 256u})
            append(selectedRows, group, group);
        for (unsigned tile : {16u, 8u, 4u})
            append(selectedRows, 32, tile);
        require(!configurations.empty(), "no legal primary dispatch trial configuration");
        NSMutableArray* trials = [NSMutableArray array];
        for (const auto& [batch, group, tile] : configurations) {
            std::vector<double> samples;
            NSMutableArray* gpuTimes = [NSMutableArray array];
            uint64_t differences = 0, faults = 0;
            double warmup = 0, maxCommand = 0;
            for (unsigned run = 0; run < totalRuns; ++run) {
                // Poison and validation stay outside the GPU timestamp interval.
                std::memset(output.contents, 0xa5, output.length);
                const double ms = runScene(m, pipeline, output, cols, rows, scene,
                                           batch, group, &maxCommand, tile);
                // Sweep timestamps isolate geometry dispatch. Normal work is
                // invariant across these configurations and validated after it.
                if (complete) complete(output, scene);
                checkPrimaryDispatchRecords(output, expected, differences, faults);
                if (run == 0) warmup = ms;
                else {
                    samples.push_back(ms);
                    [gpuTimes addObject:@(ms)];
                }
            }
            pass &= differences == 0 && faults == 0;
            [trials addObject:@{
                @"rows_per_command":@(batch),
                @"threads_per_threadgroup":@(group),
                @"tile_width":@(tile),
                @"tile_height":@(group / tile),
                @"gpu_ms":gpuTimes,
                @"median_gpu_ms":@(median(samples)),
                @"warmup_gpu_ms":@(warmup),
                @"max_command_gpu_ms":@(maxCommand),
                @"different_records":@(differences),
                @"fault_records":@(faults),
                @"checked_records":@(uint64_t(count) * totalRuns),
                @"record_bytes":@(sizeof(ExactPixel))
            }];
        }
        report[@"trials"] = @{
            @"warmup_runs":@(warmupRuns),
            @"timed_runs":@(timedRuns),
            @"records_per_run":@(count),
            @"configurations":trials
        };
    }
    report[@"pass"] = @(pass);
    return report;
}
