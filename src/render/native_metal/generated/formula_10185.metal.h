// Generated from 3Da/ABoxVSWavy.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10185 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXVSWAVY = 10185;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXVSWAVY > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using numeric :: extendedSinCos;
 numeric :: ExtendedScope scope;
 float previous = params [ DIFS_VARYSCALE_SLOT ];
 if ( native_formula_counter ( params ) <= 0 ) {
 previous = params [ 0 ];
 native_increment_formula_counter ( params );

}
 const float vary = float ( E ( previous ) + ( abs ( E ( previous ) ) - 1 ) * params [ 4 ] );
 params [ DIFS_VARYSCALE_SLOT ] = vary;
 const E fold = params [ 3 ] , twice = fold + fold;
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 thread decltype ( twice ) & twice;
 auto operator ( ) ( E v ) const {
 if ( v >= fold ) return - v + twice;
 if ( ! ( v >= - fold ) ) return - v - twice;
 return v;

}

}
;
 auto box = NativeClosure0 {
 fold , twice
}
;
 const E x = box ( E ( p . x ) ) , y = box ( E ( p . y ) ) , z = box ( E ( p . z ) );
 const E ax = x * params [ 6 ] , ay = y * params [ 6 ] , az = z * params [ 6 ];
 struct NativeClosure1 {
 auto operator ( ) ( E a ) const {
 return ( numeric :: signExponent ( a ) & 0x7fff ) != 0x7fff && abs ( a ) >= E ( 0x1p63f );

}

}
;
 auto rangeFailure = NativeClosure1 {

}
;
 if ( rangeFailure ( ax ) || rangeFailure ( ay ) || rangeFailure ( az ) ) {
 const E nan = numeric :: extendedNaN ( ) , m = params [ 1 ];
 de = float ( E ( de ) * m );
 if ( rangeFailure ( ax ) ) return FormulaVec3 ( float ( nan ) , float ( nan ) , float ( nan ) );
 if ( rangeFailure ( ay ) ) return FormulaVec3 ( float ( nan ) , float ( nan ) , float ( x * m + params [ 12 ] ) );
 return FormulaVec3 ( float ( nan ) , float ( x * m + params [ 11 ] ) , float ( y * m + params [ 12 ] ) );

}
 const auto tx = extendedSinCos ( ax ) , ty = extendedSinCos ( ay ) , tz = extendedSinCos ( az );
 const float cx = float ( tx . cos ) , sx = float ( tx . sin ) , cy = float ( ty . cos ) , sy = float ( ty . sin ) , cz = float ( tz . cos ) , sz = float ( tz . sin );
 const float rho = float ( native_formula_length3 ( z , y , x ) );
 const E theta1 = ( ( E ( cx ) * sy + E ( cy ) * sz ) + E ( cz ) * sx ) * params [ 5 ];
 const E theta2 = ( ( E ( cx ) + cy ) + cz ) * params [ 7 ];
 const E r = int ( params [ 9 ] ) == 1 ? theta1 + ( theta2 + params [ 8 ] ) * rho : ( theta1 + theta2 ) + E ( rho ) * params [ 8 ];
 const E rr = r * r;
 const E m = ! ( rr >= params [ 2 ] ) ? E ( params [ 1 ] ) : E ( 1 ) >= rr ? E ( vary ) / rr : E ( vary );
 de = float ( E ( de ) * m );
 return FormulaVec3 ( float ( x * m + params [ 10 ] ) , float ( y * m + params [ 11 ] ) , float ( z * m + params [ 12 ] ) );

}
 FormulaVec3 ABoxVSWavy_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXVSWAVY > ( p , params , de );

}

}}
