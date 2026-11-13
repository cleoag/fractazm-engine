// Generated from Ads/_reciprocalY3.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7017 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__RECIPROCALY3 = 7017;
 template < > FormulaVec3 execute_formula < FORMULA__RECIPROCALY3 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float value = p . y;
 const numeric :: X87Scope extendedScope;
 const bool negative = signbit ( value );
 const numeric :: X87Real denominator = numeric :: x87Abs ( ( numeric :: X87Real ) value ) + params [ 0 ];
 value = float ( ( numeric :: X87Real ) params [ 1 ] - numeric :: X87Real ( 1.f ) / denominator );
 if ( negative ) value = - value;
 p . y = value;
 return p;

}
 FormulaVec3 _reciprocalY3_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__RECIPROCALY3 > ( p , params , de );

}

}}
