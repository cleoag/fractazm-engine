// Generated from 3Da/IcosahedronIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10176 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ICOSAHEDRONIFS = 10176;
 template < > FormulaVec3 execute_formula < FORMULA_ICOSAHEDRONIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 const float N0 = 0.309016994374947f;
 const float N1 = - 0.5f;
 const float N2 = - 0.809016994374947f;
 float x = p . x;
 float y = p . y;
 float z = p . z;
 FormulaVec3 r1v = R1 * FormulaVec3 ( x , y , z );
 float a0 = abs ( r1v . x );
 float a1 = abs ( r1v . y );
 float a2 = abs ( r1v . z );
 x = a0;
 y = a1;
 z = a2;
 for ( int i = 0;
 i < 3;
 i ++ ) {
 float dot = x * N0 + y * N1 + z * N2;
 if ( dot < 0.0f ) {
 float d2 = 2.0f * dot;
 x -= d2 * N0;
 y -= d2 * N1;
 z -= d2 * N2;

}
 y = abs ( y );
 z = abs ( z );

}
 FormulaVec3 b_vec = R2 * FormulaVec3 ( x , y , z );
 float b0 = b_vec . x;
 float b1 = b_vec . y;
 float b2 = b_vec . z;
 de = de * Scale;
 b0 *= Scale;
 b1 *= Scale;
 b2 *= Scale;
 float k = Scale - 1.0f;
 b0 -= k * CScaleX;
 b1 -= k * CScaleY;
 b2 -= k * CScaleZ;
 return FormulaVec3 ( b0 , b1 , b2 );

}

}}
