// Generated from Ads/_Abs4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5004 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ABS4D = 5004;
 template < > FormulaVec3 execute_formula < FORMULA__ABS4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 params [ FORMULA_STATE_W_SLOT ] = abs ( params [ FORMULA_STATE_W_SLOT ] );
 return FormulaVec3 ( abs ( p . x ) , abs ( p . y ) , abs ( p . z ) );

}
 FormulaVec3 _Abs4d_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( abs ( p . x ) , abs ( p . y ) , abs ( p . z ) );

}

}}
