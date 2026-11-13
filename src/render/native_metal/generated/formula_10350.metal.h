// Generated from 4Da/Sierpinski4ex.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10350 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SIERPINSKI4EX = 10350;
 template < > FormulaVec3 execute_formula < FORMULA_SIERPINSKI4EX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CX = params [ 1 ];
 const float CY = params [ 2 ];
 const float CZ = params [ 3 ];
 const float CW = params [ 4 ];
 const float Off_A = params [ 21 ];
 const float Off_B = params [ 22 ];
 const float Off_C = params [ 23 ];
 const float Off_D = params [ 24 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 if ( x + y < 0.0f ) {
 float t = x;
 x = - y;
 y = - t;

}
 if ( x + z < 0.0f ) {
 float t = x;
 x = - z;
 z = - t;

}
 if ( y + z < 0.0f ) {
 float t = y;
 y = - z;
 z = - t;

}
 if ( x + w < 0.0f ) {
 float t = x;
 x = - w;
 w = - t;

}
 if ( y + w < 0.0f ) {
 float t = y;
 y = - w;
 w = - t;

}
 if ( z + w < 0.0f ) {
 float t = z;
 z = - w;
 w = - t;

}
 x += Off_B;
 y += Off_A;
 z += Off_C;
 w += Off_D;
 float r0 = x * params [ 5 ] + y * params [ 6 ] + z * params [ 7 ] + w * params [ 8 ];
 float r1 = x * params [ 9 ] + y * params [ 10 ] + z * params [ 11 ] + w * params [ 12 ];
 float r2 = x * params [ 13 ] + y * params [ 14 ] + z * params [ 15 ] + w * params [ 16 ];
 float r3 = x * params [ 17 ] + y * params [ 18 ] + z * params [ 19 ] + w * params [ 20 ];
 r0 -= Off_B;
 r1 -= Off_A;
 r2 -= Off_C;
 r3 -= Off_D;
 float k = Scale - 1.0f;
 float x_out = Scale * r0 - k * CX;
 float y_out = Scale * r1 - k * CY;
 float z_out = Scale * r2 - k * CZ;
 float w_out = Scale * r3 - k * CW;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
