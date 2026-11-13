// Original CylinderHMVoid: preserve clipping, Single scratch stores and map channel aliases.
#include "heightmap_cyl_torus_shared.h"
constexpr unsigned int FORMULA_CYLINDERHMVOID = 10194;
template<>
cpu_double3 execute_formula_with_direct_map<FORMULA_CYLINDERHMVOID>(cpu_double3 p,constant double* params,double& de,TLMSfunction map) {
    return execute_cyl_torus_heightmap<1>(p,params,de,map);
}
template<>
cpu_double3 execute_formula<FORMULA_CYLINDERHMVOID>(cpu_double3 p,constant double* params,double& de) {
    return execute_formula_with_direct_map<FORMULA_CYLINDERHMVOID>(p,params,de,nullptr);
}
cpu_double3 CylinderHMVoid_formula(cpu_double3 p,constant double* params,double& de) {return execute_formula<FORMULA_CYLINDERHMVOID>(p,params,de);}
constant FormulaInfo formula_info_CylinderHMVoid={.id=FORMULA_CYLINDERHMVOID,.name="CylinderHMVoid",.param_count=23,.complexity=460,.uses_transcendental=true};
