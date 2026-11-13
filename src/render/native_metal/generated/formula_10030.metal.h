// Generated from 3Da/OctahedronIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10030 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_OCTAHEDRONIFS = 10030;
 template < > FormulaVec3 execute_formula < FORMULA_OCTAHEDRONIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 FormulaVec3 rv = R1 * FormulaVec3 ( p . x , p . y , p . z );
 float st0 = rv . x;
 float st1 = rv . y;
 float st2 = rv . z;
 if ( st0 < st1 ) {
 float t = st0;
 st0 = st1;
 st1 = t;

}
 st0 = - st0;
 if ( ! ( st0 < st1 ) ) {
 float t = st0;
 st0 = st1;
 st1 = t;

}
 st0 = - st0;
 if ( st0 < st2 ) {
 float t = st0;
 st0 = st2;
 st2 = t;

}
 st0 = - st0;
 if ( ! ( st0 < st2 ) ) {
 float t = st0;
 st0 = st2;
 st2 = t;

}
 st0 = - st0;
 {
 float t = st0;
 st0 = st1;
 st1 = t;

}
 const float m50 = params [ 13 ] , m54 = params [ 14 ] , m58 = params [ 15 ];
 const float m5c = params [ 16 ] , m60 = params [ 17 ] , m64 = params [ 18 ];
 const float m68 = params [ 19 ] , m6c = params [ 20 ] , m70 = params [ 21 ];
 float r2y = st1 * m5c + st0 * m60 + st2 * m64;
 float r2x = st1 * m50 + st0 * m54 + st2 * m58;
 float r2z = st1 * m68 + st0 * m6c + st2 * m70;
 de = de * Scale;
 float k = Scale - 1.0f;
 return FormulaVec3 ( r2x * Scale - k * CScaleX , r2y * Scale - k * CScaleY , r2z * Scale - k * CScaleZ );

}

}}
