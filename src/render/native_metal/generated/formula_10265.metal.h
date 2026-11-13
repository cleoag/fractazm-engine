// Generated from 3D/BT1Pinegon.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10265 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BT1PINEGON = 10265;
 template < > FormulaVec3 execute_formula < FORMULA_BT1PINEGON > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float xOff = params [ 1 ];
 const float pixel_x = params [ 2 ];
 const float pixel_y = params [ 3 ];
 const float pixel_z = params [ 4 ];
 const float yz_angle = params [ 5 ];
 const int AngleIter = ( int ) params [ 6 ];
 const int A_AfterIter = ( int ) params [ 7 ];
 const unsigned countByte = static_cast < unsigned > ( static_cast < int > ( params [ 8 ] ) ) & 255u;
 const unsigned T1_Count = countByte ? countByte : 256u;
 const int PolyIter = ( int ) params [ 9 ];
 const int PolyIterAfter = ( int ) params [ 10 ];
 const float Sides = params [ 11 ];
 const float s23 = params [ 12 ];
 const float s13 = params [ 13 ];
 const float s12 = params [ 14 ];
 const float pi_const = params [ 15 ];
 const float Cx = params [ 16 ];
 const float Cy = params [ 17 ];
 const float Cz = params [ 18 ];
 const unsigned iteration = static_cast < unsigned > ( static_cast < int > ( params [ ITER_COUNT_SLOT ] ) ) & 255u;
 struct NativeClosure0 {
 auto operator ( ) ( int value ) const {
 return static_cast < unsigned > ( value ) & 255u;

}

}
;
 auto lowByte = NativeClosure0 {

}
;
 struct NativeClosure1 {
 auto operator ( ) ( unsigned value ) const {
 return value < 128u ? int ( value ) : int ( value ) - 256;

}

}
;
 auto signedByte = NativeClosure1 {

}
;
 const bool rotate = iteration == lowByte ( AngleIter ) || signedByte ( iteration ) >= signedByte ( lowByte ( A_AfterIter ) );
 const bool polygon = iteration == lowByte ( PolyIter ) || signedByte ( iteration ) >= signedByte ( lowByte ( PolyIterAfter ) );
 float x = p . x , y = p . y , z = p . z;
 for ( unsigned i = 0;
 i < T1_Count;
 i ++ ) {
 external_pine_t1 ( x , y , z , s23 , s13 , s12 , Scale , xOff );

}
 if ( rotate ) {
 float cos_a = cos ( yz_angle );
 float sin_a = sin ( yz_angle );
 float nz = z * sin_a + y * cos_a;
 float ny = y * sin_a - z * cos_a;
 y = ny;
 z = nz;

}
 if ( polygon ) {
 float half_angle = pi_const / Sides;
 float double_half = 2.0f * half_angle;
 float theta = abs ( atan2 ( z , y ) );
 while ( theta >= half_angle ) {
 theta -= double_half;

}
 float factor = cos ( theta );
 y *= factor;
 z *= factor;

}
 float xt = x * x;
 float yt = y * y;
 float zt = z * z;
 float t = 2.0f * x / sqrt ( yt + zt );
 float out_x = xt - yt - zt + Cx * pixel_x;
 float out_y = 2.0f * t * y * z + Cy * pixel_z;
 float out_z = t * ( yt - zt ) + Cz * pixel_y;
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
