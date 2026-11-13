// Generated from Ads/_MaxClipping.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7003 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__MAXCLIPPING = 7003;
 template < > FormulaVec3 execute_formula < FORMULA__MAXCLIPPING > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float xClip = params [ 0 ];
 const float yClip = params [ 1 ];
 const float zClip = params [ 2 ];
 return FormulaVec3 ( min ( p . x , xClip ) , min ( p . y , yClip ) , min ( p . z , zClip ) );

}
 FormulaVec3 _MaxClipping_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( min ( p . x , params [ 0 ] ) , min ( p . y , params [ 1 ] ) , min ( p . z , params [ 2 ] ) );

}

}}
