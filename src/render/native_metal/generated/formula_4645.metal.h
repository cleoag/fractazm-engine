// Generated from 4D/_Sin4Dplus.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4645 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SIN4DPLUS = 4645;
 template < > FormulaVec3 execute_formula < FORMULA__SIN4DPLUS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using namespace numeric;
 ExtendedScope scope;
 const E x = float ( E ( p . x ) + params [ 0 ] ) , y = float ( E ( p . y ) + params [ 1 ] ) , z = float ( E ( p . z ) + params [ 2 ] ) , w = float ( E ( de ) + params [ 3 ] );
 const E r = native_formula_length3 ( w , y , z );
 if ( ( numeric :: signExponent ( x ) & 0x7fff ) != 0x7fff && abs ( x ) >= E ( 0x1p63f ) ) {
 de = float ( extendedNaN ( ) );
 return FormulaVec3 ( de , de , de );

}
 const auto sc = extendedSinCos ( x );
 const E log2e = numeric :: fromBits ( 0x3fff , 0xb8aa3b295c17f0bcULL );
 const E e = extendedExp2 ( r * log2e , false );
 const E a = ( e - E ( 1 ) / e ) * params [ 5 ] , b = ( e + E ( 1 ) / e ) * params [ 5 ];
 const float ox = float ( sc . sin * b );
 const E factor = ( sc . cos * a ) / r;
 const float oy = float ( y * factor ) , oz = float ( z * factor ) , ow = float ( w * factor );
 de = float ( E ( ow ) * params [ 4 ] );
 return FormulaVec3 ( float ( E ( ox ) * params [ 4 ] ) , float ( E ( oy ) * params [ 4 ] ) , float ( E ( oz ) * params [ 4 ] ) );

}

}}
