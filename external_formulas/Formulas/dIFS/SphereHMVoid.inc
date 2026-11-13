// Original SphereHMVoid: matrix, clipping, map callback and masked channel arithmetic.
#include "heightmap_sphere_shared.h"
constexpr unsigned int FORMULA_SPHEREHMVOID = 10301;
template<> struct ExternalDifsUsesSphereMap<FORMULA_SPHEREHMVOID> { static constexpr bool value = true; };
template<>
cpu_double3 execute_formula_with_sphere_map<FORMULA_SPHEREHMVOID>(cpu_double3 p,constant double* params,double& de,TLMSfunction map){
    return execute_spherical_heightmap<true>(p,params,de,map);
}
template<>
cpu_double3 execute_formula<FORMULA_SPHEREHMVOID>(cpu_double3 p,constant double* params,double& de){
    return execute_formula_with_sphere_map<FORMULA_SPHEREHMVOID>(p,params,de,nullptr);
}
cpu_double3 SphereHMVoid_formula(cpu_double3 p,constant double* params,double& de){return execute_formula<FORMULA_SPHEREHMVOID>(p,params,de);}
constant FormulaInfo formula_info_SphereHMVoid={.id=FORMULA_SPHEREHMVOID,.name="SphereHMVoid",.param_count=23,.complexity=440,.uses_transcendental=false};
