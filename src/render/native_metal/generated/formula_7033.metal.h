// Generated from Ads/_NeoSqrY.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7033 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__NEOSQRY = 7033;
 template < > FormulaVec3 execute_formula < FORMULA__NEOSQRY > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixY = params [ 0 ];
 const float fixSq = params [ 1 ];
 const float Mul = params [ 2 ];
 const float Div = params [ 3 ];
 float A = Mul * p . y + fixY;
 float neoSqr;
 if ( A >= 0.0f ) {
 neoSqr = A * ( A - fixSq );

}
 else {
 neoSqr = A * ( fixSq - A );

}
 float result = neoSqr / Div - fixY;
 return FormulaVec3 ( p . x , result , p . z );

}

}}
