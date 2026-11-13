// Generated from 4D/Makin4D_p8b.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10333 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN4D_P8B = 10333;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN4D_P8B > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float w_add = params [ 0 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 for ( int iter = 0;
 iter < 3;
 iter ++ ) {
 float s = x * x + y * y;
 float t = z * z + w * w;
 float m = s + t;
 float tw_re = 2.0f * ( x * z - y * w );
 float tw_im = 2.0f * ( x * w + y * z );
 float tz_re = 2.0f * ( x * x - y * y );
 float tz_im = 4.0f * x * y;
 float scale = ( s - t ) / ( 2.0f * s );
 x = tz_re * scale;
 y = tz_im * scale;
 z = tw_re;
 w = tw_im;

}
 w += w_add;
 de = w;
 return FormulaVec3 ( x , y , z );

}

}}
