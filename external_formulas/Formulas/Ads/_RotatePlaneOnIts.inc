// Shipped x87 rotation: expanded matrix, persistent bFirstIt counter, inverse matrix.
#include "core/formula_iteration_rotation.h"
constexpr unsigned int FORMULA__ROTATEPLANEONITS = 10011;
template<>
cpu_double3 execute_formula<FORMULA__ROTATEPLANEONITS>(cpu_double3 p, constant double* params, double &de) {
    return formulaIterationRotation(p, params, false);
}
constant FormulaInfo formula_info__RotatePlaneOnIts = {
    .id = FORMULA__ROTATEPLANEONITS,
    .name = "_RotatePlaneOnIts",
    .param_count = 5,
    .complexity = 209,
    .uses_transcendental = true
};
