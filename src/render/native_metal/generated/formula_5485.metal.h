// Generated from Ads/_PlusSinApp.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5485 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__PLUSSINAPP = 5485;
 template < > FormulaVec3 execute_formula < FORMULA__PLUSSINAPP > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int destination = int ( params [ 0 ] ) & 3;
 const int source = int ( params [ 1 ] ) & 3;
 float coords [ 4 ] = {
 p . x , p . y , p . z , params [ FORMULA_STATE_W_SLOT ]
}
;
 coords [ destination ] = external_sine_transform ( coords [ source ] , params + 2 , params + 6 , coords [ destination ] , true );
 params [ FORMULA_STATE_W_SLOT ] = coords [ 3 ];
 return FormulaVec3 ( coords [ 0 ] , coords [ 1 ] , coords [ 2 ] );

}

}}
