// Generated from 4D/Hopf4D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9306 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HOPF4D = 9306;
 template < > FormulaVec3 execute_formula < FORMULA_HOPF4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float W_add = params [ 0 ];
 const float Z_mul = params [ 1 ];
 const float Cx = params [ 2 ];
 const float Cy = params [ 3 ];
 const float Cz = params [ 4 ];
 const float Cw = params [ 5 ];
 const float x = p . x;
 const float y = p . y;
 const float z = p . z;
 const float w = de;
 const float A = x * x + y * y;
 const float B = z * z + w * w;
 const float scale1 = 1.0f - B / A;
 const float scale2 = 2.0f * sqrt ( A / B );
 const float x_out = ( x * x - y * y ) * scale1 + Cx;
 const float y_out = 2.0f * x * y * scale1 + Cy;
 const float z_out = ( z * z - w * w ) * scale2 * Z_mul + Cz;
 const float w_out = z * w * scale2 + Cw + W_add;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
