// Generated from 3Da/Amazing_Surf.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1509 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_AMAZING_SURF = 1509;
 template < > FormulaVec3 execute_formula < FORMULA_AMAZING_SURF > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
#pragma clang fp contract(off)
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 E previous ( params [ DIFS_VARYSCALE_SLOT ] );
 if ( native_formula_counter ( params ) <= 0.0f ) {
 previous = E ( params [ 0 ] );
 native_increment_formula_counter ( params );

}
 const float vary = static_cast < float > ( previous + ( abs ( previous ) - E ( 1 ) ) * E ( params [ 13 ] ) );
 params [ DIFS_VARYSCALE_SLOT ] = vary;
 const E fold ( params [ 3 ] );
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 auto operator ( ) ( E v ) const {
 return abs ( v + fold ) - ( abs ( v - fold ) + v );

}

}
;
 auto box = NativeClosure0 {
 fold
}
;
 const E fx = box ( E ( p . x ) ) , fy = box ( E ( p . y ) );
 const float storedX = static_cast < float > ( fx ) , storedY = static_cast < float > ( fy );
 E rr = fy * fy + fx * fx;
 if ( int ( params [ 14 ] ) == 0 ) rr += E ( p . z ) * E ( p . z );
 const E m = native_box_inner_unordered ( float ( rr ) , params ) ? E ( native_aboxmod1_inner_scale ( params ) ) : ( E ( 1 ) >= rr ? E ( vary ) / rr : E ( vary ) );
 de = static_cast < float > ( E ( de ) * m );
 const E x = E ( storedX ) * m + E ( params [ 15 ] );
 const E y = E ( storedY ) * m + E ( params [ 16 ] );
 const E z = E ( p . z ) * m + E ( params [ 17 ] );
 E matrix [ 9 ];
 for ( int i = 0;
 i < 9;
 ++ i ) matrix [ i ] = E ( static_cast < float > ( params [ 4 + i ] ) );
 return FormulaVec3 ( static_cast < float > ( ( x * matrix [ 0 ] + y * matrix [ 1 ] ) + z * matrix [ 2 ] ) , static_cast < float > ( ( x * matrix [ 3 ] + y * matrix [ 4 ] ) + z * matrix [ 5 ] ) , static_cast < float > ( ( x * matrix [ 6 ] + y * matrix [ 7 ] ) + z * matrix [ 8 ] ) );

}
 FormulaVec3 Amazing_Surf_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_AMAZING_SURF > ( p , params , de );

}

}}
