#include "core/formula_validation.h"

#include "core/external_formulas.h"
#include "core/formula_registry.h"

#include <algorithm>
#include <cstring>
#include <iterator>
#include <sstream>

namespace mb3d {
namespace {

std::string SavedFormulaName(const THAformula& formula) {
    const char* name = reinterpret_cast<const char*>(formula.CustomFname);
    size_t length = 0;
    while (length < sizeof(formula.CustomFname) && name[length] != '\0') ++length;
    while (length > 0 && name[length - 1] == ' ') --length;
    return std::string(name, length);
}

int32_t PackedInteger(const void* pvar, int offset) {
    int32_t value;
    std::memcpy(&value, static_cast<const unsigned char*>(pvar) + offset,
                sizeof(value));
    return value;
}

FormulaValidationIssue Issue(FormulaValidationCode code, int slot,
                             int option, int32_t value,
                             const std::string& name, const char* reason) {
    std::ostringstream message;
    message << "FORMULA_INPUT_REJECTED[" << FormulaValidationCodeName(code) << "] ";
    if (slot >= 0) message << "slot " << slot + 1 << ' ';
    message << name;
    if (option >= 0)
        message << ", option " << option + 1 << " (packed int32=" << value << ')';
    message << ": " << reason;
    return {code, slot, option, value, name, message.str()};
}

bool IsCallerDependentHeightfield(unsigned id) {
    return id == 10222 || id == 10089 || id == 10208 || id == 10209;
}

} // namespace

const char* FormulaValidationCodeName(FormulaValidationCode code) {
    switch (code) {
    case FormulaValidationCode::UnknownExternalFormula: return "unknown_external_formula";
    case FormulaValidationCode::MissingPreparedParameters: return "missing_prepared_parameters";
    case FormulaValidationCode::CallerDependentFlip: return "caller_dependent_flip";
    case FormulaValidationCode::NonterminatingZeroPower: return "nonterminating_zero_power";
    case FormulaValidationCode::UninitializedAngleStack: return "uninitialized_angle_stack";
    }
    return "invalid_validation_code";
}

std::vector<FormulaValidationIssue> ValidateExternalFormulaInput(
    const THAformula& formula, const void* pvar, int slot) {
    if (IsInternalFormula(formula.iFnr)) return {};

    const std::string name = SavedFormulaName(formula);
    const ExternalFormulaInfo* info = FindExternalFormula(name.c_str());
    if (!info || !info->wrapperFunc) {
        return {Issue(FormulaValidationCode::UnknownExternalFormula, slot, -1, 0,
                      name, "external formula is not registered; an empty formula is not a substitute.")};
    }
    if (!pvar) {
        return {Issue(FormulaValidationCode::MissingPreparedParameters, slot, -1, 0,
                      info->name, "constant buffer was not prepared before input validation.")};
    }

    std::vector<FormulaValidationIssue> issues;
    if (IsCallerDependentHeightfield(info->id)) {
        // Original CODE: mov edx,[edi-0x54]; and edx,1; shl edx,4;
        // fld qword [esp+edx+0x10]. Odd values read outside the local frame,
        // consuming the original doHybridIFS3D caller's saved EDI/ESI bytes.
        const int32_t flip = PackedInteger(pvar, -0x54);
        if ((static_cast<uint32_t>(flip) & 1u) != 0) {
            issues.push_back(Issue(FormulaValidationCode::CallerDependentFlip,
                slot, 8, flip, info->name,
                "odd Flip XZ reads caller-stack memory in the original Pascal code; "
                "the C++ approximation is not accepted by strict rendering."));
        }
    } else if (info->id == 4225) { // TrifoxComplexAngles
        // SRECI2 expands stored option 1 to two Singles. Consequently the
        // angular power is stored option 3, expanded parameter 4, PVar-0x1c.
        const int32_t radial = PackedInteger(pvar, -0x0c);
        const int32_t angular = PackedInteger(pvar, -0x1c);
        if (radial == 0) {
            issues.push_back(Issue(FormulaValidationCode::NonterminatingZeroPower,
                slot, 0, radial, info->name,
                "Integer power R is zero; the original shift loop never terminates."));
        }
        if (angular == 0) {
            issues.push_back(Issue(FormulaValidationCode::NonterminatingZeroPower,
                slot, 3, angular, info->name,
                "Integer power Angle is zero; the original shift loop never terminates."));
        }
    } else if (info->id == 10022) { // _GenCWarp
        // Original CODE compares each word with 0..23. No default loads any
        // FPATAN operands: out-of-range modes consume the incoming x87 stack
        // (or underflow it). The C++ mode-0 fallback is not strict parity.
        for (int angle = 0; angle != 2; ++angle) {
            const int32_t mode = PackedInteger(pvar, -0x40 - angle * 4);
            if (mode < 0 || mode > 23) {
                issues.push_back(Issue(FormulaValidationCode::UninitializedAngleStack,
                    slot, 5 + angle, mode, info->name,
                    "angle selector is outside 0..23; the original FPATAN has no "
                    "initialized operands for this input."));
            }
        }
    }
    return issues;
}

std::vector<FormulaValidationIssue> ValidateFormulaInputs(
    const TMCTparameter& parameters, const THeaderCustomAddon& addon) {
    bool reachable[6] = {};
    const int mode = addon.bOptions1 & 3;
    if (mode == 1) {
        reachable[0] = reachable[1] = true;
    } else {
        const auto markPart = [&](int start, int repeat, int end) {
            // Include the initial prefix and the repeated cycle, but never
            // inspect dormant slots outside either runtime part's range.
            const int first = std::max(0, std::min(start, repeat));
            const int last = std::min(5, end);
            for (int slot = first; slot <= last; ++slot) {
                if ((static_cast<uint32_t>(parameters.nHybrid[slot]) & 0x7fffffffu) != 0)
                    reachable[slot] = true;
            }
        };
        markPart(parameters.StartFrom1, parameters.RepeatFrom1, parameters.wEndTo);
        if (mode == 2)
            markPart(parameters.StartFrom2, parameters.RepeatFrom2, parameters.iEnd2);
    }

    std::vector<FormulaValidationIssue> issues;
    for (int slot = 0; slot < 6; ++slot) {
        if (!reachable[slot]) continue;
        auto found = ValidateExternalFormulaInput(addon.Formulas[slot],
                                                  parameters.fHPVar[slot], slot);
        issues.insert(issues.end(), std::make_move_iterator(found.begin()),
                      std::make_move_iterator(found.end()));
    }
    return issues;
}

} // namespace mb3d
