// Generated from Ads/_reciprocalZ3b.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6834 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__RECIPROCALZ3B = 6834;
 template < > FormulaVec3 execute_formula < FORMULA__RECIPROCALZ3B > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float value = p . z;
 float result = params [ 1 ] - 1.0f / ( abs ( params [ 4 ] * value ) + params [ 0 ] );
 result += params [ 3 ] - 1.0f / ( abs ( params [ 5 ] * value * value ) + params [ 2 ] );
 if ( metal :: signbit ( value ) ) result = - result;
 return FormulaVec3 ( p . x , p . y , result );

}
 FormulaVec3 _reciprocalZ3b_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__RECIPROCALZ3B > ( p , params , de );

}

}}
