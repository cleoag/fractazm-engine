// Generated from Ads/_BT2_Transform2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7101 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BT2_TRANSFORM2 = 7101;
 template < > FormulaVec3 execute_formula < FORMULA__BT2_TRANSFORM2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float Offset = params [ 1 ];
 const float c0 = 0.816496580927726f;
 const float c1 = 0.577350269189626f;
 const float c2 = 0.707106781186548f;
 float x = p . x , y = p . y , z = p . z;
 float t = ( x * c0 - z * c1 ) * c2;
 float y_s = y * c2;
 float R = x * c1 + z * c0;
 float A = ( t + y_s ) * ( t + y_s );
 float B = ( t - y_s ) * ( t - y_s );
 float C = R * R;
 float x2 = abs ( sqrt ( A + C ) - Offset ) * Scale * c2;
 float y2 = abs ( sqrt ( B + C ) - Offset ) * Scale * c2;
 float z2 = abs ( sqrt ( A + B ) - Offset ) * Scale;
 float s = x2 + y2;
 float d = y2 - x2;
 float new_x = s * c0 + z2 * c1;
 float new_z = z2 * c0 - s * c1;
 float new_y = d;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
