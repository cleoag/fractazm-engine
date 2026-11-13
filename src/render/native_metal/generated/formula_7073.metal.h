// Generated from Ads/_skew.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7073 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SKEW = 7073;
 template < > FormulaVec3 execute_formula < FORMULA__SKEW > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Limit = params [ 0 ];
 const float Skewness = params [ 1 ];
 float x = p . x , y = p . y , z = p . z;
 float sk_x = Limit - x * Skewness;
 float sk_y = Limit - y * Skewness;
 float sk_z = Limit - z * Skewness;
 float m = min ( sk_x , min ( sk_y , sk_z ) );
 float shift = m + abs ( m );
 return FormulaVec3 ( x - shift , y - shift , z - shift );

}

}}
