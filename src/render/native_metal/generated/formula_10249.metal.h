// Generated from dIFSa/cayley2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10249 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_CAYLEY2IFS = 10249;
 template < > FormulaVec3 execute_formula < FORMULA_CAYLEY2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const float Mul_Z = params [ 12 ];
 float z_val = p . z + Add_Z;
 float y_val = p . y + Add_Y;
 float x_val = p . x + Add_X;
 FormulaVec3 rv = R * FormulaVec3 ( x_val , y_val , z_val );
 float rx = rv . x;
 float ry = rv . y;
 float rz = rv . z;
 rz *= Mul_Z;
 de *= abs ( Mul_Z );
 float x2 = rx * rx;
 float y2 = ry * ry;
 float m = ( x2 + y2 ) * ( x2 + y2 );
 float denom = m + 4.0f * rx * ry + 1.0f;
 float new_x = ( m - 1.0f ) / denom;
 float new_y = 2.0f * ( y2 - x2 ) / denom;
 return FormulaVec3 ( new_x , new_y , rz );

}
 FormulaVec3 cayley2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_CAYLEY2IFS > ( p , params , de );

}

}}
