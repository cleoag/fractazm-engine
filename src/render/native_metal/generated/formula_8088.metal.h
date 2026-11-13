// Generated from 3D/BenesiPine2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8088 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BENESIPINE2 = 8088;
 template < > FormulaVec3 execute_formula < FORMULA_BENESIPINE2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float xOff = params [ 1 ];
 const float p0 = 0.816496580927726f;
 const float p1 = 0.577350269189626f;
 const float p2 = 0.707106781186548f;
 float x = p . x , y = p . y , z = p . z;
 float tx = ( x * p0 - z * p1 ) * p2;
 y = y * p2;
 z = x * p1 + z * p0;
 z = z * z;
 float tx2 = ( tx - y ) * ( tx - y );
 float ty2 = ( tx + y ) * ( tx + y );
 x = fabs ( sqrt ( ty2 + z ) - xOff ) * Scale * p2;
 y = fabs ( sqrt ( tx2 + z ) - xOff ) * Scale * p2;
 z = fabs ( sqrt ( tx2 + ty2 ) - xOff ) * Scale;
 tx = y + x;
 y = y - x;
 x = tx * p0 + z * p1;
 z = z * p0 - tx * p1;
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float K = 2.0f * x / sqrt ( y2 + z2 );
 float new_x = x2 - y2 - z2;
 float new_y = 2.0f * K * y * z;
 float new_z = K * ( y2 - z2 );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
