// Generated from dIFSa/foldinginfyIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10315 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FOLDINGINFYIFS = 10315;
 template < > FormulaVec3 execute_formula < FORMULA_FOLDINGINFYIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float AddX = params [ 0 ];
 const float AddY = params [ 1 ];
 const float AddZ = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const float FoldX = params [ 12 ];
 const float FoldY = params [ 13 ];
 const float FoldZ = params [ 14 ];
 float x = p . z + AddZ;
 float y = p . y + AddY;
 float z = p . x + AddX;
 float a = p . x + AddX;
 float b = p . y + AddY;
 float c = p . z + AddZ;
 FormulaVec3 rv = R * FormulaVec3 ( a , b , c );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 struct NativeClosure0 {
 float operator ( ) ( float v , float fold ) const {
 float period = 4.0f * fold;
 float reduced = v - round ( v / period ) * period;
 reduced = - reduced;
 return reduced + fabs ( fold - reduced ) - fabs ( reduced + fold );

}

}
;
 auto foldinginfy = NativeClosure0 {

}
;
 if ( FoldX != 0.0f ) {
 rx = foldinginfy ( rx , FoldX );

}
 if ( FoldY != 0.0f ) {
 ry = foldinginfy ( ry , FoldY );

}
 if ( FoldZ != 0.0f ) {
 rz = foldinginfy ( rz , FoldZ );

}
 return FormulaVec3 ( rx , ry , rz );

}
 FormulaVec3 foldinginfyIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_FOLDINGINFYIFS > ( p , params , de );

}

}}
