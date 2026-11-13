// Generated from 4D/Makin4D_p2b.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7653 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN4D_P2B = 7653;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN4D_P2B > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float W_add = params [ 0 ];
 const float Cx = params [ 1 ];
 const float Cy = params [ 2 ];
 const float Cz = params [ 3 ];
 const float Cw = params [ 4 ];
 const float x = p . x;
 const float y = p . y;
 const float z = p . z;
 const float w = de;
 const float A = x * x + y * y;
 const float r2 = A + z * z + w * w;
 const float scale = ( 2.0f * A - r2 ) / ( 2.0f * A );
 const float x_out = 2.0f * ( x * x - y * y ) * scale + Cx;
 const float y_out = 4.0f * x * y * scale + Cy;
 const float z_out = 2.0f * ( x * z - y * w ) + Cz;
 const float w_out = 2.0f * ( x * w + y * z ) + Cw + W_add;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
