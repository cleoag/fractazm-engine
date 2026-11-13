// Generated from Ads/_invtorical.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7043 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__INVTORICAL = 7043;
 template < > FormulaVec3 execute_formula < FORMULA__INVTORICAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float R = params [ 0 ];
 float new_x = abs ( R - p . x ) * abs ( cos ( p . y ) );
 float new_y = new_x * tan ( p . y );
 return FormulaVec3 ( new_x , new_y , p . z );

}

}}
