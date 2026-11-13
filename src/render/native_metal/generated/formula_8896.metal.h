// Generated from Ads/_Transform.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8896 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__TRANSFORM = 8896;
 template < > FormulaVec3 execute_formula < FORMULA__TRANSFORM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float X_add = params [ 1 ];
 const float Y_add = params [ 2 ];
 const float Z_add = params [ 3 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 4 );
 float x = p . x + X_add;
 float y = p . y + Y_add;
 float z = p . z + Z_add;
 FormulaVec3 rv = R * FormulaVec3 ( x , y , z );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 FormulaVec3 result;
 result . x = rx * Scale;
 result . y = ry * Scale;
 result . z = rz * Scale;
 params [ FORMULA_STATE_W_SLOT ] *= Scale;
 params [ FORMULA_STATE_X_SLOT ] *= Scale;
 params [ WRITE_J1_SLOT ] = params [ 14 ];
 params [ WRITE_J2_SLOT ] = params [ 15 ];
 params [ WRITE_J3_SLOT ] = params [ 16 ];
 if ( params [ 13 ] != 0.0f && params [ 20 ] == 0.0f ) {
 params [ WRITE_J1_SLOT ] = result . x;
 params [ WRITE_J2_SLOT ] = result . y;
 params [ WRITE_J3_SLOT ] = result . z;

}
 return result;

}

}}
