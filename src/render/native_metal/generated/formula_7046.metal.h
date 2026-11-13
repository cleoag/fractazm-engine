// Generated from 3D/IdesFormula.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7046 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_IDESFORMULA = 7046;
 template < > FormulaVec3 execute_formula < FORMULA_IDESFORMULA > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = p . x , y = p . y , z = p . z;
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float Xsub_mul = params [ 3 ];
 const float Zsub_mul = params [ 4 ];
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float new_x = X_mul * x2 - ( y2 + z2 ) * Xsub_mul;
 float new_y = Y_mul * x * y * z;
 float new_z = Z_mul * z2 - ( x2 + y2 ) * Zsub_mul;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
