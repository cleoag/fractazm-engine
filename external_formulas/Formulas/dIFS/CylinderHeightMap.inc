// Original CylinderHeightMap: preserve clipping, Single scratch stores and map channel aliases.
#include "heightmap_cyl_torus_shared.h"
constexpr unsigned int FORMULA_CYLINDERHEIGHTMAP = 10193;
template<>
cpu_double3 execute_formula_with_direct_map<FORMULA_CYLINDERHEIGHTMAP>(cpu_double3 p,constant double* params,double& de,TLMSfunction map) {
    return execute_cyl_torus_heightmap<0>(p,params,de,map);
}
template<>
cpu_double3 execute_formula<FORMULA_CYLINDERHEIGHTMAP>(cpu_double3 p,constant double* params,double& de) {
    return execute_formula_with_direct_map<FORMULA_CYLINDERHEIGHTMAP>(p,params,de,nullptr);
}
cpu_double3 CylinderHeightMap_formula(cpu_double3 p,constant double* params,double& de) {return execute_formula<FORMULA_CYLINDERHEIGHTMAP>(p,params,de);}
constant FormulaInfo formula_info_CylinderHeightMap={.id=FORMULA_CYLINDERHEIGHTMAP,.name="CylinderHeightMap",.param_count=21,.complexity=460,.uses_transcendental=true};
