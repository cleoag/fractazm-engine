// Generated from Ads/_tocylindrical.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4521 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__TOCYLINDRICAL = 4521;
 template < > FormulaVec3 execute_formula < FORMULA__TOCYLINDRICAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float r = sqrt ( p . x * p . x + p . y * p . y );
 float theta = atan2 ( p . y , p . x );
 return FormulaVec3 ( r , theta , p . z );

}
 FormulaVec3 _tocylindrical_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( sqrt ( p . x * p . x + p . y * p . y ) , atan2 ( p . y , p . x ) , p . z );

}

}}
