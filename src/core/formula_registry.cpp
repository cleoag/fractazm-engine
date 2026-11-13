#include "core/formula_registry.h"
#include <algorithm>
#include <cmath>

namespace mb3d {

// From HeaderTrafos.pas line 49
const std::array<std::string, 10> InternFormulaNames = {{
    "Integer Power",
    "Real Power",
    "Quaternion",
    "Tricorn",
    "Amazing Box",
    "Bulbox",
    "Folding Int Pow",
    "test",
    "testIFS",
    "Aexion C"
}};

// DE function type for each internal formula 0..9
// 0: Integer Power -> numerical DE (DEoption 0)
// 1: Real Power    -> numerical DE (DEoption 0)
// 2: Quaternion    -> analytic DE via Deriv1 (DEoption 4)
// 3: Tricorn       -> numerical DE (DEoption 0)
// 4: Amazing Box   -> analytic DE via w (DEoption 11)
// 5: Bulbox        -> numerical DE (DEoption 0)
// 6: Folding IntPow -> numerical DE (DEoption 0)
// 7: test         -> no DE option (-1)
// 8: testIFS      -> dIFS (DEoption 20)
// 9: Aexion C     -> numerical DE (DEoption 0)
const std::array<int, 10> InternFormulasDEfunc = {{0, 0, 4, 0, 11, 0, 0, -1, 20, 0}};

FormulaFunc GetInternalFormula(int iFnr, int power, bool withDE) {
    switch (iFnr) {
        case 0: {
            // Integer power -- power selects IntPow2..IntPow8
            int p = std::max(2, std::min(8, power));
            return fHIntFunctions[p - 2];
        }
        case 1:
            return HybridFloatPow;
        case 2:
            return HybridQuat;
        case 3:
            return HybridItTricorn;
        case 4:
            // Amazing Box: with DE uses CubeDE, without uses Cube
            return withDE ? fHybridCubeDE : fHybridCube;
        case 5:
            return HybridSuperCube2;
        case 6:
            return HybridFolding;
        case 7:
            return TestHybrid;
        case 8:
            return HybridCustomIFStest;
        case 9:
            return AexionC;
        default:
            return EmptyFormula;
    }
}

bool IsInternalFormula(int iFnr) {
    return iFnr >= 0 && iFnr <= INTERN_FORMULA_MAX;
}

int GetDEoptionForFormula(int iFnr) {
    if (iFnr < 0 || iFnr >= static_cast<int>(InternFormulasDEfunc.size())) return 0;
    return InternFormulasDEfunc[iFnr];
}

} // namespace mb3d
