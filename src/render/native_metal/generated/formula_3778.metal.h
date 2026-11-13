// Generated from 3Da/MengerIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3778 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGERIFS = 3778;
 template < > FormulaVec3 execute_formula < FORMULA_MENGERIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 float ax = abs ( p . x );
 float ay = abs ( p . y );
 float az = abs ( p . z );
 float max_val = ax;
 float mid_val = ay;
 float min_val = az;
 if ( max_val < mid_val ) {
 float t = max_val;
 max_val = mid_val;
 mid_val = t;

}
 if ( max_val < min_val ) {
 float t = max_val;
 max_val = min_val;
 min_val = t;

}
 if ( mid_val < min_val ) {
 float t = mid_val;
 mid_val = min_val;
 min_val = t;

}
 float r0 = max_val * params [ 10 ] + mid_val * params [ 11 ] + min_val * params [ 12 ];
 float r1 = max_val * params [ 4 ] + mid_val * params [ 5 ] + min_val * params [ 6 ];
 float r2 = max_val * params [ 7 ] + mid_val * params [ 8 ] + min_val * params [ 9 ];
 de = de * Scale;
 float k = Scale - 1.0f;
 float x_out = r1 * Scale - k * CScaleX;
 float y_out = r2 * Scale - k * CScaleY;
 float kz = ( k / Scale ) * CScaleZ;
 float z_out = ( kz - abs ( r0 - kz ) ) * Scale;
 FormulaVec3 result;
 result . x = x_out;
 result . y = y_out;
 result . z = z_out;
 return result;

}
 FormulaVec3 MengerIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_MENGERIFS > ( p , params , de );

}

}}
