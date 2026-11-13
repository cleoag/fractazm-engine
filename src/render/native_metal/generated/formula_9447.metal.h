// Generated from Ads/_SinePow2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9447 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SINEPOW2 = 9447;
 template < > FormulaVec3 execute_formula < FORMULA__SINEPOW2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Y_mul = params [ 0 ];
 const float Z_mul = params [ 1 ];
 float x = p . x , y = p . y , z = p . z;
 float x_sq = x * x;
 float y_sq = y * y;
 float r_xy_sq = x_sq + y_sq;
 float r_xy = sqrt ( r_xy_sq );
 float t = ( r_xy_sq - z * z ) / r_xy_sq;
 FormulaVec3 result;
 result . x = ( x_sq - y_sq ) * t;
 result . y = 2.0f * x * y * t * Y_mul;
 result . z = 2.0f * r_xy * z * Z_mul;
 return result;

}
 FormulaVec3 _SinePow2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Y_mul = params [ 0 ];
 const float Z_mul = params [ 1 ];
 float x = p . x , y = p . y , z = p . z;
 float x_sq = x * x;
 float y_sq = y * y;
 float r_xy_sq = x_sq + y_sq;
 float r_xy = sqrt ( r_xy_sq );
 float t = ( r_xy_sq - z * z ) / r_xy_sq;
 FormulaVec3 result;
 result . x = ( x_sq - y_sq ) * t;
 result . y = 2.0f * x * y * t * Y_mul;
 result . z = 2.0f * r_xy * z * Z_mul;
 return result;

}

}}
