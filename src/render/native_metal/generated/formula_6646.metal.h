// Generated from 3D/Beth432.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6646 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH432 = 6646;
 template < > FormulaVec3 execute_formula < FORMULA_BETH432 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float a = - 0.395061728395061728395f;
 const float b = - 12.0f;
 const float c = 14.0f;
 const float d = - 21.0f;
 const float e = - 35.0f;
 const float f = - 105.0f;
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float x4 = x2 * x2 , y4 = y2 * y2 , z4 = z2 * z2;
 float x6 = x4 * x2 , y6 = y4 * y2 , z6 = z4 * z2;
 float poly_x = f * ( y4 * z2 + y2 * z4 ) + e * x2 * ( b * y2 * z2 + y4 + z4 ) + d * x4 * ( y2 + z2 ) + 2.0f * x6 + c * ( y6 + z6 );
 float poly_y = f * ( z4 * x2 + z2 * x4 ) + e * y2 * ( b * z2 * x2 + z4 + x4 ) + d * y4 * ( z2 + x2 ) + 2.0f * y6 + c * ( z6 + x6 );
 float poly_z = f * ( x4 * y2 + x2 * y4 ) + e * z2 * ( b * x2 * y2 + x4 + y4 ) + d * z4 * ( x2 + y2 ) + 2.0f * z6 + c * ( x6 + y6 );
 float new_x = a * x * poly_x;
 float new_y = a * y * poly_y;
 float new_z = a * z * poly_z;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
