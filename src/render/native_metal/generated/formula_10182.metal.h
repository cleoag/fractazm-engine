// Generated from 3Da/ABoxVS_icen2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10182 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXVS_ICEN2 = 10182;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXVS_ICEN2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
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
 const bool swap = int ( params [ 9 ] ) != 0;
 const E radius = swap ? ( ( z + params [ 11 ] ) * z + ( y + params [ 12 ] ) * y ) + ( x + params [ 10 ] ) * x : ( ( z + params [ 6 ] ) * z + ( y + params [ 7 ] ) * y ) + ( x + params [ 8 ] ) * x;
 const E rr = numeric :: extendedLogPower ( radius , E ( params [ 5 ] ) );
 const E m = ! ( rr >= params [ 2 ] ) ? E ( params [ 1 ] ) : E ( 1 ) >= rr ? E ( vary ) / rr : E ( vary );
 de = float ( E ( de ) * m );
 const int c = swap ? 6 : 11;
 return FormulaVec3 ( float ( x * m + params [ c ] ) , float ( y * m + params [ c + 1 ] ) , float ( z * m + params [ c + 2 ] ) );

}
 FormulaVec3 ABoxVS_icen2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXVS_ICEN2 > ( p , params , de );

}

}}
