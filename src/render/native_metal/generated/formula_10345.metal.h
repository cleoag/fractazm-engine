// Generated from 4Da/HalfOct4a.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10345 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HALFOCT4A = 10345;
 template < > FormulaVec3 execute_formula < FORMULA_HALFOCT4A > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CX = params [ 1 ];
 const float CY = params [ 2 ];
 const float CZ = params [ 3 ];
 const float CW = params [ 4 ];
 float a = p . x , b = p . y , c = p . z , d = de;
 a = - a;
 if ( a >= b ) {
 float t = a;
 a = b;
 b = t;

}
 a = - a;
 if ( a < b ) {
 float t = a;
 a = b;
 b = t;

}
 a = - a;
 if ( a >= c ) {
 float t = a;
 a = c;
 c = t;

}
 a = - a;
 if ( a >= c ) {
 float t = a;
 a = c;
 c = t;

}
 if ( a >= d ) {
 float t = a;
 a = d;
 d = t;

}
 {
 float t = a;
 a = b;
 b = t;

}
 if ( a >= d ) {
 float t = a;
 a = d;
 d = t;

}
 {
 float t = a;
 a = b;
 b = t;

}
 {
 float t = a;
 a = c;
 c = t;

}
 if ( a < d ) {
 float t = a;
 a = d;
 d = t;

}
 {
 float t = a;
 a = c;
 c = t;

}
 float r0 = a * params [ 5 ] + b * params [ 6 ] + c * params [ 7 ] + d * params [ 8 ];
 float r1 = a * params [ 9 ] + b * params [ 10 ] + c * params [ 11 ] + d * params [ 12 ];
 float r2 = a * params [ 13 ] + b * params [ 14 ] + c * params [ 15 ] + d * params [ 16 ];
 float r3 = a * params [ 17 ] + b * params [ 18 ] + c * params [ 19 ] + d * params [ 20 ];
 float k = Scale - 1.0f;
 float x_out = Scale * r0 - k * CX;
 float y_out = Scale * r1 - k * CY;
 float z_out = Scale * r2 - k * CZ;
 float w_out = Scale * r3 - k * CW;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
