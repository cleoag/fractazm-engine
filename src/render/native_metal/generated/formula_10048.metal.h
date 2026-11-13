// Generated from 3Da/DodecahedronIFSex.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10048 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_DODECAHEDRONIFS_EX = 10048;
 static inline void reflect_fold ( thread float & x , thread float & y , thread float & z , float nx , float ny , float nz ) {
 float d = x * nx + y * ny + z * nz;
 if ( d < 0.0f ) {
 float t = - 2.0f * d;
 x += t * nx;
 y += t * ny;
 z += t * nz;

}

}
 template < > FormulaVec3 execute_formula < FORMULA_DODECAHEDRONIFS_EX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float CScaleX = params [ 1 ];
 const float CScaleY = params [ 2 ];
 const float CScaleZ = params [ 3 ];
 RotMatrix3 R1 = rotMatrix3FromFloats ( params , 4 );
 RotMatrix3 R2 = rotMatrix3FromFloats ( params , 13 );
 const float Phi = params [ 22 ];
 const int Rot2Style = ( int ) params [ 23 ];
 float x = p . x;
 float y = p . y;
 float z = p . z;
 {
 FormulaVec3 r1v = R1 * FormulaVec3 ( x , y , z );
 x = r1v . x;
 y = r1v . y;
 z = r1v . z;

}
 float n1x = - ( 1.0f - Phi );
 float n1y = 1.0f / n1x;
 float n1z = - 1.0f;
 float nlen = native_formula_length3 ( n1x , n1y , n1z );
 float inv_nlen = 1.0f / nlen;
 n1x *= inv_nlen;
 n1y *= inv_nlen;
 n1z *= inv_nlen;
 reflect_fold ( x , y , z , n1x , n1y , n1z );
 if ( Rot2Style == 0 ) {
 FormulaVec3 t = R2 * FormulaVec3 ( y , x , z );
 x = t . y;
 y = t . x;
 z = t . z;

}
 for ( int i = 0;
 i < 6;
 i ++ ) {
 float tmp = n1x;
 n1x = n1z;
 n1z = n1y;
 n1y = tmp;
 reflect_fold ( x , y , z , n1x , n1y , n1z );
 if ( Rot2Style == i + 1 ) {
 FormulaVec3 t = R2 * FormulaVec3 ( y , x , z );
 x = t . y;
 y = t . x;
 z = t . z;

}

}
 float k = Scale - 1.0f;
 de *= Scale;
 float new_x = x * Scale - k * CScaleY;
 float new_y = y * Scale - k * CScaleX;
 float new_z = z * Scale - k * CScaleZ;
 x = new_x;
 y = new_y;
 z = new_z;
 if ( Rot2Style == 7 ) {
 FormulaVec3 t = R2 * FormulaVec3 ( y , x , z );
 x = t . y;
 y = t . x;
 z = t . z;

}
 return FormulaVec3 ( x , y , z );

}

}}
