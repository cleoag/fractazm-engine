// Generated from dIFSa/transform2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10243 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TRANSFORM2IFS = 10243;
 template < > FormulaVec3 execute_formula < FORMULA_TRANSFORM2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const int Abs_X = int ( params [ 12 ] );
 const int Abs_Y = int ( params [ 13 ] );
 const int Abs_Z = int ( params [ 14 ] );
 float x = p . x;
 float y = p . y;
 float z = p . z;
 if ( Abs_Z != 0 ) z = abs ( z );
 if ( Abs_Y != 0 ) y = abs ( y );
 if ( Abs_X != 0 ) x = abs ( x );
 FormulaVec3 rv = R * FormulaVec3 ( x , y , z );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 rx += Add_X;
 ry += Add_Y;
 rz += Add_Z;
 return FormulaVec3 ( rx , ry , rz );

}
 FormulaVec3 transform2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TRANSFORM2IFS > ( p , params , de );

}

}}
