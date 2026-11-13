// Generated from 4D/Makin4D_p4b.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8679 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN4D_P4B = 8679;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN4D_P4B > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float W_add = params [ 0 ];
 const float Cx = params [ 1 ];
 const float Cy = params [ 2 ];
 const float Cz = params [ 3 ];
 const float Cw = params [ 4 ];
 float x = p . x , y = p . y , z = p . z;
 float w = de;
 {
 float r2 = x * x + y * y + z * z + w * w;
 float P = 2.0f * ( x * z - y * w );
 float Q = 2.0f * ( x * w + y * z );
 float A = 2.0f * ( x * x - y * y );
 float B = 4.0f * x * y;
 float r_xy = sqrt ( A * A + B * B );
 float ratio = 1.0f - r2 / r_xy;
 x = A * ratio;
 y = B * ratio;
 z = P;
 w = Q;

}
 {
 float r2 = x * x + y * y + z * z + w * w;
 float P = 2.0f * ( x * z - y * w );
 float Q = 2.0f * ( x * w + y * z );
 float A = 2.0f * ( x * x - y * y );
 float B = 4.0f * x * y;
 float r_xy = sqrt ( A * A + B * B );
 float ratio = 1.0f - r2 / r_xy;
 x = A * ratio;
 y = B * ratio;
 z = P;
 w = Q;

}
 x += Cx;
 y += Cy;
 z += Cz;
 w += Cw + W_add;
 de = w;
 return FormulaVec3 ( x , y , z );

}

}}
