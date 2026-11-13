// Generated from 3Da/Sierpinski3.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10170 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SIERPINSKI3 = 10170;
 template < > FormulaVec3 execute_formula < FORMULA_SIERPINSKI3 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 float x = p . x;
 float y = p . y;
 float z = p . z;
 FormulaVec3 a = R1 * FormulaVec3 ( x , y , z );
 float A = a . x;
 float B = a . y;
 float C = a . z;
 float s0 = - A;
 if ( s0 >= B ) {
 float t = s0;
 s0 = B;
 B = t;

}
 if ( s0 >= C ) {
 float t = s0;
 s0 = C;
 C = t;

}
 s0 = - s0;
 float s1 = - B;
 if ( s1 >= C ) {
 float t = s1;
 s1 = C;
 C = t;

}
 s1 = - s1;
 float f0 = s0;
 float f1 = s1;
 float f2 = C;
 FormulaVec3 b = R2 * FormulaVec3 ( f0 , f1 , f2 );
 de = de * Scale;
 float k = Scale - 1.0f;
 float out_x = b . x * Scale - k * CScaleX;
 float out_y = b . y * Scale - k * CScaleY;
 float out_z = b . z * Scale - k * CScaleZ;
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
