// Generated from 3Da/CrossBiFold.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10038 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_CROSSBIFOLD = 10038;
 template < > FormulaVec3 execute_formula < FORMULA_CROSSBIFOLD > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 const E x = abs ( E ( p . x ) ) , y = abs ( E ( p . y ) ) , z = abs ( E ( p . z ) );
 E lo = ( x * params [ 10 ] + y * params [ 11 ] ) + z * params [ 12 ];
 E hi = ( x * params [ 4 ] + y * params [ 5 ] ) + z * params [ 6 ];
 E mid = ( x * params [ 7 ] + y * params [ 8 ] ) + z * params [ 9 ];
 struct NativeClosure0 {
 auto operator ( ) ( thread E & a , thread E & b , thread E & c ) const {
 if ( ! ( a >= b ) ) native_std :: swap ( a , b );
 if ( ! ( a >= c ) ) native_std :: swap ( a , c );
 if ( ! ( b >= c ) ) native_std :: swap ( b , c );

}

}
;
 auto sort = NativeClosure0 {

}
;
 sort ( hi , mid , lo );
 de = float ( E ( de ) * params [ 0 ] );
 E xv = float ( hi * params [ 0 ] ) , yv = float ( mid * params [ 0 ] ) , zv = float ( lo * params [ 0 ] );
 const E k = E ( params [ 0 ] ) - 1;
 xv = float ( xv - k * params [ 23 ] );
 const E f = k * params [ 22 ];
 xv = float ( abs ( xv + f ) + ( - abs ( xv - f ) - xv ) );
 hi = yv;
 mid = xv;
 lo = zv;
 sort ( hi , mid , lo );
 const E r2 = ( hi * params [ 19 ] + mid * params [ 20 ] ) + lo * params [ 21 ];
 const E r0 = ( hi * params [ 13 ] + mid * params [ 14 ] ) + lo * params [ 15 ];
 const E r1 = ( hi * params [ 16 ] + mid * params [ 17 ] ) + lo * params [ 18 ];
 return FormulaVec3 ( float ( r1 - k * params [ 2 ] ) , float ( r0 - k * params [ 3 ] ) , float ( k * params [ 1 ] - r2 ) );

}

}}
