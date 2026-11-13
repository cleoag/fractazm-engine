// Generated from 4D/_Atan4D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6594 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ATAN4D = 6594;
 template < > FormulaVec3 execute_formula < FORMULA__ATAN4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Px = params [ 0 ];
 const float Py = params [ 1 ];
 const float Pz = params [ 2 ];
 const float Pw = params [ 3 ];
 const float Strength = params [ 4 ];
 const float Scale = 0.5f;
 const float thresh = 1e-5f;
 const float LogScale = 0.25f;
 float x = p . x + Px;
 float y = p . y + Py;
 float z = p . z + Pz;
 float w = de + Pw;
 float r = native_formula_length3 ( y , z , w );
 if ( r <= thresh ) {
 de = w;
 return FormulaVec3 ( atan ( x ) , y , z );

}
 if ( x == 0.0f && fabs ( r - 1.0f ) <= thresh ) {
 de = w;
 return FormulaVec3 ( atan ( x ) , y , z );

}
 float x_prime = ( atan2 ( x , 1.0f - r ) - atan2 ( - x , 1.0f + r ) ) * Scale;
 float one_minus_r = 1.0f - r;
 float one_plus_r = 1.0f + r;
 float numer = one_plus_r * one_plus_r + x * x;
 float denom = one_minus_r * one_minus_r + x * x;
 float ratio = numer / denom;
 float factor = log ( ratio ) * LogScale / r;
 float x_out = x_prime * Strength;
 float y_out = y * Strength * factor;
 float z_out = z * Strength * factor;
 de = w * Strength * factor;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
