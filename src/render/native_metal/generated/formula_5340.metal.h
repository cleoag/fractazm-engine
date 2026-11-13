// Generated from 4D/_Log4D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5340 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__LOG4D = 5340;
 template < > FormulaVec3 execute_formula < FORMULA__LOG4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Threshold = 1e-200f;
 const float Scale = 0.5f;
 const float DefaultVal = 460.0f;
 float x = p . x , y = p . y , z = p . z , w = de;
 float r = native_formula_length3 ( y , z , w );
 float x_out , y_out , z_out , w_out;
 if ( r != 0.0f ) {
 float theta = atan2 ( r , x );
 float factor = theta / r;
 float q2 = x * x + r * r;
 x_out = ( q2 > Threshold ) ? ( log ( q2 ) * Scale ) : DefaultVal;
 y_out = y * factor;
 z_out = z * factor;
 w_out = w * factor;

}
 else {
 x_out = ( x > Threshold ) ? log ( x ) : DefaultVal;
 y_out = y;
 z_out = z;
 w_out = w;

}
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
