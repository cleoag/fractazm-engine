// Generated from Ads/_invcylindrical.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4522 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__INVCYLINDRICAL = 4522;
 template < > FormulaVec3 execute_formula < FORMULA__INVCYLINDRICAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float c = cos ( p . y );
 float s = sin ( p . y );
 return FormulaVec3 ( p . x * c , p . x * s , p . z );

}
 FormulaVec3 _invcylindrical_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( p . x * cos ( p . y ) , p . x * sin ( p . y ) , p . z );

}

}}
