// Generated from Ads/_BT3_Transform3.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7102 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BT3_TRANSFORM3 = 7102;
 template < > FormulaVec3 execute_formula < FORMULA__BT3_TRANSFORM3 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float Offset = params [ 1 ];
 const float c0 = 0.816496580927726f;
 const float c1 = 0.577350269189626f;
 const float c2 = 0.707106781186548f;
 float x = p . x , y = p . y , z = p . z;
 float t = ( x * c0 - z * c1 ) * c2;
 float y_s = y * c2;
 float R = x * c1 + z * c0;
 float A = abs ( t + y_s );
 float B = abs ( t - y_s );
 float C = abs ( R );
 float x3 = abs ( A + C - Offset ) * Scale * c2;
 float y3 = abs ( B + C - Offset ) * Scale * c2;
 float z3 = abs ( A + B - Offset ) * Scale;
 float s = x3 + y3;
 float d = y3 - x3;
 float new_x = s * c0 + z3 * c1;
 float new_z = z3 * c0 - s * c1;
 float new_y = d;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
