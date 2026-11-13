// Generated from Ads/_reciprocalX2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7007 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__RECIPROCALX2 = 7007;
 template < > FormulaVec3 execute_formula < FORMULA__RECIPROCALX2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Limiter = params [ 0 ];
 float new_x = copysign ( 1.0f , p . x ) / ( abs ( p . x ) + Limiter );
 return FormulaVec3 ( new_x , p . y , p . z );

}
 FormulaVec3 _reciprocalX2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( copysign ( 1.0f , p . x ) / ( abs ( p . x ) + params [ 0 ] ) , p . y , p . z );

}

}}
