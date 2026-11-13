// Generated from Ads/_reciprocalY2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7008 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__RECIPROCALY2 = 7008;
 template < > FormulaVec3 execute_formula < FORMULA__RECIPROCALY2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Limiter = params [ 0 ];
 float new_y = copysign ( 1.0f , p . y ) / ( abs ( p . y ) + Limiter );
 return FormulaVec3 ( p . x , new_y , p . z );

}
 FormulaVec3 _reciprocalY2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( p . x , copysign ( 1.0f , p . y ) / ( abs ( p . y ) + params [ 0 ] ) , p . z );

}

}}
