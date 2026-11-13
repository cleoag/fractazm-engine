// Generated from 3D/_quadist.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7078 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__QUADIST = 7078;
 template < > FormulaVec3 execute_formula < FORMULA__QUADIST > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Linear = params [ 0 ];
 const float a = params [ 1 ];
 const float b = params [ 2 ];
 const float c = params [ 3 ];
 const float d = params [ 4 ];
 const float e = params [ 5 ];
 const float f = params [ 6 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float new_x = x * Linear + ( d - a ) * x2 + ( e - b ) * y2 + ( f - c ) * z2;
 float new_y = y * Linear + ( f - a ) * x2 + ( d - b ) * y2 + ( e - c ) * z2;
 float new_z = z * Linear + ( e - a ) * x2 + ( f - b ) * y2 + ( d - c ) * z2;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
