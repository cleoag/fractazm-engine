// Generated from 3D/Beth322.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7079 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH322 = 7079;
 template < > FormulaVec3 execute_formula < FORMULA_BETH322 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float K = 5.0f;
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float x4 = x2 * x2 , y4 = y2 * y2 , z4 = z2 * z2;
 float Qx = 2.0f * x2 * ( y2 + z2 ) - ( y4 + z4 );
 float Qy = 2.0f * y2 * ( x2 + z2 ) - ( x4 + z4 );
 float Qz = 2.0f * z2 * ( x2 + y2 ) - ( x4 + y4 );
 float new_x = x * ( 2.0f * x4 - K * Qx );
 float new_y = y * ( 2.0f * y4 - K * Qy );
 float new_z = z * ( 2.0f * z4 - K * Qz );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
