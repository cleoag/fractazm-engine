// Generated from Ads/_FoldingInfy.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10015 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDINGINFY = 10015;
 template < > FormulaVec3 execute_formula < FORMULA__FOLDINGINFY > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float xFold = params [ 0 ];
 float yFold = params [ 1 ];
 float zFold = params [ 2 ];
 float x = p . x;
 float y = p . y;
 float z = p . z;
 if ( zFold != 0.0f ) {
 float m = zFold * 4.0f;
 float n = zFold;
 float t = z - round ( z / m ) * m;
 z = fabs ( t + n ) - fabs ( t - n ) - t;

}
 if ( yFold != 0.0f ) {
 float m = yFold * 4.0f;
 float n = yFold;
 float t = y - round ( y / m ) * m;
 y = fabs ( t + n ) - fabs ( t - n ) - t;

}
 if ( xFold != 0.0f ) {
 float m = xFold * 4.0f;
 float n = xFold;
 float t = x - round ( x / m ) * m;
 x = fabs ( t + n ) - fabs ( t - n ) - t;

}
 return FormulaVec3 ( x , y , z );

}

}}
