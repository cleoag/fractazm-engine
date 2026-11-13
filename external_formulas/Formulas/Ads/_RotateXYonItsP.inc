// Shipped x87 rotation: expanded matrix, persistent bFirstIt counter, inverse matrix.
#include "core/formula_iteration_rotation.h"
constexpr unsigned int FORMULA__ROTATEXYONITSP = 10013;
template<>
cpu_double3 execute_formula<FORMULA__ROTATEXYONITSP>(cpu_double3 p, constant double* params, double &de) {
    return formulaIterationRotation(p, params, true);
}
constant FormulaInfo formula_info__RotateXYonItsP = {
    .id = FORMULA__ROTATEXYONITSP,
    .name = "_RotateXYonItsP",
    .param_count = 7,
    .complexity = 259,
    .uses_transcendental = true
};
