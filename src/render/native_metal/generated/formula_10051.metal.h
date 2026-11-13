// Generated from 3Da/MengerSmt.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10051 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGERSMT = 10051;
 static inline float smooth_abs ( float v , float s , bool cubic ) {
 if ( cubic ) {
 float av = fabs ( v );
 float t = av * av;
 return ( av * t ) / ( t + s );

}
 else {
 return sqrt ( v * v + s );

}

}
 static inline void smooth_sort_step ( thread float & a , thread float & b , float s , float formula_half , bool cubic ) {
 float d = a - b;
 float smt;
 if ( cubic ) {
 float ad = fabs ( d );
 float t = ad * ad;
 smt = ( ad * t ) / ( t + s );

}
 else {
 smt = sqrt ( d * d + s );

}
 float diff = ( d - smt ) * formula_half;
 a -= diff;
 b += diff;

}
 template < > FormulaVec3 execute_formula < FORMULA_MENGERSMT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 const float LSmoothing = params [ 13 ];
 const float ESmoothing = params [ 14 ];
 const float half_val = 0.5f;
 const float s_l = fabs ( LSmoothing );
 const float s_e = fabs ( ESmoothing );
 const bool l_cubic = ( LSmoothing <= 0.0f );
 const bool e_cubic = ( ESmoothing <= 0.0f );
 float x = p . x;
 float y = p . y;
 float z = p . z;
 x = smooth_abs ( x , s_l , l_cubic );
 y = smooth_abs ( y , s_l , l_cubic );
 z = smooth_abs ( z , s_l , l_cubic );
 smooth_sort_step ( x , y , s_e , half_val , e_cubic );
 smooth_sort_step ( x , z , s_e , half_val , e_cubic );
 smooth_sort_step ( y , z , s_e , half_val , e_cubic );
 float r0 = x * params [ 10 ] + y * params [ 11 ] + z * params [ 12 ];
 float r1 = x * params [ 4 ] + y * params [ 5 ] + z * params [ 6 ];
 float r2 = x * params [ 7 ] + y * params [ 8 ] + z * params [ 9 ];
 de *= Scale;
 float k = Scale - 1.0f;
 float kz = ( k / Scale ) * CScaleZ;
 float x_out = r1 * Scale - k * CScaleX;
 float y_out = r2 * Scale - k * CScaleY;
 float z_val = r0 - kz;
 float sabs_z = smooth_abs ( z_val , s_l , l_cubic );
 float z_out = ( kz - sabs_z ) * Scale;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
