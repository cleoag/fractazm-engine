// Generated from 3D/Makin3D-3-4.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7093 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN3D_3_4 = 7093;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN3D_3_4 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int Mode1 = int ( params [ 0 ] );
 const int Mode2 = int ( params [ 1 ] );
 const float Epsilon = params [ 2 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float new_x = x2 - y2 - z2;
 float z2_s;
 if ( Mode1 > 0 ) {
 z2_s = - z2;

}
 else if ( Mode1 == 0 ) {
 z2_s = z2;

}
 else {
 z2_s = ( z > 0.0f ) ? z2 : - z2;

}
 float denom_y = x2 + y2 + abs ( Epsilon );
 float new_y = 2.0f * x * y * ( 1.0f + z2_s / denom_y );
 float y2_s;
 if ( Mode2 > 0 ) {
 y2_s = - y2;

}
 else if ( Mode2 == 0 ) {
 y2_s = y2;

}
 else {
 y2_s = ( y > 0.0f ) ? y2 : - y2;

}
 float denom_z = x2 + z2 + abs ( Epsilon );
 float new_z = 2.0f * x * z * ( 1.0f + y2_s / denom_z );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
