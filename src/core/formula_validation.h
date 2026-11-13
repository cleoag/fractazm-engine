#pragma once

#include "core/types.h"

#include <cstdint>
#include <string>
#include <vector>

namespace mb3d {

enum class FormulaValidationCode {
    UnknownExternalFormula,
    MissingPreparedParameters,
    CallerDependentFlip,
    NonterminatingZeroPower,
    UninitializedAngleStack,
};

struct FormulaValidationIssue {
    FormulaValidationCode code;
    int slot;          // Zero-based hybrid slot; -1 when unavailable.
    int option_index;  // Zero-based stored option, not expanded PVar parameter.
    int32_t effective_value;
    std::string formula;
    std::string message;
};

const char* FormulaValidationCodeName(FormulaValidationCode code);

// Validate one selected external formula, as used by the point probe. pvar must
// point into its prepared constant buffer (at least 0x54 readable bytes before
// it for the four heightfields, 0x1c for Trifox, 0x44 for _GenCWarp).
// Integer options are read from
// that buffer with memcpy: stored Double bits are never treated as integers,
// and the loader's conversion/defaults are not reimplemented here.
std::vector<FormulaValidationIssue> ValidateExternalFormulaInput(
    const THAformula& formula, const void* pvar, int slot = -1);

// Call after GetMCTparasFromHeader, before starting workers or invoking any
// formula. Validate both reachable DE-combination parts; interpolation invokes
// both slots even when a weight is zero. Initial-zero ordinary slots contain
// EmptyFormula in both Pascal and C++, including when a conditional later
// changes their counter, so their dormant saved formula name is not validated.
// An empty result means only "no known unsupported input", never parity PASS.
std::vector<FormulaValidationIssue> ValidateFormulaInputs(
    const TMCTparameter& parameters, const THeaderCustomAddon& addon);

} // namespace mb3d
