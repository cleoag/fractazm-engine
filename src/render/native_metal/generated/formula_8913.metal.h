// Generated from 3D/Beth651.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8913 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH651 = 8913;
 template < > FormulaVec3 execute_formula < FORMULA_BETH651 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float p0 = - 2.7639320225002103f;
 const float p1 = 1.2360679774997897f;
 const float p2 = 2.0f;
 const float p3 = 2.2360679774997897f;
 const float p4 = 2.4f;
 const float p5 = 3.2360679774997897f;
 const float p6 = 4.4721359549995794f;
 const float p7 = 7.2360679774997897f;
 float x = p . x , y = p . y , z = p . z;
 float X = x * x;
 float Y = y * y;
 float Z = z * z;
 float X2 = X * X;
 float Y2 = Y * Y;
 float Z2 = Z * Z;
 float inner_x = p6 * Y * Z - ( p7 * Y + p0 * Z ) * X + p3 * X2;
 float poly_x = p1 * Y2 + p5 * Z2 - p2 * inner_x;
 float new_x = p4 * y * z * poly_x;
 float inner_y = p6 * Z * X - ( p7 * Z + p0 * X ) * Y + p3 * Y2;
 float poly_y = p1 * Z2 + p5 * X2 - p2 * inner_y;
 float new_y = p4 * z * x * poly_y;
 float inner_z = p6 * X * Y - ( p7 * X + p0 * Y ) * Z + p3 * Z2;
 float poly_z = p1 * X2 + p5 * Y2 - p2 * inner_z;
 float new_z = p4 * x * y * poly_z;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
