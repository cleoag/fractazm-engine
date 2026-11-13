// Generated from 3Da/Sierpinski3alt.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10024 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SIERPINSKI3ALT = 10024;
 template < > FormulaVec3 execute_formula < FORMULA_SIERPINSKI3ALT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 const float dX = params [ 4 ];
 const float dY = params [ 5 ];
 const float dZ = params [ 6 ];
 float s0 = - p . x;
 float s1 = p . y;
 float s2 = p . z;
 if ( s0 >= s1 ) {
 float t = s0;
 s0 = s1;
 s1 = t;

}
 if ( s0 >= s2 ) {
 float t = s0;
 s0 = s2;
 s2 = t;

}
 s0 = - s0;
 {
 float t = s0;
 s0 = s1;
 s1 = t;

}
 s0 = - s0;
 if ( s0 >= s2 ) {
 float t = s0;
 s0 = s2;
 s2 = t;

}
 s0 = - s0;
 {
 float t = s0;
 s0 = s1;
 s1 = t;

}
 float fx = s0;
 float fy = s1;
 float fz = s2;
 float p0 = fx + dX;
 float p1 = fy + dY;
 float p2 = fz + dZ;
 RotMatrix3 R = rotMatrix3FromFloats ( params , 7 );
 FormulaVec3 rv = R * FormulaVec3 ( p0 , p1 , p2 );
 float r0 = rv . x;
 float r1 = rv . y;
 float r2 = rv . z;
 float v0 = r0 - dX;
 float v1 = r1 - dY;
 float v2 = r2 - dZ;
 de *= Scale;
 v0 *= Scale;
 v1 *= Scale;
 v2 *= Scale;
 float k = Scale - 1.0f;
 float out_x = v0 - k * CScaleX;
 float out_y = v1 - k * CScaleY;
 float out_z = v2 - k * CScaleZ;
 return FormulaVec3 ( out_x , out_y , out_z );

}
 FormulaVec3 Sierpinski3alt_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SIERPINSKI3ALT > ( p , params , de );

}

}}
