// Generated from Ads/_CelticMode.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7058 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__CELTICMODE = 7058;
 template < > FormulaVec3 execute_formula < FORMULA__CELTICMODE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Good_rune = params [ 0 ];
 const float Evil_rune = params [ 1 ];
 const float Mystic_rune = params [ 2 ];
 const float Heretic_rune = params [ 3 ];
 const float Cx = params [ 4 ];
 const float Cz = params [ 6 ];
 float new_x = ( fabs ( p . x + Cx * Good_rune ) - Cx * Good_rune ) + p . x * Evil_rune;
 float new_z = p . z;
 if ( Mystic_rune != 0.0f ) {
 new_z = fabs ( p . z + Cz * Mystic_rune ) - Cz * Heretic_rune;

}
 return FormulaVec3 ( new_x , p . y , new_z );

}

}}
