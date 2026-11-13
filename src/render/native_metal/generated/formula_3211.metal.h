// Generated from 3D/BenesiPine1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3211 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BENESIPINE1 = 3211;
 template < > FormulaVec3 execute_formula < FORMULA_BENESIPINE1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float xOff = params [ 1 ];
 const float p0 = 0.816496580927726f;
 const float p1 = 0.577350269189626f;
 const float p2 = 0.707106781186548f;
 float x = p . x , y = p . y , z = p . z;
 float tx = ( x * p0 - z * p1 ) * p2;
 z = fabs ( x * p1 + z * p0 );
 x = fabs ( tx - y * p2 );
 y = fabs ( tx + y * p2 );
 tx = ( x + y ) * p2;
 y = ( y - x ) * p2;
 x = tx * p0 + z * p1;
 z = z * p0 - tx * p1;
 x = x * Scale - xOff;
 y = y * Scale;
 z = z * Scale;
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
