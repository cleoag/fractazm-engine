// Generated from Ads/_FoldingIcosa.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4734 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDINGICOSA = 4734;
 template < > FormulaVec3 execute_formula < FORMULA__FOLDINGICOSA > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 RotMatrix3 R = rotMatrix3FromFloats ( params , 0 );
 const float m00 = R . col0 . x , m01 = R . col1 . x , m02 = R . col2 . x;
 const float m10 = R . col0 . y , m11 = R . col1 . y , m12 = R . col2 . y;
 const float m20 = R . col0 . z , m21 = R . col1 . z , m22 = R . col2 . z;
 const float n0 = 0.309016994374947f;
 const float n1 = - 0.5f;
 const float n2 = - 0.809016994374947f;
 float x = p . x , y = p . y , z = p . z;
 float A = fabs ( x * m00 + y * m01 + z * m02 );
 float B = fabs ( x * m10 + y * m11 + z * m12 );
 float C = fabs ( x * m20 + y * m21 + z * m22 );
 float val0 = B;
 float val1 = C;
 float val2 = A;
 for ( int i = 0;
 i < 3;
 i ++ ) {
 float dot = val0 * n0 + val1 * n1 + val2 * n2;
 if ( dot < 0.0f ) {
 float t = 2.0f * dot;
 val0 -= t * n0;
 val1 -= t * n1;
 val2 -= t * n2;

}
 val1 = fabs ( val1 );
 val2 = fabs ( val2 );

}
 return FormulaVec3 ( val0 , val1 , val2 );

}

}}
