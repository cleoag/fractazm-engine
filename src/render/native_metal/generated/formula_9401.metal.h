// Generated from 4D/Makin4D_p2a.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9401 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN4D_P2A = 9401;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN4D_P2A > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float W_add = params [ 0 ];
 float x = p . x , y = p . y , z = p . z;
 float w = de;
 float P = 2.0f * ( x * z - y * w );
 float Q = 2.0f * ( x * w + y * z );
 float A = x * x + y * y;
 float B = z * z + w * w;
 float x_out , y_out;
 if ( B < A ) {
 float scale = ( A - B ) / A;
 x_out = ( x * x - y * y ) * scale;
 y_out = 2.0f * x * y * scale;

}
 else {
 float scale = ( A - B ) / B;
 x_out = ( z * z - w * w ) * scale;
 y_out = 2.0f * z * w * scale;

}
 float z_out = P;
 float w_out = Q + W_add;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
