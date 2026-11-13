// Generated from 3D/Beth1522.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5304 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH1522 = 5304;
 template < > FormulaVec3 execute_formula < FORMULA_BETH1522 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float a = 0.15625f;
 const float b = 20.652475842498528f;
 const float c = - 10.652475842498528f;
 const float d = 2.0f;
 const float e = 30.0f;
 float x = p . x , y = p . y , z = p . z;
 float X = x * x;
 float Y = y * y;
 float Z = z * z;
 float X2 = X * X;
 float Y2 = Y * Y;
 float Z2 = Z * Z;
 float inner_x = e * Y * Z - ( c * Y + b * Z ) * X + X2;
 float poly_x = b * Y2 + c * Z2 - d * inner_x;
 float inner_y = e * Z * X - ( c * Z + b * X ) * Y + Y2;
 float poly_y = b * Z2 + c * X2 - d * inner_y;
 float inner_z = e * X * Y - ( c * X + b * Y ) * Z + Z2;
 float poly_z = b * X2 + c * Y2 - d * inner_z;
 float new_x = - ( a * x * poly_x );
 float new_y = - ( a * y * poly_y );
 float new_z = - ( a * z * poly_z );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
