// Generated from Ads/_AbsZ.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5003 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ABSZ = 5003;
 template < > FormulaVec3 execute_formula < FORMULA__ABSZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixZ = params [ 0 ];
 return FormulaVec3 ( p . x , p . y , abs ( p . z + fixZ ) - fixZ );

}
 FormulaVec3 _AbsZ_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( p . x , p . y , abs ( p . z + params [ 0 ] ) - params [ 0 ] );

}

}}
