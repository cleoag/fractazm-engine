// Generated from 3Da/DodecahedronIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10041 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_DODECAHEDRONIFS = 10041;
 template < > FormulaVec3 execute_formula < FORMULA_DODECAHEDRONIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 const float C0 = 0.309016994374947f;
 const float C1 = - 0.5f;
 const float C2 = - 0.809016994374947f;
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 FormulaVec3 r1v = R1 * p;
 float row0 = abs ( r1v . x );
 float row1 = abs ( r1v . y );
 float row2 = abs ( r1v . z );
 float x = row0;
 float y = row1;
 float z = row2;
 for ( int i = 0;
 i < 3;
 i ++ ) {
 float dot = x * C0 + y * C1 + z * C2;
 if ( dot < 0.0f ) {
 float d2 = 2.0f * dot;
 x -= d2 * C0;
 y -= d2 * C1;
 z -= d2 * C2;

}
 y = abs ( y );
 z = abs ( z );

}
 FormulaVec3 r2v = R2 * FormulaVec3 ( x , y , z );
 float rot0 = r2v . x;
 float rot1 = r2v . y;
 float rot2 = r2v . z;
 de *= Scale;
 rot0 *= Scale;
 rot1 *= Scale;
 rot2 *= Scale;
 float k = Scale - 1.0f;
 rot0 -= k * CScaleX;
 rot1 -= k * CScaleY;
 rot2 -= k * CScaleZ;
 return FormulaVec3 ( rot0 , rot1 , rot2 );

}

}}
