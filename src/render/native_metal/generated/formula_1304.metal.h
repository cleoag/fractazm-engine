// Generated from dIFS/sliceExIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1304 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SLICEEXIFS = 1304;
 template < > FormulaVec3 execute_formula < FORMULA_SLICEEXIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Angle = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Sph_Radius = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float RepeatAngle = params [ 7 ];
 const float Thickness = params [ 8 ];
 const float z_Swirl = params [ 9 ];
 const float z_Angle = params [ 10 ];
 const float z_RepeatAngle = params [ 11 ];
 const int Or_And_Nor = int ( params [ 12 ] );
 const float K_360_OVER_PI = params [ 13 ];
 const float K_PI_OVER_360 = params [ 14 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const auto swirl = numeric :: extendedSinCos ( E ( z ) * E ( z_Swirl ) );
 const E s ( static_cast < float > ( swirl . sin ) ) , c ( static_cast < float > ( swirl . cos ) );
 const E ex ( x ) , ey ( y ) , ez ( z );
 struct NativeClosure0 {
 thread decltype ( K_360_OVER_PI ) & K_360_OVER_PI;
 thread decltype ( K_PI_OVER_360 ) & K_PI_OVER_360;
 thread decltype ( Or_And_Nor ) & Or_And_Nor;
 auto operator ( ) ( E angle , float repeat , float width ) const {
 const E sector = angle * E ( K_360_OVER_PI ) / E ( repeat );
 E value = ( abs ( ( sector - round ( sector ) ) * E ( repeat ) ) / E ( width ) - E ( 1 ) ) * E ( width ) * E ( K_PI_OVER_360 );
 return Or_And_Nor == 2 ? value : - value;

}

}
;
 auto tooth = NativeClosure0 {
 K_360_OVER_PI , K_PI_OVER_360 , Or_And_Nor
}
;
 const E xy = tooth ( numeric :: extendedAtan2 ( ex * s - ey * c , ex * c + ey * s ) , RepeatAngle , Angle );
 const E zz = tooth ( numeric :: extendedAtan2 ( sqrt ( ex * ex + ey * ey ) , ez ) , z_RepeatAngle , z_Angle );
 const E combined = ( Or_And_Nor != 1 && Or_And_Nor != 2 ) ? ( zz >= xy ? zz : xy ) : ( zz >= xy ? xy : zz );
 const E shell = abs ( native_formula_length3 ( ex , ey , ez ) - E ( Sph_Radius ) + E ( Thickness ) ) - E ( Thickness );
 E result = shell >= combined ? shell : combined;
 if ( ! Apply_scale_add ) result /= E ( Scale );
 de = static_cast < float > ( result );
 return Apply_scale_add ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 sliceExIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SLICEEXIFS > ( p , params , de );

}

}}
