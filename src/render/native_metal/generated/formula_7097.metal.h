// Generated from Ads/_BT1_Transform1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7097 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BT1_TRANSFORM1 = 7097;
 template < > FormulaVec3 execute_formula < FORMULA__BT1_TRANSFORM1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float Offset = params [ 1 ];
 const float c0 = 0.816496580927726f;
 const float c1 = 0.577350269189626f;
 const float c2 = 0.707106781186548f;
 float x = p . x , y = p . y , z = p . z;
 float t = ( x * c0 - z * c1 ) * c2;
 float z1 = abs ( x * c1 + z * c0 );
 float x1 = abs ( t - y * c2 );
 float y1 = abs ( t + y * c2 );
 float s = ( x1 + y1 ) * c2;
 float d = ( y1 - x1 ) * c2;
 float fx = s * c0 + z1 * c1;
 float fz = z1 * c0 - s * c1;
 float fy = d;
 float new_x = fx * Scale - Offset;
 float new_y = fy * Scale;
 float new_z = fz * Scale;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
