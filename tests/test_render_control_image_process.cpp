#include "core/image_process.h"
#include "core/render_control.h"

#include <cstdint>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <vector>

using namespace mb3d;

namespace {
int checks = 0;

void check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}

template<class F> void stopped(F&& fn, RenderStatus expected) {
    bool caught = false;
    try { fn(); }
    catch (const RenderStopped& error) { caught = error.status() == expected; }
    check(caught, "wrong or absent image-process terminal reason");
}

constexpr int width = 36;
constexpr int height = 24;

std::vector<uint8_t> sourceImage() {
    std::vector<uint8_t> pixels(width * height * 3);
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            for (int c = 0; c < 3; ++c) {
                pixels[(y * width + x) * 3 + c] =
                    static_cast<uint8_t>((x * 31 + y * 17 + c * 73 + x * y) & 255);
            }
        }
    }
    return pixels;
}

FinalRGBSurface process(const std::vector<uint8_t>& input, int scale, int sharpen,
                        int crop, RenderControl* control) {
    if (control) control->start();
    ScopedRenderControl binding(control);
    FinalRGBSurface output;
    check(BuildFinalRGBSurface(input.data(), width, height, scale, sharpen, crop, output),
          "valid final image failed");
    if (control) control->complete();
    return output;
}

void equalSurface(const FinalRGBSurface& expected, const FinalRGBSurface& actual) {
    check(expected.width == actual.width && expected.height == actual.height,
          "render control changed final image dimensions");
    check(expected.pixels == actual.pixels, "render control changed final RGB bytes");
}

void finiteControls(const std::vector<uint8_t>& input) {
    // Work-v1 counts each AA horizontal/vertical output, each box output and
    // input tap, and each copied/cropped pixel. This checks the production
    // kernel hooks against counts derived from dimensions, not an earlier run.
    const uint64_t expectedWork[] = {1312, 760, 444, 946};
    for (int scale = 1; scale <= 4; ++scale) {
        for (int sharpen = 0; sharpen <= 3; ++sharpen) {
            const auto baseline = process(input, scale, sharpen, 4, nullptr);
            RenderControl unlimited;
            equalSurface(baseline, process(input, scale, sharpen, 4, &unlimited));
            check(unlimited.status() == RenderStatus::Completed &&
                      unlimited.work_used() == expectedWork[scale - 1],
                  "image work event count or completion differs from the contract");

            RenderControl ample(100000);
            equalSurface(baseline, process(input, scale, sharpen, 4, &ample));
            check(ample.work_used() == unlimited.work_used(), "finite budget changed work count");

            RenderControl exact(expectedWork[scale - 1]);
            equalSurface(baseline, process(input, scale, sharpen, 4, &exact));
            check(exact.status() == RenderStatus::Completed &&
                      exact.work_used() == exact.max_work(),
                  "image cannot complete at its exact work limit");

            RenderControl shortBudget(expectedWork[scale - 1] - 1);
            stopped([&] { process(input, scale, sharpen, 4, &shortBudget); },
                    RenderStatus::WorkExhausted);
            check(shortBudget.work_used() <= shortBudget.max_work() &&
                      shortBudget.status() == RenderStatus::WorkExhausted,
                  "image exhausted budget overshot or lost its reason");
            check(current_render_control == nullptr, "image exception leaked its control binding");
        }
    }
}

void stopsAndRecovery(const std::vector<uint8_t>& input) {
    for (int scale = 1; scale <= 4; ++scale) {
        RenderControl cancelled;
        cancelled.cancel();
        stopped([&] { process(input, scale, 1, 4, &cancelled); }, RenderStatus::Cancelled);
        check(cancelled.work_used() == 0, "pre-cancelled image admitted work");
    }

    // The limit interrupts the real AA horizontal pass after output allocation.
    // ASan/LSan runs additionally verify that this unwind releases the raw-API
    // result through SdoAA's internal owner before it can be returned.
    for (int scale : {2, 3}) {
        RenderControl limited(17);
        limited.start();
        {
            ScopedRenderControl binding(&limited);
            stopped([&] {
                int outWidth = 0, outHeight = 0;
                std::unique_ptr<uint8_t[]> image(SdoAA(input.data(), width, height, 3,
                    scale, 2, &outWidth, &outHeight));
            }, RenderStatus::WorkExhausted);
        }
        check(limited.work_used() == 17, "AA did not stop at its exact admitted work limit");
        check(current_render_control == nullptr, "AA unwind retained stopped control");

        RenderControl recovery;
        equalSurface(process(input, scale, 2, 0, nullptr),
                     process(input, scale, 2, 0, &recovery));
        check(recovery.status() == RenderStatus::Completed, "AA did not recover with a fresh control");
    }
}
} // namespace

int main() {
    try {
        const auto input = sourceImage();
        finiteControls(input);
        stopsAndRecovery(input);
        std::printf("render_control_image_process: %d checks passed\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "render_control_image_process: %s after %d checks\n", error.what(), checks);
        return 1;
    }
}
