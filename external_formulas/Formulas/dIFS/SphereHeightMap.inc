// Original SphereHeightMap: matrix, clipping, map callback and masked channel arithmetic.
#include "heightmap_sphere_shared.h"
constexpr unsigned int FORMULA_SPHEREHEIGHTMAP = 10191;
template<> struct ExternalDifsUsesSphereMap<FORMULA_SPHEREHEIGHTMAP> { static constexpr bool value = true; };
template<>
cpu_double3 execute_formula_with_sphere_map<FORMULA_SPHEREHEIGHTMAP>(cpu_double3 p,constant double* params,double& de,TLMSfunction map){
    return execute_spherical_heightmap<false>(p,params,de,map);
}
template<>
cpu_double3 execute_formula<FORMULA_SPHEREHEIGHTMAP>(cpu_double3 p,constant double* params,double& de){
    return execute_formula_with_sphere_map<FORMULA_SPHEREHEIGHTMAP>(p,params,de,nullptr);
}
cpu_double3 SphereHeightMap_formula(cpu_double3 p,constant double* params,double& de){return execute_formula<FORMULA_SPHEREHEIGHTMAP>(p,params,de);}
constant FormulaInfo formula_info_SphereHeightMap={.id=FORMULA_SPHEREHEIGHTMAP,.name="SphereHeightMap",.param_count=20,.complexity=440,.uses_transcendental=false};
