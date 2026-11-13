// Generated from 3Da/OctaFullIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10172 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_OCTAFULLIFS = 10172;
 template < > FormulaVec3 execute_formula < FORMULA_OCTAFULLIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 float x = abs ( p . x );
 float y = abs ( p . y );
 float z = abs ( p . z );
 FormulaVec3 a_vec = R1 * FormulaVec3 ( x , y , z );
 float a0 = a_vec . x;
 float a1 = a_vec . y;
 float a2 = a_vec . z;
 if ( a0 > a1 ) {
 float t = a0;
 a0 = a1;
 a1 = t;

}
 if ( a0 > a2 ) {
 float t = a0;
 a0 = a2;
 a2 = t;

}
 if ( a1 > a2 ) {
 float t = a1;
 a1 = a2;
 a2 = t;

}
 const float st0 = a0;
 const float st1 = a1;
 const float st2 = a2;
 const float m50 = params [ 13 ] , m54 = params [ 14 ] , m58 = params [ 15 ];
 const float m5c = params [ 16 ] , m60 = params [ 17 ] , m64 = params [ 18 ];
 const float m68 = params [ 19 ] , m6c = params [ 20 ] , m70 = params [ 21 ];
 const float r2y = st1 * m5c + st0 * m60 + st2 * m64;
 const float r2x = st1 * m50 + st0 * m54 + st2 * m58;
 const float r2z = st1 * m68 + st0 * m6c + st2 * m70;
 de = de * Scale;
 float k = Scale - 1.0f;
 float out_x = r2x * Scale - k * CScaleX;
 float out_y = r2y * Scale - k * CScaleY;
 float out_z = r2z * Scale - k * CScaleZ;
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
