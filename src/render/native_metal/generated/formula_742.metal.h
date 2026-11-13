// Generated from 3D/Beth323.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_742 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH323 = 742;
 template < > FormulaVec3 execute_formula < FORMULA_BETH323 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float a = 21.0f;
 const float b = - 35.0f;
 const float c = 7.0f;
 float x = p . x , y = p . y , z = p . z;
 float X = x * x;
 float Y = y * y;
 float Z = z * z;
 float X2 = X * X;
 float Y2 = Y * Y;
 float Z2 = Z * Z;
 float X3 = X2 * X;
 float Y3 = Y2 * Y;
 float Z3 = Z2 * Z;
 float poly_x = a * ( Y - Z ) * X2 + b * ( Y2 - Z2 ) * X + c * ( Y3 - Z3 );
 float poly_y = a * ( Z - X ) * Y2 + b * ( Z2 - X2 ) * Y + c * ( Z3 - X3 );
 float poly_z = a * ( X - Y ) * Z2 + b * ( X2 - Y2 ) * Z + c * ( X3 - Y3 );
 float new_x = x * poly_x;
 float new_y = y * poly_y;
 float new_z = z * poly_z;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
