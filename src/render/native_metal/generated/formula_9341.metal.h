// Generated from Ads/_asdamTrans.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9341 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ASDAMTRANS = 9341;
 template < > FormulaVec3 execute_formula < FORMULA__ASDAMTRANS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 RotMatrix3 R = rotMatrix3FromFloats ( params , 0 );
 const float m00 = R . col0 . x , m01 = R . col1 . x , m02 = R . col2 . x;
 const float m10 = R . col0 . y , m11 = R . col1 . y , m12 = R . col2 . y;
 const float m20 = R . col0 . z , m21 = R . col1 . z , m22 = R . col2 . z;
 const float Term_Scale = params [ 9 ];
 float x = p . x , y = p . y , z = p . z;
 float r = native_formula_length3 ( x , y , z );
 float s = Term_Scale / r;
 float sx = x * s;
 float sy = y * s;
 float sz = z * s;
 float row0 = sx * m00 + sy * m01 + sz * m02;
 float row1 = sx * m10 + sy * m11 + sz * m12;
 float row2 = sx * m20 + sy * m21 + sz * m22;
 return FormulaVec3 ( x + row0 , y + row1 , z + row2 );

}

}}
