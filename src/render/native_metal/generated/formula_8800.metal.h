// Generated from 4D/CommQuat.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8800 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_COMMQUAT = 8800;
 template < > FormulaVec3 execute_formula < FORMULA_COMMQUAT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float W_mul = params [ 3 ];
 const float W_add = params [ 4 ];
 const float Cx = params [ 5 ];
 const float Cy = params [ 6 ];
 const float Cz = params [ 7 ];
 const float Cw = params [ 8 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float x_out = X_mul * ( x * x - y * y - 2.0f * z * w ) + Cx;
 float y_out = Y_mul * ( 2.0f * x * y + z * z - w * w ) + Cy;
 float z_out = Z_mul * 2.0f * ( x * z - y * w ) + Cz;
 float w_out = W_mul * 2.0f * ( x * w + y * z ) + W_add + Cw;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
