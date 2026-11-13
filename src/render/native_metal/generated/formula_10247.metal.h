// Generated from dIFSa/Menger1-IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10247 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGER1_IFS = 10247;
 template < > FormulaVec3 execute_formula < FORMULA_MENGER1_IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScale_X = params [ 1 ];
 const float CScale_Y = params [ 2 ];
 const float CScale_Z = params [ 3 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 4 );
 float x = abs ( p . x );
 float y = abs ( p . y );
 float z = abs ( p . z );
 if ( x < y ) {
 float t = x;
 x = y;
 y = t;

}
 if ( x < z ) {
 float t = x;
 x = z;
 z = t;

}
 if ( y < z ) {
 float t = y;
 y = z;
 z = t;

}
 FormulaVec3 rv = R * FormulaVec3 ( x , y , z );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 rx *= Scale;
 ry *= Scale;
 float t = Scale - 1.0f;
 rx -= t * CScale_X;
 ry -= t * CScale_Y;
 float c = ( t / Scale ) * CScale_Z;
 float rz_new = ( c - abs ( rz - c ) ) * Scale;
 return FormulaVec3 ( rx , ry , rz_new );

}
 FormulaVec3 Menger1_IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_MENGER1_IFS > ( p , params , de );

}

}}
