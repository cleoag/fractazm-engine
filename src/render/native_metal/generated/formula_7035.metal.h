// Generated from 3D/Makin3D-1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7035 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN3D_1 = 7035;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN3D_1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = p . x , y = p . y , z = p . z;
 float new_x = x * x - y * y - z * z;
 float new_y = 2.0f * x * y;
 float new_z = 2.0f * z * ( x - y );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
