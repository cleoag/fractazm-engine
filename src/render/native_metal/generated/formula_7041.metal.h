// Generated from Ads/_BRotateFromMag.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7041 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BROTATEFFROMMAG = 7041;
 template < > FormulaVec3 execute_formula < FORMULA__BROTATEFFROMMAG > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float c_sqrt_2_3 = 0.816496580927726f;
 const float c_sqrt_1_3 = 0.5773502691896258f;
 const float c_sqrt_1_2 = 0.7071067811865475f;
 float tx = ( p . x + p . y ) * c_sqrt_1_2;
 float new_y = ( p . y - p . x ) * c_sqrt_1_2;
 float new_x = tx * c_sqrt_2_3 + p . z * c_sqrt_1_3;
 float new_z = p . z * c_sqrt_2_3 - tx * c_sqrt_1_3;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
