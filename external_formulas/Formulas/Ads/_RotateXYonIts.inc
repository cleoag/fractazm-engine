// Shipped x87 rotation: expanded matrix, persistent bFirstIt counter, inverse matrix.
#include "core/formula_iteration_rotation.h"
constexpr unsigned int FORMULA__ROTATEXYONITS = 10012;
template<>
cpu_double3 execute_formula<FORMULA__ROTATEXYONITS>(cpu_double3 p, constant double* params, double &de) {
    return formulaIterationRotation(p, params, false);
}
constant FormulaInfo formula_info__RotateXYonIts = {
    .id = FORMULA__ROTATEXYONITS,
    .name = "_RotateXYonIts",
    .param_count = 5,
    .complexity = 209,
    .uses_transcendental = true
};
