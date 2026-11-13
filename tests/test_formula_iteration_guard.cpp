#include "core/external_formulas.h"
#include "core/formula_iteration_guard.h"
#include "core/formulas.h"
#include "core/header_trafos.h"
#include "core/ray_marcher.h"
#include "core/thread_pool.h"

#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>

#include "fixtures/interpolation_setup.h"
#include "fixtures/hybrid_layout.h"

namespace {

using namespace mb3d;
constexpr uint32_t kBudget = FormulaIterationPolicy::maxCallsWithoutProgress;
int scenarios = 0;
ThybridIteration2 conditionalRadius = nullptr;
ThybridIteration2 conditionalSwitch = nullptr;
ThybridIteration2 conditionalSwitch2 = nullptr;

void check(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}

TIteration3Dext& extended(void* raw) {
    return *reinterpret_cast<TIteration3Dext*>(static_cast<char*>(raw) -
                                              offsetof(TIteration3Dext, C1));
}

int32_t uncounted(uint32_t repetitions) {
    const uint32_t bits = repetitions | 0x80000000u;
    int32_t result;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

// Ju1/Ju2 are unused by these non-Julia tests; they record actual dispatches,
// independently of ItResultI (IFS returns the iteration of the minimum DE).
void record(double&, double&, double&, double&, void* raw) {
    auto& ext = extended(raw);
    ++ext.Ju1;
    ext.Rout = 1.0;
}

void counted(double& x, double& y, double& z, double& w, void* raw) {
    record(x, y, z, w, raw);
    ++extended(raw).Ju2;
}

void disableAll(double& x, double& y, double& z, double& w, void* raw) {
    record(x, y, z, w, raw);
    for (auto& count : extended(raw).nHybrid) count = 0;
}

void resetCounter(double& x, double& y, double& z, double& w, void* raw) {
    record(x, y, z, w, raw);
    extended(raw).ItResultI = -1;
}

void runConditionalRadius(double& x, double& y, double& z, double& w, void* raw) {
    record(x, y, z, w, raw);
    conditionalRadius(x, y, z, w, raw);
}

void runConditionalSwitch(double& x, double& y, double& z, double& w, void* raw) {
    record(x, y, z, w, raw);
    conditionalSwitch(x, y, z, w, raw);
}

TIteration3Dext initialState() {
    TIteration3Dext ext{};
    ext.C1 = ext.x = 1.0;
    ext.maxIt = 2;
    ext.RStop = 256.0f;
    ext.RStopD = -1.0;
    ext.bIsInsideRender = -1;
    ext.nHybrid[0] = 1;
    for (auto& callback : ext.fHybrid) callback = record;
    for (int i = 0; i < 4; ++i) ext.SMatrix4[i][i] = 1.0f;
    return ext;
}

constexpr std::array<const char*, 11> driverNames = {
    "doHybridPas", "doHybridPasDE", "doHybrid4DPas", "doHybrid4DDEPas",
    "doHybridIFS3D", "doHybridIFS3DnoVecIni", "do3DalternateOTrap",
    "doInterpolHybridPas", "doInterpolHybridPasDE",
    "doInterpolHybridPas4D", "doInterpolHybridPas4DDE"
};

void run(int driver, TIteration3Dext& ext) {
    auto* it = reinterpret_cast<TPIteration3D>(&ext.C1);
    switch (driver) {
    case 0: doHybridPas(it); break;
    case 1: doHybridPasDE(it); break;
    case 2: doHybrid4DPas(it); break;
    case 3: doHybrid4DDEPas(it); break;
    case 4: doHybridIFS3D(it); break;
    case 5: doHybridIFS3DnoVecIni(it); break;
    case 6: do3DalternateOTrap(it); break;
    case 7: doInterpolHybridPas(it); break;
    case 8: doInterpolHybridPasDE(it); break;
    case 9: doInterpolHybridPas4D(it); break;
    case 10: doInterpolHybridPas4DDE(it); break;
    default: throw std::runtime_error("invalid test driver");
    }
}

void expectFailure(int driver, TIteration3Dext& ext, FormulaIterationFailure failure) {
    try {
        run(driver, ext);
    } catch (const FormulaIterationError& error) {
        check(error.failure() == failure, "wrong iteration failure category");
        const std::string message = error.what();
        check(message.find(driverNames[driver]) != std::string::npos,
              "iteration diagnostic omits the production driver");
        check(message.find("slot=") != std::string::npos &&
              message.find("ItResultI=") != std::string::npos &&
              message.find("limit=" + std::to_string(kBudget)) != std::string::npos,
              "iteration diagnostic omits state or policy");
        ++scenarios;
        return;
    }
    throw std::runtime_error("invalid hybrid chain silently succeeded");
}

struct Parameters {
    alignas(double) std::array<char, 128> bytes{};

    void* center() { return bytes.data() + 64; }
    template<typename T> void put(int offset, T value) {
        std::memcpy(bytes.data() + 64 + offset, &value, sizeof(value));
    }
};

void testOrdinaryDrivers() {
    for (int driver = 0; driver < 7; ++driver) {
        auto ext = initialState();
        ext.nHybrid[0] = 0;
        ext.EndTo = 2;
        if (driver == 0) {
            run(driver, ext);
            check(ext.ItResultI == ext.maxIt,
                  "Pascal empty start wrap did not return as inside");
            ++scenarios;
        } else {
            expectFailure(driver, ext, FormulaIterationFailure::EmptyCycle);
        }
        check(ext.Ju1 == 0, "empty cycle executed a callback");

        // The repeat range excludes the initial active slot. Looking only for
        // a return to iStartFrom fails to detect this empty cycle.
        ext = initialState();
        ext.EndTo = 2;
        ext.iRepeatFrom = 1;
        expectFailure(driver, ext, FormulaIterationFailure::EmptyCycle);
        check(ext.Ju1 == 1, "empty repeated suffix skipped the initial prefix");

        ext = initialState();
        ext.nHybrid[0] = uncounted(1);
        ext.fHybrid[0] = disableAll;
        if (driver == 0) {
            run(driver, ext);
            check(ext.ItResultI == ext.maxIt,
                  "Pascal dynamically disabled start did not return as inside");
            ++scenarios;
        } else {
            expectFailure(driver, ext, FormulaIterationFailure::EmptyCycle);
        }
        check(ext.Ju1 == 1, "dynamic empty cycle was not detected after one callback");

        for (int field = 0; field < 3; ++field) {
            ext = initialState();
            if (field == 0) ext.iStartFrom = 6;
            if (field == 1) ext.iRepeatFrom = 6;
            if (field == 2) ext.EndTo = 6;
            expectFailure(driver, ext, FormulaIterationFailure::InvalidSlotRange);
            check(ext.Ju1 == 0, "invalid slot range executed a callback");
        }

        ext = initialState();
        ext.nHybrid[0] = uncounted(1);
        expectFailure(driver, ext, FormulaIterationFailure::NoProgressBudget);
        check(ext.Ju1 == kBudget, "uncounted cycle has an incorrect dispatch bound");

        // Real finite runs beyond the old 16-bit/65,535 scale must work.
        // The complete invocation exceeds the policy limit, but each segment
        // between counted iterations is below it and must reset the budget.
        constexpr uint32_t longRepeat = 100003;
        ext = initialState();
        ext.EndTo = 5;
        ext.maxIt = 3;
        for (int slot = 0; slot < 5; ++slot) ext.nHybrid[slot] = uncounted(longRepeat);
        ext.nHybrid[5] = 1;
        ext.fHybrid[5] = counted;
        run(driver, ext);
        check(ext.Ju1 == (5.0 * longRepeat + 1.0) * 3.0 && ext.Ju2 == 3,
              "long valid hybrid chain was truncated or miscounted");
        ++scenarios;

        // Initially there is no counted slot. The actual registered conditional
        // formula enables its partner later; static 'no counted slot' rejection
        // would incorrectly reject this finite Pascal-compatible chain.
        Parameters parameters;
        parameters.put<int32_t>(-12, 100);
        parameters.put<int32_t>(-16, 2);
        parameters.put<int32_t>(-20, 0);
        parameters.put<int32_t>(-24, 1);
        parameters.put<int32_t>(-28, 1);
        ext = initialState();
        ext.EndTo = 1;
        ext.nHybrid[0] = uncounted(1);
        ext.nHybrid[1] = 0;
        ext.fHybrid[0] = runConditionalSwitch;
        ext.fHybrid[1] = counted;
        ext.fHPVar[0] = parameters.center();
        run(driver, ext);
        check(ext.Ju1 == 103 && ext.Ju2 == 2 && ext.bFirstIt == 101,
              "conditional formula could not enable a delayed counted partner");
        ++scenarios;
    }
}

void testDisabledStartCompatibility() {
    Parameters parameters;
    parameters.put<double>(-16, 1.0);
    parameters.put<double>(-24, 4.0);
    parameters.put<int32_t>(-28, 1); // The real conditional disables itself.
    parameters.put<int32_t>(-32, 0);
    parameters.put<int32_t>(-36, 1);
    parameters.put<int32_t>(-40, 2);
    auto ext = initialState();
    ext.EndTo = 5;
    ext.maxIt = 60;
    ext.CalcSIT = 1;
    ext.SmoothItD = 123.25f;
    ext.nHybrid[0] = uncounted(1);
    ext.fHybrid[0] = runConditionalRadius;
    ext.fHPVar[0] = parameters.center();
    for (int slot = 1; slot < 6; ++slot) {
        ext.nHybrid[slot] = 1;
        ext.fHybrid[slot] = counted;
    }
    run(0, ext);
    check(ext.ItResultI == 60 && ext.Ju1 == 6 && ext.Ju2 == 5 &&
              ext.nHybrid[0] == uncounted(0) && ext.SmoothItD == 123.25f,
          "disabled start lost the Pascal finite inside-return convention");
    ++scenarios;

    ext = initialState();
    ext.EndTo = 1;
    ext.maxIt = 60;
    ext.nHybrid[0] = 0;
    ext.nHybrid[1] = 1;
    ext.fHybrid[1] = counted;
    run(0, ext);
    check(ext.ItResultI == 60 && ext.Ju2 == 1,
          "initially disabled start did not return only after wrapping");
    ++scenarios;

    ext = initialState();
    ext.EndTo = 1;
    ext.iRepeatFrom = 1;
    ext.maxIt = 60;
    ext.nHybrid[0] = 0;
    ext.nHybrid[1] = 1;
    ext.fHybrid[1] = counted;
    run(0, ext);
    check(ext.ItResultI == 60 && ext.Ju2 == 60,
          "disabled prefix changed a repeat suffix that never reaches it");
    ++scenarios;
}

void testConditionalAllDisabledExit() {
    // Original M3F _CondItcountSwX2 parameters from the unchanged
    // challenge__CondItcountSwX2_p6_zero scene. At execution 1 pass A
    // disables slot 2, while pass B clamps formula number 0 to slot 1
    // and disables itself. Immutable formulas.pas:3465-3469 returns
    // ItResultI=maxIt on the following wrap, without smooth iteration.
    Parameters parameters;
    const int32_t values[] = {1, 2, 2, 0, 1, 3, 0, 0, 3, 1};
    for (int i = 0; i < 10; ++i) parameters.put<int32_t>(-12 - i * 4, values[i]);
    for (int end : {1, 5}) {
        auto ext = initialState();
        ext.EndTo = end;
        ext.maxIt = 60;
        ext.CalcSIT = 1;
        ext.SmoothItD = 123.25f;
        ext.nHybrid[0] = uncounted(1);
        ext.nHybrid[1] = 1;
        ext.fHybrid[0] = conditionalSwitch2;
        ext.fHybrid[1] = counted;
        ext.fHPVar[0] = parameters.center();
        run(0, ext);
        check(ext.ItResultI == 60 && ext.bFirstIt == 1 && ext.Ju2 == 0 &&
                  ext.nHybrid[0] == uncounted(0) && ext.nHybrid[1] == 0 &&
                  ext.SmoothItD == 123.25f,
              "all-disabled conditional lost the original finite inside exit");
        ++scenarios;

        // The same state has no finite inside exit when its repeat range
        // excludes the start. Preserve the explicit nontermination error.
        ext = initialState();
        ext.EndTo = end;
        ext.iRepeatFrom = 1;
        ext.nHybrid[0] = uncounted(1);
        ext.nHybrid[1] = 1;
        ext.fHybrid[0] = conditionalSwitch2;
        ext.fHPVar[0] = parameters.center();
        expectFailure(0, ext, FormulaIterationFailure::EmptyCycle);
        check(ext.bFirstIt == 1, "conditional repeat-suffix guard skipped execution");
    }
}

void testDynamicAndHugeCounts() {
    for (int driver : {0, 4}) {
        // A real conditional formula repeatedly disables an initially counted
        // partner. The active conditional itself means this is not an empty
        // slot-selection cycle; it must fail by the bounded-work policy.
        Parameters parameters;
        parameters.put<double>(-16, 1.0);
        parameters.put<double>(-24, 2.0);
        parameters.put<int32_t>(-28, 2);
        parameters.put<int32_t>(-32, 0);
        parameters.put<int32_t>(-36, 0);
        parameters.put<int32_t>(-40, 0);
        auto ext = initialState();
        ext.EndTo = 1;
        ext.nHybrid[0] = uncounted(1);
        ext.nHybrid[1] = 1;
        ext.fHybrid[0] = runConditionalRadius;
        ext.fHybrid[1] = counted;
        ext.fHPVar[0] = parameters.center();
        expectFailure(driver, ext, FormulaIterationFailure::NoProgressBudget);
        check(ext.Ju1 == kBudget && ext.Ju2 == 0 && ext.nHybrid[1] == 0,
              "conditional disabling of counted partner escaped the budget");

        ext = initialState();
        ext.nHybrid[0] = uncounted(std::numeric_limits<int32_t>::max());
        expectFailure(driver, ext, FormulaIterationFailure::NoProgressBudget);
        check(ext.Ju1 == kBudget, "huge repeat count expanded the fixed policy limit");

        ext = initialState();
        ext.fHybrid[0] = resetCounter;
        expectFailure(driver, ext, FormulaIterationFailure::NoProgressBudget);
        check(ext.Ju1 == kBudget, "counter reset defeated the high-water progress check");
    }
}

void testInterpolatingDrivers() {
    for (int driver = 7; driver < 11; ++driver) {
        auto ext = initialState();
        const float weight = 0.5f;
        std::memcpy(&ext.nHybrid[0], &weight, sizeof(weight));
        std::memcpy(&ext.nHybrid[1], &weight, sizeof(weight));
        ext.maxIt = 8;
        run(driver, ext);
        check(ext.Ju1 == 16 && ext.ItResultI == 8,
              "normal interpolation did not run both callbacks per iteration");
        ++scenarios;

        ext.Ju1 = 0;
        ext.fHybrid[0] = ext.fHybrid[1] = resetCounter;
        expectFailure(driver, ext, FormulaIterationFailure::NoProgressBudget);
        check(ext.Ju1 == kBudget, "interpolation counter reset escaped the budget");
    }
}

void testPolicyBoundaries() {
    auto ext = initialState();
    ext.EndTo = 1;
    ext.maxIt = 1;
    ext.nHybrid[0] = uncounted(kBudget - 1);
    ext.nHybrid[1] = 1;
    ext.fHybrid[1] = counted;
    run(0, ext);
    check(ext.Ju1 == kBudget && ext.Ju2 == 1,
          "a completing dispatch at the exact policy boundary was rejected");
    ++scenarios;

    ext = initialState();
    ext.EndTo = 1;
    ext.maxIt = 1;
    ext.nHybrid[0] = uncounted(kBudget);
    ext.nHybrid[1] = 1;
    ext.fHybrid[1] = counted;
    expectFailure(0, ext, FormulaIterationFailure::NoProgressBudget);
    check(ext.Ju1 == kBudget && ext.Ju2 == 0,
          "policy did not reject the first dispatch beyond its explicit limit");

    ext = initialState();
    ext.maxIt = static_cast<int32_t>(kBudget + 1);
    run(0, ext);
    check(ext.Ju1 == kBudget + 1 && ext.ItResultI == ext.maxIt,
          "work policy silently clamped the scene's counted maxIt");
    ++scenarios;
}

void testWorkerPropagation() {
    ThreadPool pool(2);
    auto result = pool.submit([] {
        auto ext = initialState();
        ext.nHybrid[0] = 0;
        run(1, ext); // DE driver has no Pascal finite empty-start exit.
    });
    pool.wait_all();
    try {
        result.get();
    } catch (const FormulaIterationError& error) {
        check(error.failure() == FormulaIterationFailure::EmptyCycle,
              "worker changed the interpreter failure category");
        ++scenarios;
        return;
    }
    throw std::runtime_error("worker swallowed the interpreter exception");
}

TMandHeader10 headerForCountTests() {
    TMandHeader10 header{};
    header.Width = 32;
    header.Height = 24;
    header.Iterations = header.iMaxItsF2 = 20;
    header.MinimumIterations = 1;
    header.RStop = 16.0;
    header.dFOVy = 45.0;
    header.bCalc3D = 1;
    header.sDEstop = 0.001f;
    header.mZstepDiv = header.sRaystepLimiter = 1.0f;
    return header;
}

void expectHeaderFailure(TMandHeader10& header, THeaderCustomAddon& addon,
                         const char* diagnostic) {
    try {
        GetMCTparasFromHeader(header, &addon, 0);
    } catch (const std::invalid_argument& error) {
        check(std::string(error.what()).find(diagnostic) != std::string::npos,
              "header count rejection omitted its cause");
        ++scenarios;
        return;
    }
    throw std::runtime_error("unrepresentable raw header count silently succeeded");
}

void testHeaderCountValidation() {
    auto header = headerForCountTests();
    for (int mode : {0, 2, 3}) {
        for (int slot = 0; slot < 6; ++slot) {
            THeaderCustomAddon addon{};
            addon.bOptions1 = static_cast<uint8_t>(mode);
            addon.Formulas[slot].iItCount = std::numeric_limits<int32_t>::min();
            expectHeaderFailure(header, addon, "INT_MIN");
        }
    }

    // Interpolation stores float weights in iItCount. INT_MIN is -0.0f there
    // and must not be confused with an unrepresentable ordinary repeat count.
    for (int negativeZeroSlot : {0, 1}) {
        THeaderCustomAddon addon{};
        addon.bOptions1 = 1;
        addon.Formulas[negativeZeroSlot].iItCount = std::numeric_limits<int32_t>::min();
        addon.Formulas[1 - negativeZeroSlot].iItCount = 0x3f800000; // 1.0f
        const auto mct = GetMCTparasFromHeader(header, &addon, 0);
        check(mct.bMCTisValid != 0 &&
              mct.nHybrid[negativeZeroSlot] == std::numeric_limits<int32_t>::min(),
              "interpolation negative zero was rejected or lost");
        ++scenarios;
    }

    THeaderCustomAddon addon{};
    addon.Formulas[0].iItCount = -7;
    addon.Formulas[1].iItCount = 1;
    auto mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid != 0 && mct.nHybrid[0] == uncounted(7),
          "negative raw repeat count did not retain sign-bit magnitude encoding");
    ++scenarios;

    // Exercise both scale-only and w-DE scale/color accumulation branches.
    for (int formula : {0, 4}) {
        addon = {};
        addon.Formulas[0].iFnr = addon.Formulas[1].iFnr = formula;
        addon.Formulas[0].iItCount = std::numeric_limits<int32_t>::max();
        addon.Formulas[1].iItCount = 1;
        expectHeaderFailure(header, addon, "repeat-weight sum");
    }

    addon = {};
    addon.Formulas[0].iItCount = std::numeric_limits<int32_t>::max() - 1;
    addon.Formulas[1].iItCount = 1;
    mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid != 0, "exact INT_MAX aggregate count was rejected");
    ++scenarios;

    addon = {};
    addon.Formulas[0].iItCount = std::numeric_limits<int32_t>::max();
    mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid != 0 && mct.nHybrid[0] == std::numeric_limits<int32_t>::max(),
          "large representable single-slot count was clamped or rejected");
    ++scenarios;

    // Each DE-combination part has its own accumulator: a global sum limit
    // would needlessly reject two individually representable parts.
    addon = {};
    addon.bOptions1 = 2;
    addon.bHybOpt2 = 1;
    addon.Formulas[0].iItCount = addon.Formulas[1].iItCount =
        std::numeric_limits<int32_t>::max();
    mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid != 0 && mct.StartFrom2 == 1,
          "independently representable DE-combination sums were rejected");
    ++scenarios;
}

void testDirectHybridLayout() {
    // Expected bytes/fields come from extracted immutable Pascal source,
    // compiled separately. File-load GUI normalization is intentionally absent.
    for (const auto& fixture : kHybridLayoutFixtures) {
        auto header = headerForCountTests();
        THeaderCustomAddon addon{};
        for (int slot = 0; slot < 6; ++slot) {
            GetHAddOnFromInternFormula(&addon, 0, slot);
            addon.Formulas[slot].iItCount = fixture.counts[slot];
        }
        addon.bOptions1 = static_cast<uint8_t>(fixture.mode);
        addon.bHybOpt1 = static_cast<uint8_t>(fixture.opt1);
        addon.bHybOpt2 = static_cast<uint16_t>(fixture.opt2);
        const auto mct = GetMCTparasFromHeader(header, &addon, 0);
        check(addon.bOptions1 == fixture.mode &&
                  addon.bHybOpt1 == fixture.normalized1 &&
                  addon.bHybOpt2 == fixture.normalized2,
              "direct hybrid layout changed original mode or normalized option bytes");
        // GetMCT subsequently extends interpolation's first group to slot 1
        // (HeaderTrafos.pas:606); other modes retain the captured initial end.
        const int end = (fixture.mode & 3) == 1 ? 1 : fixture.layout[1];
        check(mct.StartFrom1 == fixture.layout[0] && mct.wEndTo == end &&
                  mct.RepeatFrom1 == fixture.layout[2] &&
                  mct.StartFrom2 == fixture.layout[3] &&
                  mct.iEnd2 == fixture.layout[4] &&
                  mct.RepeatFrom2 == fixture.layout[5],
              "direct GetMCT hybrid boundaries disagree with original Pascal");
        ++scenarios;
    }
}

void testHeaderIterationLimits() {
    auto header = headerForCountTests();
    THeaderCustomAddon addon{};
    addon.Formulas[0].iItCount = 1;
    header.Iterations = std::numeric_limits<int32_t>::max();
    expectHeaderFailure(header, addon, "maximum iteration limit");

    header = headerForCountTests();
    header.iMaxItsF2 = std::numeric_limits<int32_t>::max();
    auto mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid && mct.iMaxitF2 == header.Iterations,
          "unused second iteration limit was rejected");
    ++scenarios;

    addon.bOptions1 = 2;
    addon.bHybOpt2 = 1;
    addon.Formulas[1].iItCount = 1;
    expectHeaderFailure(header, addon, "maximum iteration limit");

    // FormulaType 6 uses the sum of both limits, with one extra unit in
    // the smooth-iteration denominator. Its last representable sum is valid.
    addon.bOptions3 = 5;
    addon.Formulas[1].iFnr = 20;
    std::memcpy(addon.Formulas[1].CustomFname, "SphereIFS", sizeof("SphereIFS"));
    header.Iterations = std::numeric_limits<int32_t>::max() - 21;
    header.iMaxItsF2 = 20;
    mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid && mct.FormulaType == 6,
          "representable combined iteration limit was rejected");
    ++scenarios;
    header.iMaxItsF2 = 21;
    expectHeaderFailure(header, addon, "combined maximum iteration limit");
    header.iMaxItsF2 = 22;
    expectHeaderFailure(header, addon, "combined maximum iteration limit");

    header.Iterations = header.iMaxItsF2 = std::numeric_limits<int32_t>::min();
    expectHeaderFailure(header, addon, "combined maximum iteration limit");

    addon = {};
    addon.Formulas[0].iItCount = 1;
    header = headerForCountTests();
    header.Iterations = -1;
    mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid && mct.iMaxIt == -1,
          "legacy negative iteration limit was changed");
    ++scenarios;
}

void testDEMixTypeValidation() {
    // Mix requires a dIFS second group. The first group may itself be dIFS;
    // min/max/difference/smooth combinations have no such restriction.
    for (int firstDifs : {0, 1}) {
        for (int secondDifs : {0, 1}) {
            for (int mode : {0, 2}) {
                for (int operation = 0; operation <= 6; ++operation) {
                    auto header = headerForCountTests();
                    header.sFmixPow = 2;
                    THeaderCustomAddon addon{};
                    GetHAddOnFromInternFormula(&addon, firstDifs ? 8 : 0, 0);
                    GetHAddOnFromInternFormula(&addon, secondDifs ? 8 : 0, 1);
                    addon.Formulas[0].iItCount = addon.Formulas[1].iItCount = 1;
                    addon.bOptions1 = static_cast<uint8_t>(mode);
                    addon.bOptions3 = static_cast<uint8_t>(operation);
                    addon.bHybOpt2 = 1;
                    if (mode == 2 && operation >= 5 && !secondDifs) {
                        expectHeaderFailure(header, addon, "FORMULA_INPUT_REJECTED[invalid_de_mix]");
                    } else {
                        const auto mct = GetMCTparasFromHeader(header, &addon, 0);
                        // Alternating dIFS and non-dIFS in one group is already
                        // invalid, independently of the unused mix selector.
                        const bool valid = mode == 2 || firstDifs == secondDifs;
                        check(bool(mct.bMCTisValid) == valid,
                              "DE mix validation changed ordinary group compatibility");
                        ++scenarios;
                    }
                }
            }
        }
    }
}

void testInterpolationColorWeighting() {
    // HeaderTrafos.pas uses the general zero-based accumulation, including
    // near-unit scale remapping, for interpolated hybrids as well.
    for (const auto weights : {std::array<float, 2>{1.0f, 0.0f},
                               std::array<float, 2>{0.25f, 0.75f},
                               std::array<float, 2>{0.75f, 0.25f}}) {
        for (double scale : {2.0, 1.125, -1.125, 1.0, -1.0}) {
            auto header = headerForCountTests();
            THeaderCustomAddon addon{};
            GetHAddOnFromInternFormula(&addon, 0, 0);
            GetHAddOnFromInternFormula(&addon, 4, 1);
            addon.bOptions1 = 1;
            addon.Formulas[1].dOptionValue[0] = scale;
            for (int slot = 0; slot < 2; ++slot) {
                std::memcpy(&addon.Formulas[slot].iItCount, &weights[slot], sizeof(float));
            }
            const auto mct = GetMCTparasFromHeader(header, &addon, 0);
            double expected = 0.6;
            if (std::fabs(scale) > 1.0) {
                // At abs(scale)=1.125 Pascal remaps it to 1.1390625.
                const double logScale = std::fabs(scale) == 1.125 ? 1.1390625 : 2.0;
                expected = weights[0] * 0.6 + weights[1] * (0.5 / (std::log(logScale) + 0.03));
            }
            check(mct.bMCTisValid && mct.mctColVarDEstopMul == static_cast<float>(expected),
                  "interpolation color weight differs from active Pascal accumulation");
            ++scenarios;
        }
    }

    // Historical integer-written weights become subnormal Singles. Both
    // implementations normalize their tiny sum to (1, 0), with coefficient .6.
    auto header = headerForCountTests();
    THeaderCustomAddon addon{};
    GetHAddOnFromInternFormula(&addon, 0, 0);
    GetHAddOnFromInternFormula(&addon, 4, 1);
    addon.bOptions1 = 1;
    addon.Formulas[0].iItCount = 25;
    addon.Formulas[1].iItCount = 75;
    const auto mct = GetMCTparasFromHeader(header, &addon, 0);
    check(mct.bMCTisValid && mct.nHybrid[0] == 0x3f800000 && mct.nHybrid[1] == 0 &&
          mct.mctColVarDEstopMul == 0.6f,
          "subnormal interpolation fallback retained an extra color offset");
    ++scenarios;
}

void testOrbitTrapSentinelLimits() {
    for (bool full : {false, true}) {
        auto ext = initialState();
        ext.maxIt = std::numeric_limits<int32_t>::max();
        ext.RStop = 0.0f; // One callback, then escape; never run INT_MAX iterations.
        TMCTparameter mct{};
        mct.iMaxIt = ext.maxIt;
        mct.dDEscale = 1.0f;
        try {
            if (full) CalcOTrapDEfull(&ext, &mct);
            else CalcOTrapDE(&ext, &mct);
        } catch (const std::invalid_argument& error) {
            check(std::string(error.what()).find("orbit-trap maximum iteration limit") !=
                      std::string::npos,
                  "orbit-trap sentinel rejection omitted its cause");
            check(ext.Ju1 == 1.0, "sentinel test failed to escape after one callback");
            ++scenarios;
            continue;
        }
        throw std::runtime_error("unrepresentable orbit-trap sentinel silently succeeded");
    }
}

uint32_t singleBits(float value) {
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

void testInterpolationPreparation() {
    // These are independent FPC outputs from the unchanged expressions in
    // HeaderTrafos.pas, not expected values recalculated by this test.
    for (const auto& fixture : kInterpolationSetupFixtures) {
        auto header = headerForCountTests();
        THeaderCustomAddon addon{};
        GetHAddOnFromInternFormula(&addon, 0, 0);
        GetHAddOnFromInternFormula(&addon, 0, 1);
        addon.bOptions1 = 1;
        addon.Formulas[0].dOptionValue[0] = 3.0;
        addon.Formulas[1].dOptionValue[0] = 2.0;
        std::memcpy(&addon.Formulas[0].iItCount, &fixture.a, sizeof(uint32_t));
        std::memcpy(&addon.Formulas[1].iItCount, &fixture.b, sizeof(uint32_t));
        const auto mct = GetMCTparasFromHeader(header, &addon, 0);
        check(static_cast<uint32_t>(mct.nHybrid[0]) == fixture.normalized_a &&
              static_cast<uint32_t>(mct.nHybrid[1]) == fixture.normalized_b,
              "interpolation normalization differs from native Pascal fixture");
        if (mct.bMCTisValid) {
            check(singleBits(mct.fHln[0]) == fixture.blended_log,
                  "interpolation Single log blend differs from native Pascal fixture");
        }
        ++scenarios;
    }

    for (int formula : {0, 2, 4}) {
        for (int disableADE : {0, 1}) {
            auto header = headerForCountTests();
            header.dZoom = 17.0 / 13.0;
            header.Iterations = header.iMaxItsF2 = 13;
            THeaderCustomAddon addon{};
            GetHAddOnFromInternFormula(&addon, formula, 0);
            GetHAddOnFromInternFormula(&addon, formula, 1);
            addon.bOptions1 = 1;
            addon.bOptions2 = static_cast<uint8_t>(disableADE);
            addon.Formulas[0].iItCount = 0x3e800000; // .25f
            addon.Formulas[1].iItCount = 0x3f400000; // .75f
            if (formula == 4) {
                addon.Formulas[0].dOptionValue[0] = 2.0;
                addon.Formulas[1].dOptionValue[0] = 2.0;
            }
            const auto mct = GetMCTparasFromHeader(header, &addon, 0);
            const bool fourD = formula == 2;
            const bool analytic = formula == 4;
            check(mct.bMCTisValid && bool(mct.IsCustomDE) == analytic &&
                  bool(mct.IsCustomDE2) == analytic &&
                  mct.mMandFunction == (fourD ? doInterpolHybridPas4D : fIsMemberIpol) &&
                  mct.mMandFunctionDE == (fourD ? doInterpolHybridPas4DDE : fIsMemberIpolDE) &&
                  mct.mMandFunction2 == mct.mMandFunction &&
                  mct.mMandFunctionDE2 == mct.mMandFunctionDE &&
                  mct.CalcDE == (analytic ? CalcDEanalytic : CalcDEnoADE),
                  "interpolation driver/analytic-DE selection differs from Pascal");
            if (analytic) {
                check(singleBits(mct.dDEscale) == kInterpolationFinalScale,
                      "analytic interpolation scale rounded the Double step width early");
            } else {
                check(singleBits(mct.dColPlus) == kInterpolationColorOffset,
                      "interpolation color offset rounded the iteration product early");
            }
            ++scenarios;
        }
    }

    for (int mode : {0, 1}) {
        for (double power : {1e100, -1e100, 2.5, 3.5}) {
            auto header = headerForCountTests();
            THeaderCustomAddon addon{};
            GetHAddOnFromInternFormula(&addon, 0, 0);
            GetHAddOnFromInternFormula(&addon, 0, 1);
            addon.bOptions1 = static_cast<uint8_t>(mode);
            addon.Formulas[0].dOptionValue[0] = power;
            addon.Formulas[0].iItCount = mode ? 0x3f000000 : 1;
            addon.Formulas[1].iItCount = mode ? 0x3f000000 : 0;
            const auto mct = GetMCTparasFromHeader(header, &addon, 0);
            const int expectedPower = power > 8.0 ? 8 : power == 3.5 ? 4 : 2;
            check(mct.bMCTisValid && mct.fHybrid[0] ==
                      reinterpret_cast<ThybridIteration2>(fHIntFunctions[expectedPower - 2]),
                  "internal integer power was rounded before its Pascal source clamp");
            ++scenarios;
        }
    }

    auto header = headerForCountTests();
    THeaderCustomAddon addon{};
    GetHAddOnFromInternFormula(&addon, 8, 0);
    GetHAddOnFromInternFormula(&addon, 8, 1);
    addon.bOptions1 = 1;
    addon.Formulas[0].iItCount = addon.Formulas[1].iItCount = 0x3f000000;
    expectHeaderFailure(header, addon, "dIFS formulas cannot be interpolated");
}

} // namespace

int main() {
    try {
        const auto* radius = mb3d::FindExternalFormula("_CondItcountR");
        const auto* switching = mb3d::FindExternalFormula("_CondItcountSwitch");
        const auto* switching2 = mb3d::FindExternalFormula("_CondItcountSwX2");
        check(radius && radius->wrapperFunc && switching && switching->wrapperFunc &&
                  switching2 && switching2->wrapperFunc,
              "ordinary conditional formula registration is missing");
        conditionalRadius = reinterpret_cast<ThybridIteration2>(radius->wrapperFunc);
        conditionalSwitch = reinterpret_cast<ThybridIteration2>(switching->wrapperFunc);
        conditionalSwitch2 = reinterpret_cast<ThybridIteration2>(switching2->wrapperFunc);
        testOrdinaryDrivers();
        testDisabledStartCompatibility();
        testConditionalAllDisabledExit();
        testDynamicAndHugeCounts();
        testInterpolatingDrivers();
        testPolicyBoundaries();
        testWorkerPropagation();
        testHeaderCountValidation();
        testDirectHybridLayout();
        testHeaderIterationLimits();
        testDEMixTypeValidation();
        testInterpolationColorWeighting();
        testInterpolationPreparation();
        testOrbitTrapSentinelLimits();
        std::printf("Formula iteration guard: %d scenarios passed; %u calls without progress allowed.\n",
                    scenarios, kBudget);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Formula iteration guard FAILED after %d scenarios: %s\n",
                     scenarios, error.what());
        return 1;
    }
}
