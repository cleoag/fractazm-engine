// Generated from Ads/_SphereInvC.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7922 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SPHEREINVC = 7922;
 template < > FormulaVec3 execute_formula < FORMULA__SPHEREINVC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float c_sq = params [ 0 ] * params [ 0 ] + params [ 1 ] * params [ 1 ] + params [ 2 ] * params [ 2 ];
 if ( c_sq > 0.0f ) {
 float c_inv = 1.0f / c_sq;
 params [ WRITE_J1_SLOT ] = params [ 0 ] * c_inv;
 params [ WRITE_J2_SLOT ] = params [ 1 ] * c_inv;
 params [ WRITE_J3_SLOT ] = params [ 2 ] * c_inv;

}
 float r_sq = p . x * p . x + p . y * p . y + p . z * p . z;
 float inv = 1.0f / r_sq;
 return p * inv;

}
 FormulaVec3 _SphereInvC_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float c_sq = params [ 0 ] * params [ 0 ] + params [ 1 ] * params [ 1 ] + params [ 2 ] * params [ 2 ];
 if ( c_sq > 0.0f ) {
 float c_inv = 1.0f / c_sq;
 params [ WRITE_J1_SLOT ] = params [ 0 ] * c_inv;
 params [ WRITE_J2_SLOT ] = params [ 1 ] * c_inv;
 params [ WRITE_J3_SLOT ] = params [ 2 ] * c_inv;

}
 float r_sq = p . x * p . x + p . y * p . y + p . z * p . z;
 float inv = 1.0f / r_sq;
 return p * inv;

}

}}
