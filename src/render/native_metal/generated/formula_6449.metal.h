// Generated from 4D/Lambda4Dnc.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6449 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_LAMBDA4DNC = 6449;
 template < > FormulaVec3 execute_formula < FORMULA_LAMBDA4DNC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float P0 = params [ 0 ];
 const float P1 = params [ 1 ];
 const float P2 = params [ 2 ];
 const float P3 = params [ 3 ];
 const float Cx = params [ 4 ];
 const float Cy = params [ 5 ];
 const float Cz = params [ 6 ];
 const float Cw = params [ 7 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float qx = - x * x + y * y + z * z + w * w + P0;
 float qy = - 2.0f * x * y + P1;
 float qz = - 2.0f * x * z + P2;
 float qw = - 2.0f * x * w + P3;
 float x_out = qx * Cx - qy * Cy - qz * Cz - qw * Cw;
 float y_out = qx * Cy + qy * Cx + qz * Cw - qw * Cz;
 float z_out = qx * Cz - qy * Cw + qz * Cx + qw * Cy;
 float w_out = qx * Cw + qy * Cz - qz * Cy + qw * Cx;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
