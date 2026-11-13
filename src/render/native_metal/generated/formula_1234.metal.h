// Generated from Ads/_NeoSqrX.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1234 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__NEOSQRX = 1234;
 template < > FormulaVec3 execute_formula < FORMULA__NEOSQRX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixX = params [ 0 ];
 const float fixSq = params [ 1 ];
 const float Mul = params [ 2 ];
 const float Div = params [ 3 ];
 float x = p . x;
 float A = Mul * x + fixX;
 float neo;
 if ( A >= 0.0f ) {
 neo = A * ( A - fixSq );

}
 else {
 neo = A * ( fixSq - A );

}
 float new_x = neo / Div - fixX;
 return FormulaVec3 ( new_x , p . y , p . z );

}
 FormulaVec3 _NeoSqrX_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float fX = params [ 0 ] , fS = params [ 1 ] , M = params [ 2 ] , D = params [ 3 ];
 float A = M * p . x + fX;
 float neo = ( A >= 0.0f ) ? A * ( A - fS ) : A * ( fS - A );
 return FormulaVec3 ( neo / D - fX , p . y , p . z );

}

}}
