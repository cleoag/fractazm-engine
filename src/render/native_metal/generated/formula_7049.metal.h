// Generated from Ads/_hopalm2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7049 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__HOPALM2 = 7049;
 template < > FormulaVec3 execute_formula < FORMULA__HOPALM2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = p . x , y = p . y , z = p . z;
 const float Strength = params [ 0 ];
 const float Alpha = params [ 1 ];
 const float Beta = params [ 2 ];
 const float Gamma = params [ 3 ];
 const float Delta = params [ 4 ];
 const float Epsilon = params [ 5 ];
 float t = Beta * x - Gamma * z + Delta;
 float new_x = y + atan ( t );
 float new_y = ( Alpha - x ) * Strength;
 float new_z = z + ( Alpha * y - Beta * x ) * Epsilon;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
