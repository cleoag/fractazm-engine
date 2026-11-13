// Generated from Ads/_HopSqrtZ.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7031 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__HOPSQRTZ = 7031;
 template < > FormulaVec3 execute_formula < FORMULA__HOPSQRTZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixZ = params [ 0 ];
 const float fixSq = params [ 1 ];
 const float Mul = params [ 2 ];
 const float Div = params [ 3 ];
 float t = Mul * p . z + fixZ;
 float result = fixSq + sign ( t ) * sqrt ( abs ( t ) ) / Div;
 return FormulaVec3 ( p . x , p . y , result );

}

}}
