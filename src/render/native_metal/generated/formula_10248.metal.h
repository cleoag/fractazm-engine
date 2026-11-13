// Generated from dIFSa/foldingoctIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10248 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FOLDINGOCTIFS = 10248;
 template < > FormulaVec3 execute_formula < FORMULA_FOLDINGOCTIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const int CtrlX = int ( params [ 12 ] );
 const int CtrlY = int ( params [ 13 ] );
 const int CtrlZ = int ( params [ 14 ] );
 float z_val = p . z + Add_Z;
 float y_val = p . y + Add_Y;
 float x_val = p . x + Add_X;
 FormulaVec3 rv = R * FormulaVec3 ( x_val , y_val , z_val );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 if ( ( CtrlX & 1 ) != 0 ) rx = abs ( rx );
 if ( ( CtrlY & 1 ) != 0 ) ry = abs ( ry );
 if ( ( CtrlZ & 1 ) != 0 ) rz = abs ( rz );
 if ( ( CtrlX & 2 ) != 0 ) {
 if ( ! ( rx > ry ) ) {
 float t = rx;
 rx = ry;
 ry = t;

}

}
 if ( ( CtrlY & 2 ) != 0 ) {
 if ( ! ( rx > rz ) ) {
 float t = rx;
 rx = rz;
 rz = t;

}

}
 {
 float t = rx;
 rx = ry;
 ry = t;

}
 if ( ( CtrlZ & 2 ) != 0 ) {
 if ( ! ( rx > rz ) ) {
 float t = rx;
 rx = rz;
 rz = t;

}

}
 return FormulaVec3 ( rx , ry , rz );

}
 FormulaVec3 foldingoctIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_FOLDINGOCTIFS > ( p , params , de );

}

}}
