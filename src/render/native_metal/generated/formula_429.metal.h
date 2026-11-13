// Generated from 3Da/genIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_429 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GENIFS = 429;
 template < > FormulaVec3 execute_formula < FORMULA_GENIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const int abs_x = ( int ) params [ 1 ];
 const int abs_y = ( int ) params [ 2 ];
 const int abs_z = ( int ) params [ 3 ];
 const float offset_x = params [ 4 ];
 const float offset_y = params [ 5 ];
 const float offset_z = params [ 6 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 7 );
 float x = ( abs_x != 0 ) ? abs ( p . x ) : p . x;
 float y = ( abs_y != 0 ) ? abs ( p . y ) : p . y;
 float z = ( abs_z != 0 ) ? abs ( p . z ) : p . z;
 x += offset_x;
 y += offset_y;
 z += offset_z;
 FormulaVec3 rv = R * FormulaVec3 ( x , y , z );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 FormulaVec3 result;
 result . x = rx * Scale;
 result . y = ry * Scale;
 result . z = rz * Scale;
 de *= Scale;
 return result;

}
 FormulaVec3 genIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_GENIFS > ( p , params , de );

}

}}
