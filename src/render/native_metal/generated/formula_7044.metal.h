// Generated from Ads/_totorical2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7044 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__TOTORICAL2 = 7044;
 template < > FormulaVec3 execute_formula < FORMULA__TOTORICAL2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float R = params [ 0 ];
 const float Gamma = params [ 1 ];
 float t = sqrt ( p . x * p . x + p . y * p . y ) - R;
 float new_x = sqrt ( t * t + p . z * p . z );
 float new_y = atan2 ( t , p . z ) - Gamma - M_PI_2_F;
 float new_z = atan2 ( p . y , p . x );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
