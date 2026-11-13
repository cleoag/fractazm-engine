// Generated from Ads/_vanderpol2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9222 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__VANDERPOL2 = 9222;
 template < > FormulaVec3 execute_formula < FORMULA__VANDERPOL2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float Alpha = params [ 1 ];
 const float Beta = params [ 2 ];
 const float Gamma = params [ 3 ];
 const float Delta = params [ 4 ];
 const float Epsilon = params [ 5 ];
 const float Phi = params [ 6 ];
 const float Rho = params [ 7 ];
 const float r = params [ 8 ];
 float x = p . x , y = p . y , z = p . z , w = params [ FORMULA_STATE_W_SLOT ];
 float x2 = x * x;
 float x_out = ( Rho * x + Alpha * y + Beta * z + Gamma * w ) * Strength;
 float y_out = Delta * z * w - Alpha * x + ( Alpha - r * x2 ) * y;
 float z_out = Epsilon * y * w - Beta * x + ( Beta - r * x2 ) * z;
 float w_out = Phi * y * z - Gamma * x + ( Gamma - r * x2 ) * w;
 params [ FORMULA_STATE_W_SLOT ] = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
