#ifndef MB3D_CORE_FORMULA_REGISTRY_H
#define MB3D_CORE_FORMULA_REGISTRY_H

#include "core/formulas.h"
#include <array>
#include <string>

namespace mb3d {

constexpr int INTERN_FORMULA_MAX = 9;
constexpr int EXTERN_FORMULA_THRESHOLD = 20;

// InternFormulaNames: maps formula number (0..9) to human-readable name
// From HeaderTrafos.pas line 49
extern const std::array<std::string, 10> InternFormulaNames;

// InternFormulasDEfunc: maps formula number (0..9) to DE function type
// 0: no analytic DE (numerical only)
// 4: Quaternion DE mode
// 11: AmazingBox/Mandelbox with DE tracking on w
// -1: intentionally no DE option (test formula 7)
// 20: dIFS (test formula 8)
extern const std::array<int, 10> InternFormulasDEfunc;

// FormulaFunc type alias -- same as ThybridIteration
using FormulaFunc = ThybridIteration;

// GetInternalFormula -- get formula function for internal formula ID
//
// iFnr 0: Integer Power (power parameter selects IntPow2..IntPow8)
// iFnr 1: Real Power (FloatPow)
// iFnr 2: Quaternion
// iFnr 3: Tricorn
// iFnr 4: Amazing Box (selects CubeDE or Cube based on DE mode)
// iFnr 5: Bulbox
// iFnr 6: Folding Int Pow
// iFnr 7: test formula
// iFnr 8: testIFS formula
// iFnr 9: Aexion C
FormulaFunc GetInternalFormula(int iFnr, int power = 2, bool withDE = true);

// IsInternalFormula -- check if formula number is internal (0..9)
bool IsInternalFormula(int iFnr);

// GetDEoptionForFormula -- get the DE option code for a formula
// Returns: 0=numerical, 2=analytic via w, 4=quaternion DE, 5=w-based DE,
//          11=w-based AmazingBox DE, 20=dIFS
int GetDEoptionForFormula(int iFnr);

} // namespace mb3d

#endif // MB3D_CORE_FORMULA_REGISTRY_H
