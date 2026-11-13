// Generated from Ads/_gnarl2D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7063 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__GNARL2D = 7063;
 template < > FormulaVec3 execute_formula < FORMULA__GNARL2D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Strength = params [ 0 ];
 const float X_step = params [ 1 ];
 const float Y_step = params [ 2 ];
 const float Alpha = params [ 3 ];
 const float Beta = params [ 4 ];
 float x = p . x , y = p . y;
 float gy = sin ( Beta * y );
 gy = sin ( Alpha * ( y + gy ) );
 gy = sin ( y + gy );
 float gx = sin ( Beta * x );
 gx = sin ( Alpha * ( x + gx ) );
 gx = sin ( x + gx );
 float new_x = ( x - X_step * gy ) * Strength;
 float new_y = ( y - Y_step * gx ) * Strength;
 return FormulaVec3 ( new_x , new_y , p . z );

}

}}
