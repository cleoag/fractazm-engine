// Generated from 3Da/OctKoch.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10039 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_OCTKOCH = 10039;
 template < > FormulaVec3 execute_formula < FORMULA_OCTKOCH > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 const E x = abs ( E ( p . x ) ) , y = abs ( E ( p . y ) ) , z = abs ( E ( p . z ) );
 E hi = ( x * params [ 4 ] + y * params [ 5 ] ) + z * params [ 6 ];
 E mid = ( x * params [ 7 ] + y * params [ 8 ] ) + z * params [ 9 ];
 E lo = ( x * params [ 10 ] + y * params [ 11 ] ) + z * params [ 12 ];
 if ( ! ( hi > mid ) ) native_std :: swap ( hi , mid );
 if ( ! ( hi > lo ) ) native_std :: swap ( hi , lo );
 if ( ! ( mid > lo ) ) native_std :: swap ( mid , lo );
 E xv = float ( hi ) , yv = float ( mid ) , zv = float ( lo );
 const E k = E ( params [ 0 ] ) - 1 , f = k * params [ 22 ];
 xv = float ( xv - k * params [ 23 ] );
 xv = abs ( xv + f ) + ( - abs ( xv - f ) - xv );
 if ( int ( params [ 24 ] ) != 0 ) xv = abs ( xv );
 xv = float ( xv );
 hi = yv;
 mid = xv;
 lo = zv;
 if ( ! ( hi >= mid ) ) native_std :: swap ( hi , mid );
 if ( ! ( hi >= lo ) ) native_std :: swap ( hi , lo );
 if ( ! ( mid >= lo ) ) native_std :: swap ( mid , lo );
 const E r0 = ( hi * params [ 13 ] + mid * params [ 14 ] ) + lo * params [ 15 ];
 const E r1 = ( hi * params [ 16 ] + mid * params [ 17 ] ) + lo * params [ 18 ];
 const E r2 = ( hi * params [ 19 ] + mid * params [ 20 ] ) + lo * params [ 21 ];
 de = float ( E ( de ) * params [ 0 ] );
 return FormulaVec3 ( float ( ( r0 - k * params [ 1 ] ) * params [ 0 ] ) , float ( ( r1 - k * params [ 2 ] ) * params [ 0 ] ) , float ( ( r2 - k * params [ 3 ] ) * params [ 0 ] ) );

}

}}
