// Generated from Ads/_invspherical.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4523 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__INVSPHERICAL = 4523;
 template < > FormulaVec3 execute_formula < FORMULA__INVSPHERICAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = p . x;
 float sy = sin ( p . y );
 float cy = cos ( p . y );
 float sz = sin ( p . z );
 float cz = cos ( p . z );
 return FormulaVec3 ( x * sy * cz , x * sy * sz , x * cy );

}
 FormulaVec3 _invspherical_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float sy = sin ( p . y ) , cy = cos ( p . y );
 float sz = sin ( p . z ) , cz = cos ( p . z );
 return FormulaVec3 ( p . x * sy * cz , p . x * sy * sz , p . x * cy );

}

}}
