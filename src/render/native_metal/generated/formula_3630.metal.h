// Generated from 4D/CubicQuat.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3630 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_CUBICQUAT = 3630;
 template < > FormulaVec3 execute_formula < FORMULA_CUBICQUAT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float W_mul = params [ 3 ];
 const float W_add = params [ 4 ];
 const float X_twist = params [ 5 ];
 const float Y_twist = params [ 6 ];
 const float Cx = params [ 7 ];
 const float Cy = params [ 8 ];
 const float Cz = params [ 9 ];
 const float Cw = params [ 10 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float x_out = X_mul * x * x - ( Y_mul * y * y + Z_mul * z * z + W_mul * w * w ) + Cx + Cy * X_twist;
 float y_out = 2.0f * Y_mul * x * y + Cy + Cx * Y_twist;
 float z_out = 2.0f * Z_mul * x * z + Cz;
 float w_out = 2.0f * W_mul * x * w + W_add + Cw;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
