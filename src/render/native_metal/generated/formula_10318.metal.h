// Generated from 3D/General.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10318 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GENERAL = 10318;
 inline native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > general_extended_pair ( int args , numeric :: Extended80 x , numeric :: Extended80 y , numeric :: Extended80 z ) {
 switch ( args ) {
 case 0 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( x , y );
 case 1 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( x , z );
 case 2 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( y , x );
 case 3 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( y , z );
 case 4 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( z , x );
 case 5 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( z , y );
 case 6 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( x , sqrt ( z * z + y * y ) );
 case 7 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( y , sqrt ( x * x + z * z ) );
 case 8 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( z , sqrt ( x * x + y * y ) );
 case 9 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( sqrt ( z * z + y * y ) , x );
 case 10 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( sqrt ( x * x + z * z ) , y );
 case 11 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( sqrt ( x * x + y * y ) , z );
 case 12 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( x , abs ( y ) > abs ( z ) ? abs ( z ) : abs ( y ) );
 case 13 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( y , abs ( x ) > abs ( z ) ? abs ( z ) : abs ( x ) );
 case 14 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( z , abs ( y ) > abs ( x ) ? abs ( x ) : abs ( y ) );
 case 15 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( abs ( y ) > abs ( z ) ? abs ( z ) : abs ( y ) , x );
 case 16 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( abs ( x ) > abs ( z ) ? abs ( z ) : abs ( x ) , y );
 case 17 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( abs ( y ) > abs ( x ) ? abs ( x ) : abs ( y ) , z );
 case 18 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( x , abs ( y ) );
 case 19 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( y , abs ( x ) );
 case 20 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( z , abs ( y ) );
 case 21 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( abs ( y ) , x );
 case 22 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( abs ( x ) , y );
 case 23 : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( abs ( y ) , z );
 default : return native_std :: pair < numeric :: Extended80 , numeric :: Extended80 > ( NAN , NAN );

}

}
 inline float2 general_angle_pair ( int args , float x , float y , float z ) {
 switch ( args ) {
 case 0 : return float2 ( x , y );
 case 1 : return float2 ( x , z );
 case 2 : return float2 ( y , x );
 case 3 : return float2 ( y , z );
 case 4 : return float2 ( z , x );
 case 5 : return float2 ( z , y );
 case 6 : return float2 ( x , sqrt ( z * z + y * y ) );
 case 7 : return float2 ( y , sqrt ( x * x + z * z ) );
 case 8 : return float2 ( z , sqrt ( x * x + y * y ) );
 case 9 : return float2 ( sqrt ( z * z + y * y ) , x );
 case 10 : return float2 ( sqrt ( x * x + z * z ) , y );
 case 11 : return float2 ( sqrt ( x * x + y * y ) , z );
 case 12 : return float2 ( x , fabs ( y ) > fabs ( z ) ? fabs ( z ) : fabs ( y ) );
 case 13 : return float2 ( y , fabs ( x ) > fabs ( z ) ? fabs ( z ) : fabs ( x ) );
 case 14 : return float2 ( z , fabs ( y ) > fabs ( x ) ? fabs ( x ) : fabs ( y ) );
 case 15 : return float2 ( fabs ( y ) > fabs ( z ) ? fabs ( z ) : fabs ( y ) , x );
 case 16 : return float2 ( fabs ( x ) > fabs ( z ) ? fabs ( z ) : fabs ( x ) , y );
 case 17 : return float2 ( fabs ( y ) > fabs ( x ) ? fabs ( x ) : fabs ( y ) , z );
 case 18 : return float2 ( x , fabs ( y ) );
 case 19 : return float2 ( y , fabs ( x ) );
 case 20 : return float2 ( z , fabs ( y ) );
 case 21 : return float2 ( fabs ( y ) , x );
 case 22 : return float2 ( fabs ( x ) , y );
 case 23 : return float2 ( fabs ( y ) , z );
 default : return float2 ( NAN , NAN );

}

}
 inline float general_angle ( float2 pair , int tweak , float power , float halfpi ) {
 if ( tweak == 1 || tweak == 3 ) {
 power = - power;
 pair . y = fabs ( pair . y );

}
 float angle = atan2 ( pair . x , pair . y );
 if ( tweak == 2 || tweak == 3 ) angle -= fabs ( halfpi + angle ) - fabs ( halfpi - angle );
 return angle * power;

}
 template < > FormulaVec3 execute_formula < FORMULA_GENERAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using namespace numeric;
 ExtendedScope scope;
 if ( int ( params [ 7 ] ) ) p . x = metal :: fabs ( p . x );
 if ( int ( params [ 8 ] ) ) p . y = metal :: fabs ( p . y );
 if ( int ( params [ 9 ] ) ) p . z = metal :: fabs ( p . z );
 struct NativeClosure0 {
 thread decltype ( p ) & p;
 thread decltype ( params ) & params;
 auto operator ( ) ( int selector , int tweak , float power ) const {
 auto [ a , b ] = general_extended_pair ( selector , E ( p . x ) , E ( p . y ) , E ( p . z ) );
 if ( tweak == 1 || tweak == 3 ) {
 power = - power;
 b = abs ( b );

}
 E value = extendedAtan2 ( a , b );
 if ( tweak == 2 || tweak == 3 ) value -= abs ( E ( params [ 11 ] ) + value ) - abs ( E ( params [ 11 ] ) - value );
 return extendedSinCos ( value * power );

}

}
;
 auto angle = NativeClosure0 {
 p , params
}
;
 const auto t = angle ( int ( params [ 3 ] ) , int ( params [ 4 ] ) , params [ 0 ] );
 const auto f = angle ( int ( params [ 5 ] ) , int ( params [ 6 ] ) , params [ 1 ] );
 const float ct = float ( t . cos ) , st = float ( t . sin ) , cp = float ( f . cos ) , sp = float ( f . sin );
 const E x = p . x , y = p . y , z = p . z;
 const E radius = native_formula_length3 ( x , y , z );
 const float rp = float ( extendedExp2 ( E ( params [ 0 ] ) * extendedLog2 ( abs ( radius ) ) , false ) );
 E ox , oy , oz;
 if ( int ( params [ 10 ] ) == 0 ) {
 ox = E ( ct ) * cp;
 oy = E ( st ) * cp;
 oz = E ( params [ 2 ] ) * sp;

}
 else if ( int ( params [ 10 ] ) == 1 ) {
 ox = E ( ct ) * sp;
 oy = E ( st ) * sp;
 oz = E ( params [ 2 ] ) * cp;

}
 else {
 ox = E ( ct ) * cp;
 oy = st;
 oz = ( E ( params [ 2 ] ) * ct ) * sp;

}
 struct NativeClosure1 {
 thread decltype ( params ) & params;
 auto operator ( ) ( int i ) const {
 return int ( params [ 7 + i ] ) == - 1 ? abs ( E ( params [ 12 + i ] ) ) : E ( params [ 12 + i ] );

}

}
;
 auto c = NativeClosure1 {
 params
}
;
 return FormulaVec3 ( float ( ox * rp + c ( 0 ) ) , float ( oy * rp + c ( 1 ) ) , float ( oz * rp + c ( 2 ) ) );

}
 FormulaVec3 General_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_GENERAL > ( p , params , de );

}

}}
