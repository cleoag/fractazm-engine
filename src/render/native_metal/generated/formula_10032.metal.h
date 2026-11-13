// Generated from 3Da/koch_oct.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10032 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KOCH_OCT = 10032;
 template < > FormulaVec3 execute_formula < FORMULA_KOCH_OCT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 const float Fold = params [ 22 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 float ax = abs ( p . x );
 float ay = abs ( p . y );
 float az = abs ( p . z );
 FormulaVec3 r1v = R1 * FormulaVec3 ( ax , ay , az );
 float rx = r1v . x;
 float ry = r1v . y;
 float rz = r1v . z;
 float a = rx , b = ry , c = rz;
 if ( a >= b ) {
 float t = a;
 a = b;
 b = t;

}
 if ( a >= c ) {
 float t = a;
 a = c;
 c = t;

}
 {
 float t = a;
 a = b;
 b = t;

}
 if ( a >= c ) {
 float t = a;
 a = c;
 c = t;

}
 {
 float t = a;
 a = b;
 b = t;

}
 FormulaVec3 r2v = R2 * FormulaVec3 ( a , b , c );
 float r2x = r2v . x;
 float r2y = r2v . y;
 float r2z = r2v . z;
 de = de * Scale;
 float sx = r2x * Scale;
 float sy = r2y * Scale;
 float sz = r2z * Scale;
 float x_folded = abs ( Fold + sx ) - abs ( Fold - sx ) - sx;
 float k = Scale - 1.0f;
 float x_out = x_folded - k * CScaleX;
 float y_out = sy - k * CScaleY;
 float z_out = sz - k * CScaleZ;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
