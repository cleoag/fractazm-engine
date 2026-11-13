// Generated from Ads/_Folding2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6002 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDING2 = 6002;
 template < > FormulaVec3 execute_formula < FORMULA__FOLDING2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fold = params [ 0 ] , twice = params [ 1 ];
 return FormulaVec3 ( p . x + abs ( p . x - twice ) - abs ( p . x - fold ) - fold , p . y + abs ( p . y - twice ) - abs ( p . y - fold ) - fold , p . z + abs ( p . z - twice ) - abs ( p . z - fold ) - fold );

}

}}
