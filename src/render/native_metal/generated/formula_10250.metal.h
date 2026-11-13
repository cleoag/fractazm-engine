// Generated from dIFSa/trans-qIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10250 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TRANS_QIFS = 10250;
 template < > FormulaVec3 execute_formula < FORMULA_TRANS_QIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const float Quadratic_X = params [ 12 ];
 const float Quadratic_Y = params [ 13 ];
 float z_val = p . z + Add_Z;
 float y_val = p . y + Add_Y;
 float x_val = p . x + Add_X;
 FormulaVec3 rv = R * FormulaVec3 ( x_val , y_val , z_val );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 float x = 1.0f - abs ( rx );
 float y = 1.0f - abs ( ry );
 if ( x > 0.0f ) {
 float a1 = Quadratic_X - 1.0f;
 float b1 = 1.0f - a1;
 x = a1 * x * x + b1 * x;

}
 if ( y > 0.0f ) {
 float a2 = Quadratic_Y - 1.0f;
 float b2 = 1.0f - a2;
 y = a2 * y * y + b2 * y;

}
 return FormulaVec3 ( x , y , rz );

}
 FormulaVec3 trans_qIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TRANS_QIFS > ( p , params , de );

}

}}
