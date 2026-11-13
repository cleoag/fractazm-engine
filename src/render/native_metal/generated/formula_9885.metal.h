// Generated from 3D/Fuzzy.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9885 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FUZZY = 9885;
 template < > FormulaVec3 execute_formula < FORMULA_FUZZY > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using namespace numeric;
 ExtendedScope scope;
 const E n = ( E ( 1 ) + E ( int ( params [ ITER_COUNT_SLOT ] ) ) ) * params [ 0 ];
 const E radius = params [ FORMULA_ROUT_SLOT ] , v = E ( p . z ) / sqrt ( radius );
 const E phi = extendedAtan2 ( v , sqrt ( E ( 1 ) - v * v ) ) * n;
 const E theta = extendedAtan2 ( E ( p . y ) , E ( p . x ) ) * n;
 const auto ph = extendedSinCos ( phi ) , th = extendedSinCos ( theta );
 struct NativeClosure0 {
 auto operator ( ) ( E a ) const {
 return ( numeric :: signExponent ( a ) & 0x7fff ) != 0x7fff && abs ( a ) >= E ( 0x1p63f );

}

}
;
 auto rangeFailure = NativeClosure0 {

}
;
 if ( rangeFailure ( phi ) || rangeFailure ( theta ) ) {
 const E base = rangeFailure ( phi ) ? theta : ph . cos;
 const E rp = extendedLogPower ( base , E ( params [ 0 ] ) - 1 );
 return FormulaVec3 ( float ( extendedNaN ( ) ) , float ( extendedNaN ( ) ) , float ( E ( params [ 4 ] ) - radius * rp ) );

}
 const E rp = extendedLogPower ( radius , E ( params [ 0 ] ) - 1 ) , radial = ph . cos * rp;
 return FormulaVec3 ( float ( th . cos * radial + params [ 2 ] ) , float ( ( th . sin * radial ) * params [ 1 ] + params [ 3 ] ) , float ( E ( params [ 4 ] ) - ph . sin * rp ) );

}

}}
