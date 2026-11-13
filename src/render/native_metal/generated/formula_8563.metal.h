// Generated from 4D/_SphereInvC4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8563 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SPHEREINVC4D = 8563;
 template < > FormulaVec3 execute_formula < FORMULA__SPHEREINVC4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float cx = params [ 0 ];
 const float cy = params [ 1 ];
 const float cz = params [ 2 ];
 const float cw = params [ 3 ];
 float c_sq = cx * cx + cy * cy + cz * cz + cw * cw;
 if ( c_sq > 0.0f ) {
 float c_inv = 1.0f / c_sq;
 params [ WRITE_J1_SLOT ] = cx * c_inv;
 params [ WRITE_J2_SLOT ] = cy * c_inv;
 params [ WRITE_J3_SLOT ] = cz * c_inv;
 params [ WRITE_J4_SLOT ] = cw * c_inv;

}
 float w = de;
 float r_sq = p . x * p . x + p . y * p . y + p . z * p . z + w * w;
 float inv = 1.0f / r_sq;
 de = w * inv;
 return p * inv;

}
 FormulaVec3 _SphereInvC4d_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__SPHEREINVC4D > ( p , params , de );

}

}}
