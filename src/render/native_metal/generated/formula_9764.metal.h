// Generated from Ads/_lorenz.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9764 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__LORENZ = 9764;
 template < > FormulaVec3 execute_formula < FORMULA__LORENZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float Alpha = params [ 1 ];
 const float Beta = params [ 2 ];
 const float Gamma = params [ 3 ];
 const float R = params [ 4 ];
 float x = p . x , y = p . y , z = p . z , w = params [ FORMULA_STATE_W_SLOT ];
 float x_out = ( Alpha * ( y - x ) + w ) * Strength;
 float y_out = ( Gamma * x - y - x * z ) * Strength;
 float z_out = ( x * y - Beta * z ) * Strength;
 float w_out = ( R * w - y * z ) * Strength;
 params [ FORMULA_STATE_W_SLOT ] = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
