// Generated from 3Da/ABoxVS_icen1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10184 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXVS_ICEN1 = 10184;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXVS_ICEN1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 float previous = params [ DIFS_VARYSCALE_SLOT ];
 if ( native_formula_counter ( params ) <= 0 ) {
 previous = params [ 0 ];
 native_increment_formula_counter ( params );

}
 const float vary = float ( E ( previous ) + ( abs ( E ( previous ) ) - 1 ) * params [ 3 ] );
 params [ DIFS_VARYSCALE_SLOT ] = vary;
 const E fold = params [ 2 ] , twice = fold + fold;
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
 const E rr = ( z * z + y * y ) + x * x , minr2 = E ( params [ 1 ] ) * params [ 1 ];
 const E m = minr2 >= rr ? E ( vary ) * params [ 4 ] / minr2 : E ( params [ 4 ] ) >= rr ? E ( vary ) / ( rr / params [ 4 ] ) : E ( vary );
 de = float ( E ( de ) * m );
 if ( int ( params [ 8 ] ) == 0 ) return FormulaVec3 ( float ( ( x * m + params [ 9 ] ) - params [ 5 ] ) , float ( ( y * m + params [ 10 ] ) - params [ 6 ] ) , float ( ( z * m + params [ 11 ] ) - params [ 7 ] ) );
 return FormulaVec3 ( float ( ( x * m - params [ 9 ] ) + params [ 5 ] ) , float ( ( y * m - params [ 10 ] ) + params [ 6 ] ) , float ( ( z * m - params [ 11 ] ) + params [ 7 ] ) );

}
 FormulaVec3 ABoxVS_icen1_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXVS_ICEN1 > ( p , params , de );

}

}}
