// Generated from 3Da/_AmazingBoxSSE2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10351 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__AMAZINGBOXSSE2 = 10351;
 template < > FormulaVec3 execute_formula < FORMULA__AMAZINGBOXSSE2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float ScaleOverMinR2 = params [ 0 ];
 const float minR2 = params [ 1 ];
 const float Scale = params [ 3 ];
 const float Fold = params [ 2 ];
 const float FixedR2 = 1.0f;
 float bx = p . x > - Fold ? p . x : - Fold;
 bx = 2.0f * ( bx < Fold ? bx : Fold ) - p . x;
 float by = p . y > - Fold ? p . y : - Fold;
 by = 2.0f * ( by < Fold ? by : Fold ) - p . y;
 float bz = p . z > - Fold ? p . z : - Fold;
 bz = 2.0f * ( bz < Fold ? bz : Fold ) - p . z;
 float rr = ( bx * bx + bz * bz ) + by * by;
 float scale;
 if ( ! ( rr >= minR2 ) ) {
 scale = ScaleOverMinR2;

}
 else if ( ! ( rr >= FixedR2 ) ) {
 scale = Scale / rr;

}
 else {
 scale = Scale;

}
 de *= scale;
 return FormulaVec3 ( bx * scale , by * scale , bz * scale );

}

}}
