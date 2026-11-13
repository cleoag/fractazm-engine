#include "core/external_formulas.h"
#include "core/render_control.h"
#include <array>
#include <bit>
#include <climits>
#include <cstdio>
#include <stdexcept>

// Instantiate the actual portable production bodies and their real local work
// guard. No formula implementation or numerical helper is replaced here.
// These exact finite controls do not qualify x87/SSE or full-render parity.
namespace mb3d {
struct FormulaInfo {
    unsigned int id;
    const char* name;
    int param_count;
    int complexity;
    bool uses_transcendental;
};
#define constant
#define MB3D_FORCE_PORTABLE_FORMULA_LOOPS
#include "../external_formulas/Formulas/3D/_e_3Dtess.inc"
#include "../external_formulas/Formulas/3D/_IFS_tess.inc"
#include "../external_formulas/Formulas/dIFS/Knots2IFS.inc"
#undef MB3D_FORCE_PORTABLE_FORMULA_LOOPS
#undef constant
} // namespace mb3d

namespace {
using Params = std::array<double, mb3d::MAX_FORMULA_PARAMS>;
using Body = mb3d::cpu_double3 (*)(mb3d::cpu_double3, double*, double&);
int checks = 0;

void check(bool value, const char* message) {
    ++checks;
    if (!value) throw std::runtime_error(message);
}

Params e3Params(int count) {
    Params p{};
    p[3] = p[4] = p[5] = 1;
    p[7] = count;
    p[9] = 2;
    p[12] = -1;
    return p;
}

Params ifsParams(int count) {
    Params p{};
    p[3] = p[5] = p[10] = 1;
    p[7] = count;
    p[9] = .5;
    p[11] = M_PI / 180.;
    p[12] = 500;
    return p;
}

Params knotsParams(int count) {
    Params p{};
    p[0] = p[1] = p[7] = p[8] = p[9] = 1;
    p[11] = count;
    p[13] = 2 * M_PI;
    p[14] = 1e20;
    p[mb3d::DIFS_VARYSCALE_SLOT] = 1;
    return p;
}

struct Result {
    mb3d::cpu_double3 xyz;
    double de = 123;
    Params params;
};

Result run(Body body, mb3d::cpu_double3 input, Params params,
           mb3d::RenderControl* control) {
    mb3d::ScopedRenderControl binding(control);
    if (control) {
        control->start();
        control->consume(7); // Simulate already admitted caller work, not loop steps.
    }
    Result result{input, 123, params};
    result.xyz = body(input, result.params.data(), result.de);
    if (control) {
        // Direct body calls poll; real dispatchers account for formula calls.
        check(control->work_used() == 7, "portable body charged a second global work event");
        control->complete();
    }
    return result;
}

bool sameBits(double a, double b) {
    return std::bit_cast<uint64_t>(a) == std::bit_cast<uint64_t>(b);
}

void sameResult(const Result& a, const Result& b) {
    check(sameBits(a.xyz.x, b.xyz.x) && sameBits(a.xyz.y, b.xyz.y) &&
          sameBits(a.xyz.z, b.xyz.z) && sameBits(a.de, b.de),
          "render control changed portable formula result bits");
    for (size_t i = 0; i < a.params.size(); ++i)
        check(sameBits(a.params[i], b.params[i]), "render control changed formula writeback bits");
}

void finiteControl(Body body, mb3d::cpu_double3 input, const Params& params,
                   mb3d::cpu_double3 expected, double expectedDE) {
    const auto baseline = run(body, input, params, nullptr);
    check(baseline.xyz.x == expected.x && baseline.xyz.y == expected.y &&
          baseline.xyz.z == expected.z && baseline.de == expectedDE,
          "portable finite control changed its known exact result");
    for (double value : baseline.params)
        check(std::isfinite(value), "portable finite control produced a nonfinite parameter");
    mb3d::RenderControl unlimited;
    mb3d::RenderControl ample(100);
    sameResult(baseline, run(body, input, params, &unlimited));
    sameResult(baseline, run(body, input, params, &ample));
    check(unlimited.status() == mb3d::RenderStatus::Completed &&
          ample.status() == mb3d::RenderStatus::Completed, "finite formula control did not complete");
}

void exhaustionAndCancellation(Body body, mb3d::cpu_double3 input, const Params& params) {
    {
        mb3d::ScopedRenderControl binding(nullptr);
        auto work = params;
        double de = 123;
        bool exhausted = false;
        try { body(input, work.data(), de); }
        catch (const mb3d::FormulaWorkError& error) {
            exhausted = error.steps() == mb3d::FormulaWorkPolicy::maxInnerStepsPerCall &&
                        error.limit() == mb3d::FormulaWorkPolicy::maxInnerStepsPerCall;
        }
        check(exhausted, "real portable body did not retain its exact local work limit");
    }
    mb3d::RenderControl control;
    control.start();
    control.cancel();
    mb3d::ScopedRenderControl binding(&control);
    auto work = params;
    double de = 123;
    mb3d::cpu_double3 published(91, 92, 93);
    bool stopped = false;
    try { published = body(input, work.data(), de); }
    catch (const mb3d::RenderStopped& error) {
        stopped = error.status() == mb3d::RenderStatus::Cancelled;
    }
    check(stopped, "portable body failed to poll cancellation before a loop step");
    check(published.x == 91 && published.y == 92 && published.z == 93,
          "cancelled portable callback returned a partial point");
    check(control.work_used() == 0, "cancelled direct body charged caller work");
}
} // namespace

int main() {
    try {
        const Body e3 = mb3d::execute_formula<mb3d::FORMULA_E3DTESS>;
        const Body ifs = mb3d::execute_formula<mb3d::FORMULA_IFSTESS>;
        const Body knots = mb3d::execute_formula<mb3d::FORMULA_KNOTS2IFS>;
        for (int count : {INT_MIN, -1, 0, 1, 8}) {
            finiteControl(e3, {0, 0, 0}, e3Params(count), {0, 0, 0}, 123);
            finiteControl(knots, {1, 0, 0}, knotsParams(count), {1, 0, 0}, 1);
        }
        for (int count : {0, 1, 7})
            finiteControl(ifs, {2, 0, 0}, ifsParams(count), {1000, 0, 0}, 123);
        auto early = e3Params(INT_MAX);
        early[8] = .5;
        finiteControl(e3, {1.5, 0, 0}, early, {0, 1, 1}, 123);
        finiteControl(ifs, {0, 0, 0}, ifsParams(INT_MAX), {0, 0, 0}, 123);
        exhaustionAndCancellation(e3, {0, 0, 0}, e3Params(INT_MAX));
        exhaustionAndCancellation(ifs, {2, 0, 0}, ifsParams(INT_MAX));
        exhaustionAndCancellation(knots, {1, 0, 0}, knotsParams(INT_MAX));
        std::printf("Portable formula control: %d checks, 15 finite cases, 3 real bodies; no x87 parity claim\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Portable formula control failed after %d checks: %s\n", checks, error.what());
        return 1;
    }
}
