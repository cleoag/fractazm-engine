// Generated from 4Da/Sierpinski4.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10344 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SIERPINSKI4 = 10344;
 template < > FormulaVec3 execute_formula < FORMULA_SIERPINSKI4 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CX = params [ 1 ];
 const float CY = params [ 2 ];
 const float CZ = params [ 3 ];
 const float CW = params [ 4 ];
 RotMatrix4 RM = rotMatrix4FromFloats ( params , 5 );
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
 float r0 = x * RM . m [ 0 ] [ 0 ] + y * RM . m [ 0 ] [ 1 ] + z * RM . m [ 0 ] [ 2 ] + w * RM . m [ 0 ] [ 3 ];
 float r1 = x * RM . m [ 1 ] [ 0 ] + y * RM . m [ 1 ] [ 1 ] + z * RM . m [ 1 ] [ 2 ] + w * RM . m [ 1 ] [ 3 ];
 float r2 = x * RM . m [ 2 ] [ 0 ] + y * RM . m [ 2 ] [ 1 ] + z * RM . m [ 2 ] [ 2 ] + w * RM . m [ 2 ] [ 3 ];
 float r3 = x * RM . m [ 3 ] [ 0 ] + y * RM . m [ 3 ] [ 1 ] + z * RM . m [ 3 ] [ 2 ] + w * RM . m [ 3 ] [ 3 ];
 float k = Scale - 1.0f;
 float x_out = Scale * r0 - k * CX;
 float y_out = Scale * r1 - k * CY;
 float z_out = Scale * r2 - k * CZ;
 float w_out = Scale * r3 - k * CW;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
