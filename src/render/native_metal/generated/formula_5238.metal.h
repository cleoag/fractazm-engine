// Generated from Ads/_RotatedAbs.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5238 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ROTATEDABS = 5238;
 template < > FormulaVec3 execute_formula < FORMULA__ROTATEDABS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 RotMatrix3 R = rotMatrix3FromFloats ( params , 0 );
 const float m00 = R . col0 . x , m01 = R . col1 . x , m02 = R . col2 . x;
 const float m10 = R . col0 . y , m11 = R . col1 . y , m12 = R . col2 . y;
 const float m20 = R . col0 . z , m21 = R . col1 . z , m22 = R . col2 . z;
 const float AddX = params [ 9 ];
 const float SubX = params [ 10 ];
 float x = p . x , y = p . y , z = p . z;
 float r0 = x * m00 + y * m01 + z * m02;
 float r1 = x * m10 + y * m11 + z * m12;
 float r2 = x * m20 + y * m21 + z * m22;
 r0 = fabs ( r0 + AddX ) - SubX;
 float new_x = r0 * m00 + r1 * m10 + r2 * m20;
 float new_y = r0 * m01 + r1 * m11 + r2 * m21;
 float new_z = r0 * m02 + r1 * m12 + r2 * m22;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
