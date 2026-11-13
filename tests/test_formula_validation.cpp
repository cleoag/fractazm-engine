#include "core/formula_validation.h"

#include "core/external_formulas.h"
#include "core/header_trafos.h"

#include <array>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace {

int checks = 0;

void Check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}

THAformula Formula(const char* name) {
    THAformula formula{};
    formula.iFnr = 20;
    formula.iItCount = 1;
    const size_t size = std::strlen(name);
    std::memcpy(formula.CustomFname, name,
                size < sizeof(formula.CustomFname) ? size : sizeof(formula.CustomFname));
    return formula;
}

struct Packed {
    std::array<unsigned char, 1025> bytes{};
    // Deliberately unaligned: validators must use memcpy, not int32_t* casts.
    void* pvar() { return bytes.data() + 513; }
    void integer(int offset, int32_t value) {
        std::memcpy(bytes.data() + 513 + offset, &value, sizeof(value));
    }
};

void CheckPackedOptions() {
    using mb3d::FormulaValidationCode;
    const char* names[] = {"gnarlyIFS", "gnarly2IFS", "gnarly3IFS", "ducksIFS"};
    const int32_t values[] = {std::numeric_limits<int32_t>::min(), -257, -3, -2,
        -1, 0, 1, 2, 3, 256, 257, std::numeric_limits<int32_t>::max()};
    for (const char* name : names) {
        auto formula = Formula(name);
        // The raw saved Double is deliberately unrelated to the packed value.
        formula.dOptionValue[8] = 1.0;
        Packed buffer;
        for (int32_t value : values) {
            buffer.integer(-0x54, value);
            const auto issues = mb3d::ValidateExternalFormulaInput(formula, buffer.pvar(), 2);
            const bool odd = (static_cast<uint32_t>(value) & 1u) != 0;
            Check(issues.size() == (odd ? 1u : 0u), "heightfield parity bit was decoded incorrectly");
            if (odd) {
                Check(issues[0].code == FormulaValidationCode::CallerDependentFlip &&
                      issues[0].slot == 2 && issues[0].option_index == 8 &&
                      issues[0].effective_value == value,
                      "heightfield rejection lost its exact packed input identity");
                Check(issues[0].message.find("FORMULA_INPUT_REJECTED[caller_dependent_flip]") !=
                      std::string::npos, "missing explicit machine-readable rejection diagnostic");
            }
        }
    }

    auto trifox = Formula("TrifoxComplexAngles");
    trifox.dOptionValue[0] = trifox.dOptionValue[3] = 0; // Packed values govern.
    Packed buffer;
    buffer.integer(-0x0c, 2);
    buffer.integer(-0x1c, -3);
    Check(mb3d::ValidateExternalFormulaInput(trifox, buffer.pvar()).empty(),
          "nonzero Trifox powers must remain supported");
    buffer.integer(-0x0c, 0);
    auto issues = mb3d::ValidateExternalFormulaInput(trifox, buffer.pvar());
    Check(issues.size() == 1 && issues[0].option_index == 0 &&
          issues[0].code == FormulaValidationCode::NonterminatingZeroPower,
          "zero radial power must be rejected");
    buffer.integer(-0x0c, -1);
    buffer.integer(-0x1c, 0);
    issues = mb3d::ValidateExternalFormulaInput(trifox, buffer.pvar());
    Check(issues.size() == 1 && issues[0].option_index == 3,
          "angular power uses stored option 3, not expanded parameter 4");
    buffer.integer(-0x0c, 0);
    Check(mb3d::ValidateExternalFormulaInput(trifox, buffer.pvar()).size() == 2,
          "both zero powers must be diagnosed");

    auto padded = Formula("gNaRlYiFs");
    std::memset(padded.CustomFname + 9, ' ', sizeof(padded.CustomFname) - 9);
    buffer.integer(-0x54, 1);
    issues = mb3d::ValidateExternalFormulaInput(padded, buffer.pvar());
    Check(issues.size() == 1 && issues[0].code == FormulaValidationCode::CallerDependentFlip,
          "space-padded, case-insensitive, non-NUL name must resolve safely");
    auto unknown = Formula("not_a_registered_external_formula"); // Fills all 32 bytes.
    issues = mb3d::ValidateExternalFormulaInput(unknown, nullptr);
    Check(issues.size() == 1 && issues[0].code == FormulaValidationCode::UnknownExternalFormula,
          "unknown external formula must not silently become EmptyFormula");
    Check(issues[0].formula.size() == 32, "non-NUL unknown name must stay within its field");
    issues = mb3d::ValidateExternalFormulaInput(Formula("gnarlyIFS"), nullptr);
    Check(issues.size() == 1 && issues[0].code == FormulaValidationCode::MissingPreparedParameters,
          "missing prepared buffer must be a diagnostic, not a dereference");
    unknown.iFnr = 0;
    Check(mb3d::ValidateExternalFormulaInput(unknown, nullptr).empty(),
          "internal formulas must not be resolved by their unused external name");
}

void CheckLoadedOptions() {
    // Independent nearest-even boundary expectations; these exercise the real
    // loader and its default migration before validating the resulting bytes.
    const struct { double saved; bool odd; } flips[] = {
        {0, false}, {0.5, false}, {std::nextafter(0.5, 0.0), false},
        {std::nextafter(0.5, 1.0), true}, {1.5, false}, {2.5, false},
        {3.5, false}, {-0.5, false}, {-1.5, false},
        {std::nextafter(-0.5, -1.0), true}, {257.0, true}, {-257.0, true},
    };
    THeaderCustomAddon addon{};
    for (const char* name : {"gnarlyIFS", "gnarly2IFS", "gnarly3IFS", "ducksIFS"}) {
        for (const auto& fixture : flips) {
            addon.Formulas[0] = Formula(name);
            addon.Formulas[0].iOptionCount = 9;
            addon.Formulas[0].dOptionValue[8] = fixture.saved;
            mb3d::ApplyExternalFormulaOptionTypes(addon.Formulas[0]);
            mb3d::BuildFormulaConstBuffer(0, &addon, 0);
            auto issues = mb3d::ValidateExternalFormulaInput(addon.Formulas[0],
                mb3d::GetFormulaConstBuffers().pConstPointer[0]);
            Check(issues.size() == (fixture.odd ? 1u : 0u),
                  "loaded Flip must use Pascal integer conversion, not truncation or Double bits");
        }
        addon.Formulas[0] = Formula(name);
        addon.Formulas[0].iOptionCount = 8;
        addon.Formulas[0].dOptionValue[8] = 1; // Not saved: default Flip is zero.
        mb3d::ApplyExternalFormulaOptionTypes(addon.Formulas[0]);
        mb3d::BuildFormulaConstBuffer(0, &addon, 0);
        Check(mb3d::ValidateExternalFormulaInput(addon.Formulas[0],
              mb3d::GetFormulaConstBuffers().pConstPointer[0]).empty(),
              "old scene must validate the migrated Flip default, not stale payload");
    }
    for (int option : {0, 3}) {
        for (double value : {0.0, -0.0, 0.5, -0.5, std::nextafter(0.5, 1.0),
                              std::nextafter(-0.5, -1.0), 1.0, -1.0, 2.0}) {
            addon.Formulas[0] = Formula("TrifoxComplexAngles");
            mb3d::ApplyExternalFormulaOptionTypes(addon.Formulas[0]);
            addon.Formulas[0].dOptionValue[option] = value;
            mb3d::BuildFormulaConstBuffer(0, &addon, 0);
            const auto issues = mb3d::ValidateExternalFormulaInput(addon.Formulas[0],
                mb3d::GetFormulaConstBuffers().pConstPointer[0]);
            const bool zeroAfterRounding = value >= -0.5 && value <= 0.5;
            Check(issues.size() == (zeroAfterRounding ? 1u : 0u),
                  "Trifox zero-power validation must follow integer packing");
            if (zeroAfterRounding)
                Check(issues[0].option_index == option, "Trifox diagnostic reports wrong saved option");
        }
    }
    addon.Formulas[0] = Formula("TrifoxComplexAngles");
    mb3d::ApplyExternalFormulaOptionTypes(addon.Formulas[0]);
    mb3d::BuildFormulaConstBuffer(0, &addon, 0);
    Check(mb3d::ValidateExternalFormulaInput(addon.Formulas[0],
          mb3d::GetFormulaConstBuffers().pConstPointer[0]).empty(),
          "missing Trifox powers must acquire nonzero defaults");
}

void CheckAngleStackInputs() {
    using mb3d::FormulaValidationCode;
    auto formula = Formula("_GenCWarp");
    formula.dOptionValue[5] = formula.dOptionValue[6] = -1; // Packed words govern.
    Packed buffer;
    for (int angle = 0; angle != 2; ++angle) {
        for (int32_t value : {std::numeric_limits<int32_t>::min(), -1, 0, 1,
                             22, 23, 24, std::numeric_limits<int32_t>::max()}) {
            buffer.integer(-0x40, 2);
            buffer.integer(-0x44, 8);
            buffer.integer(-0x40 - angle * 4, value);
            const auto issues = mb3d::ValidateExternalFormulaInput(formula, buffer.pvar(), 1);
            const bool invalid = value < 0 || value > 23;
            Check(issues.size() == (invalid ? 1u : 0u), "angle stack range decoded incorrectly");
            if (invalid) {
                Check(issues[0].code == FormulaValidationCode::UninitializedAngleStack &&
                      issues[0].option_index == 5 + angle && issues[0].effective_value == value &&
                      issues[0].slot == 1, "angle rejection lost packed input identity");
                Check(issues[0].message.find("FORMULA_INPUT_REJECTED[uninitialized_angle_stack]") !=
                      std::string::npos, "angle rejection lacks its diagnostic code");
            }
        }
    }
    for (int theta = 0; theta != 24; ++theta) {
        for (int phi = 0; phi != 24; ++phi) {
            buffer.integer(-0x40, theta);
            buffer.integer(-0x44, phi);
            Check(mb3d::ValidateExternalFormulaInput(formula, buffer.pvar()).empty(),
                  "defined angle pair must remain supported");
        }
    }
    buffer.integer(-0x40, -1);
    buffer.integer(-0x44, 24);
    Check(mb3d::ValidateExternalFormulaInput(formula, buffer.pvar()).size() == 2,
          "both invalid angle selectors must be diagnosed");

    const struct { double saved; bool rejected; } values[] = {
        {-1.0, true}, {-0.5, false}, {std::nextafter(-0.5, -1.0), true},
        {0.0, false}, {23.0, false}, {std::nextafter(23.5, 0.0), false},
        {23.5, true}, {24.0, true},
    };
    for (int option : {5, 6}) {
        for (const auto& value : values) {
            THeaderCustomAddon addon{};
            addon.Formulas[0] = Formula("_GenCWarp");
            mb3d::ApplyExternalFormulaOptionTypes(addon.Formulas[0]);
            addon.Formulas[0].dOptionValue[option] = value.saved;
            mb3d::BuildFormulaConstBuffer(0, &addon, 0);
            const auto issues = mb3d::ValidateExternalFormulaInput(addon.Formulas[0],
                mb3d::GetFormulaConstBuffers().pConstPointer[0]);
            Check(issues.size() == (value.rejected ? 1u : 0u),
                  "angle validation must follow real integer loading and matrix expansion");
            if (value.rejected)
                Check(issues[0].option_index == option, "angle stored option index is incorrect");
        }
    }
}

void CheckHybridReachability() {
    THeaderCustomAddon addon{};
    TMCTparameter parameters{};
    Packed buffer;
    buffer.integer(-0x54, 1);
    for (int slot = 0; slot < 6; ++slot) {
        addon.Formulas[slot] = Formula("gnarlyIFS");
        parameters.fHPVar[slot] = buffer.pvar();
        parameters.nHybrid[slot] = 1;
    }
    parameters.wEndTo = 1;
    Check(mb3d::ValidateFormulaInputs(parameters, addon).size() == 2,
          "ordinary validation must not inspect slots beyond the active part");
    parameters.nHybrid[0] = 0;
    parameters.nHybrid[1] = std::numeric_limits<int32_t>::min() + 1;
    auto issues = mb3d::ValidateFormulaInputs(parameters, addon);
    Check(issues.size() == 1 && issues[0].slot == 1,
          "disable counting bit does not disable formula execution");
    parameters.nHybrid[1] = std::numeric_limits<int32_t>::min();
    Check(mb3d::ValidateFormulaInputs(parameters, addon).empty(),
          "zero low-bit count must remain dormant");
    // Initial-zero named slots become EmptyFormula in Pascal HeaderTrafos.
    addon.Formulas[0] = Formula("_CondItcountSwitch");
    parameters.nHybrid[0] = std::numeric_limits<int32_t>::min() + 1;
    buffer.integer(-16, 2); // Conditional names slot 2, whose initial count is zero.
    buffer.integer(-20, 1);
    buffer.integer(-24, 1);
    buffer.integer(-28, 1);
    Check(mb3d::ValidateFormulaInputs(parameters, addon).empty(),
          "conditional activation of a dormant EmptyFormula must not validate its saved name");

    addon.Formulas[0] = Formula("gnarlyIFS");
    addon.bOptions1 = 1;
    parameters.nHybrid[0] = parameters.nHybrid[1] = 0;
    Check(mb3d::ValidateFormulaInputs(parameters, addon).size() == 2,
          "interpolation executes both formulas even at zero weight");

    addon.bOptions1 = 2;
    parameters.StartFrom2 = parameters.RepeatFrom2 = 4;
    parameters.iEnd2 = 5;
    Check(mb3d::ValidateFormulaInputs(parameters, addon).size() == 2,
          "second DE-combination part must also be validated");
    addon.bOptions1 = 3;
    Check(mb3d::ValidateFormulaInputs(parameters, addon).empty(),
          "IFS mode must not activate the dormant DE-combination part");
    parameters.StartFrom1 = 3;
    parameters.RepeatFrom1 = 2;
    parameters.wEndTo = 3;
    issues = mb3d::ValidateFormulaInputs(parameters, addon);
    Check(issues.size() == 2 && issues[0].slot == 2 && issues[1].slot == 3,
          "initial prefix and repeat range must both be validated");

    addon.Formulas[2] = Formula("unknown_external");
    issues = mb3d::ValidateFormulaInputs(parameters, addon);
    Check(issues.size() == 2 &&
          issues[0].code == mb3d::FormulaValidationCode::UnknownExternalFormula,
          "unknown reachable external must be diagnosed before workers start");
    parameters.nHybrid[2] = 0;
    Check(mb3d::ValidateFormulaInputs(parameters, addon).size() == 1,
          "dormant unknown saved formula must not reject a valid hybrid");
}

} // namespace

int main() {
    try {
        CheckPackedOptions();
        CheckLoadedOptions();
        CheckAngleStackInputs();
        CheckHybridReachability();
        std::printf("Formula input validation: %d checks passed (not a numerical parity result).\n", checks);
        return 0;
    } catch (const std::exception& error) {
        std::fprintf(stderr, "Formula input validation failed after %d checks: %s\n", checks, error.what());
        return 1;
    }
}
