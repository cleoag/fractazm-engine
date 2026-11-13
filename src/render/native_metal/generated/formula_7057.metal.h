// Generated from 3D/Faehrten.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7057 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FAEHRTEN = 7057;
 template < > FormulaVec3 execute_formula < FORMULA_FAEHRTEN > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Y_mul = params [ 0 ];
 const float Z_mul = params [ 1 ];
 const float Sq_mul = params [ 2 ];
 const float Twist = params [ 3 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float new_x = x * ( y * Y_mul + z * Z_mul );
 float new_y = y2 + ( x2 + z2 ) * Sq_mul + x * z * Twist;
 float new_z = z2 + ( x2 + y2 ) * Sq_mul + x * y * Twist;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
