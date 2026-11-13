// Generated from Ads/_SinApprox.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4611 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SINAPPROX = 4611;
 template < > FormulaVec3 execute_formula < FORMULA__SINAPPROX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int index = int ( params [ 0 ] ) & 3;
 float coords [ 4 ] = {
 p . x , p . y , p . z , params [ FORMULA_STATE_W_SLOT ]
}
;
 coords [ index ] = external_sine_transform ( coords [ index ] , params + 1 , params + 5 , 0.0f , false );
 params [ FORMULA_STATE_W_SLOT ] = coords [ 3 ];
 return FormulaVec3 ( coords [ 0 ] , coords [ 1 ] , coords [ 2 ] );

}

}}
