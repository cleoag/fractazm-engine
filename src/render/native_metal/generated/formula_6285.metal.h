// Generated from Ads/_vanderpol.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6285 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__VANDERPOL = 6285;
 template < > FormulaVec3 execute_formula < FORMULA__VANDERPOL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float Phi = params [ 1 ];
 const float Beta = params [ 2 ];
 const float Gamma = params [ 3 ];
 const float Delta = params [ 4 ];
 const float Alpha = params [ 5 ];
 const float r1 = params [ 6 ];
 const float r2 = params [ 7 ];
 float x = p . x , y = p . y , z = p . z , w = params [ FORMULA_STATE_W_SLOT ];
 float x_out = ( z + Beta * x ) * Strength;
 float y_out = ( w + Beta * y ) * Strength;
 float z_out = Gamma * y - Delta * x + ( Alpha - r1 * x * x ) * z;
 float w_out = Gamma * x - Delta * y + ( Phi - r2 * y * y ) * w;
 params [ FORMULA_STATE_W_SLOT ] = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
