// Generated from 4D/_conj4D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4516 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__CONJ4D = 4516;
 template < > FormulaVec3 execute_formula < FORMULA__CONJ4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 ( void ) params;
 de = - de;
 return FormulaVec3 ( p . x , - p . y , - p . z );

}
 FormulaVec3 _conj4D_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 ( void ) params;
 de = - de;
 return FormulaVec3 ( p . x , - p . y , - p . z );

}

}}
