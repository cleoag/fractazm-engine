// Generated from Ads/_BRotateToMag.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7047 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BROTATETOMAG = 7047;
 template < > FormulaVec3 execute_formula < FORMULA__BROTATETOMAG > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float a = 0.816496580927726f;
 const float b = 0.5773502691896258f;
 const float c = 0.7071067811865475f;
 float tx = ( p . x * a - p . z * b ) * c;
 float ty = p . y * c;
 float new_x = tx - ty;
 float new_y = tx + ty;
 float new_z = p . x * b + p . z * a;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
