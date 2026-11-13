// Generated from Ads/_BT5b_Transform5b.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7104 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BT5B_TRANSFORM5B = 7104;
 template < > FormulaVec3 execute_formula < FORMULA__BT5B_TRANSFORM5B > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float Offset = params [ 1 ];
 const float c0 = 0.816496580927726f;
 const float c1 = 0.577350269189626f;
 const float c2 = 0.707106781186548f;
 float x = p . x , y = p . y , z = p . z;
 float t = ( x * c0 - z * c1 ) * c2;
 float y_s = y * c2;
 float R = x * c1 + z * c0;
 float tp = t + y_s;
 float tp2 = tp * tp;
 float tp4 = tp2 * tp2;
 float A = tp4 * tp4;
 float tm = t - y_s;
 float tm2 = tm * tm;
 float tm4 = tm2 * tm2;
 float B = tm4 * tm4;
 float R2 = R * R;
 float R4 = R2 * R2;
 float C = R4 * R4;
 float x5 = abs ( sqrt ( sqrt ( A + C ) ) - Offset ) * Scale * c2;
 float y5 = abs ( sqrt ( sqrt ( B + C ) ) - Offset ) * Scale * c2;
 float z5 = abs ( sqrt ( sqrt ( A + B ) ) - Offset ) * Scale;
 float s = x5 + y5;
 float d = y5 - x5;
 float new_x = s * c0 + z5 * c1;
 float new_z = z5 * c0 - s * c1;
 float new_y = d;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
