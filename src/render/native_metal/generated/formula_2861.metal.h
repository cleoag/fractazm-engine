// Generated from 3D/Quintic.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_2861 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_QUINTIC = 2861;
 template < > FormulaVec3 execute_formula < FORMULA_QUINTIC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float A = params [ 0 ];
 const float B = params [ 1 ];
 const float C = params [ 2 ];
 const float D = params [ 3 ];
 const int Negative = int ( params [ 4 ] );
 const float A0 = - 10.0f;
 const float A1 = 5.0f;
 const float Cx = params [ 7 ];
 const float Cy = params [ 8 ];
 const float Cz = params [ 9 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float x3 = x2 * x , y3 = y2 * y , z3 = z2 * z;
 float xy = x * y , xz = x * z , yz = y * z;
 float xyz = xy * z;
 float sign = ( Negative != 0 ) ? - 1.0f : 1.0f;
 float xT1 = x3 * x2;
 float xT2 = ( y2 + z2 + yz * A ) * x3 * A0;
 float xT3 = ( y2 * y2 + z2 * z2 + y3 * z * B + yz * yz * C + yz * z2 * B ) * x * A1;
 float xT4 = x2 * yz * ( y + z ) * D;
 float x_out = sign * ( xT1 + xT2 + xT3 + xT4 ) + Cx;
 float yT1 = y3 * y2;
 float yT2 = ( z2 + x2 + xz * A ) * y3 * A0;
 float yT3 = ( z2 * z2 + x2 * x2 + xz * z2 * B + xz * xz * C + x2 * xz * B ) * y * A1;
 float yT4 = y2 * xz * ( z + x ) * D;
 float y_out = sign * ( yT1 + yT2 + yT3 + yT4 ) + Cy;
 float zT1 = z3 * z2;
 float zT2 = ( x2 + y2 + xy * A ) * z3 * A0;
 float zT3 = ( x2 * x2 + y2 * y2 + x3 * y * B + xy * xy * C + xy * y2 * B ) * z * A1;
 float zT4 = z2 * xy * ( x + y ) * D;
 float z_out = sign * ( zT1 + zT2 + zT3 + zT4 ) + Cz;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
