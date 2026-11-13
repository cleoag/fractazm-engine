// Generated from 4D/Makin4D_p4a.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10326 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN4D_P4A = 10326;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN4D_P4A > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float w_add = params [ 0 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 for ( int iter = 0;
 iter < 2;
 iter ++ ) {
 float tw_re = 2.0f * ( x * z - y * w );
 float tw_im = 2.0f * ( x * w + y * z );
 float s = x * x + y * y;
 float t = z * z + w * w;
 float tz_re , tz_im;
 if ( t < s ) {
 float ratio = 1.0f - t / s;
 tz_re = ( x * x - y * y ) * ratio;
 tz_im = 2.0f * x * y * ratio;

}
 else {
 float ratio = s / t - 1.0f;
 tz_re = ( z * z - w * w ) * ratio;
 tz_im = 2.0f * z * w * ratio;

}
 x = tz_re;
 y = tz_im;
 z = tw_re;
 w = tw_im;

}
 w += w_add;
 de = w;
 return FormulaVec3 ( x , y , z );

}

}}
