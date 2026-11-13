// Original TorusHMVoid: preserve clipping, Single scratch stores and map channel aliases.
#include "heightmap_cyl_torus_shared.h"
constexpr unsigned int FORMULA_TORUSHMVOID = 10197;
template<>
cpu_double3 execute_formula_with_direct_map<FORMULA_TORUSHMVOID>(cpu_double3 p,constant double* params,double& de,TLMSfunction map) {
    return execute_cyl_torus_heightmap<2>(p,params,de,map);
}
template<>
cpu_double3 execute_formula<FORMULA_TORUSHMVOID>(cpu_double3 p,constant double* params,double& de) {
    return execute_formula_with_direct_map<FORMULA_TORUSHMVOID>(p,params,de,nullptr);
}
cpu_double3 TorusHMVoid_formula(cpu_double3 p,constant double* params,double& de) {return execute_formula<FORMULA_TORUSHMVOID>(p,params,de);}
constant FormulaInfo formula_info_TorusHMVoid={.id=FORMULA_TORUSHMVOID,.name="TorusHMVoid",.param_count=22,.complexity=460,.uses_transcendental=true};
