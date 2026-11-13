// Generated from Ads/_gnarl2D_2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7064 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__GNARL2D_2 = 7064;
 template < > FormulaVec3 execute_formula < FORMULA__GNARL2D_2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float X_step = params [ 1 ];
 const float Alpha = params [ 3 ];
 const float Beta = params [ 4 ];
 const float Gamma = params [ 5 ];
 float x = p . x , y = p . y;
 float t1 = sin ( x + cos ( x ) * Beta ) * Alpha;
 float new_x = ( y + X_step * sin ( ( y + x ) * Gamma - t1 ) ) * Strength;
 float t2 = sin ( ( y + cos ( y ) * Beta ) * Alpha );
 float new_y = ( x + X_step * sin ( t2 + ( x - y ) * Gamma ) ) * Strength;
 return FormulaVec3 ( new_x , new_y , p . z );

}

}}
