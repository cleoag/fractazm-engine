// Generated from Ads/_helispiral.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10010 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__HELISPIRAL = 10010;
 template < > FormulaVec3 execute_formula < FORMULA__HELISPIRAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const E px ( p . x ) , py ( p . y ) , pz ( p . z );
 const E x ( static_cast < float > ( px * E ( params [ 0 ] ) + py * E ( params [ 1 ] ) + pz * E ( params [ 2 ] ) ) );
 const E y ( static_cast < float > ( pz * E ( params [ 5 ] ) + ( py * E ( params [ 4 ] ) + px * E ( params [ 3 ] ) ) ) );
 const float storedZ = static_cast < float > ( px * E ( params [ 6 ] ) + py * E ( params [ 7 ] ) + pz * E ( params [ 8 ] ) );
 const E z ( storedZ );
 const E a = int ( params [ 12 ] ) == 1 ? E ( params [ 15 ] ) : x;
 const E b = int ( params [ 12 ] ) == 1 ? E ( params [ 16 ] ) : y;
 const E h = int ( params [ 11 ] ) == 1 ? E ( params [ 17 ] ) : z;
 const E angle = sqrt ( a * a + b * b ) * E ( params [ 10 ] ) + h * E ( params [ 9 ] ) + E ( params [ 13 ] ) * E ( params [ 14 ] );
 if ( abs ( angle ) >= E ( 0x1p63f ) ) return FormulaVec3 ( static_cast < float > ( numeric :: extendedNaN ( ) ) );
 const auto trig = numeric :: extendedSinCos ( angle );
 const E nx ( static_cast < float > ( trig . cos * x + trig . sin * y ) );
 const E ny ( static_cast < float > ( y * trig . cos - x * trig . sin ) );
 return FormulaVec3 ( static_cast < float > ( nx * E ( params [ 0 ] ) + ny * E ( params [ 3 ] ) + z * E ( params [ 6 ] ) ) , static_cast < float > ( z * E ( params [ 7 ] ) + ( ny * E ( params [ 4 ] ) + nx * E ( params [ 1 ] ) ) ) , static_cast < float > ( nx * E ( params [ 2 ] ) + ny * E ( params [ 5 ] ) + z * E ( params [ 8 ] ) ) );

}

}}
