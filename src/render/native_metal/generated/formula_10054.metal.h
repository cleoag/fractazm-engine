// Generated from 3Da/ABoxVaryScale.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10054 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXVARYSCALE = 10054;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXVARYSCALE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 E previous ( params [ DIFS_VARYSCALE_SLOT ] );
 if ( native_formula_counter ( params ) <= 0 ) {
 previous = E ( params [ 5 ] );
 native_increment_formula_counter ( params );

}
 const float vary = static_cast < float > ( previous + ( abs ( previous ) - E ( 1 ) ) * E ( params [ 3 ] ) );
 params [ DIFS_VARYSCALE_SLOT ] = vary;
 const E fold ( params [ 2 ] ) , twice = fold + fold;
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 thread decltype ( twice ) & twice;
 auto operator ( ) ( E v ) const {
 return v >= fold ? - v + twice : ( ! ( v >= - fold ) ? - v - twice : v );

}

}
;
 auto folded = NativeClosure0 {
 fold , twice
}
;
 const E x = folded ( E ( p . x ) ) , y = folded ( E ( p . y ) ) , z = folded ( E ( p . z ) );
 const E rr = numeric :: extendedLogPower ( z * z + y * y + x * x , E ( params [ 4 ] ) );
 const E scale = ! ( rr >= E ( params [ 1 ] ) ) ? E ( params [ 0 ] ) : ( rr <= E ( 1 ) ? E ( vary ) / rr : E ( vary ) );
 de = static_cast < float > ( E ( de ) * scale );
 return FormulaVec3 ( static_cast < float > ( x * scale + E ( params [ 6 ] ) ) , static_cast < float > ( y * scale + E ( params [ 7 ] ) ) , static_cast < float > ( z * scale + E ( params [ 8 ] ) ) );

}
 FormulaVec3 ABoxVaryScale_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXVARYSCALE > ( p , params , de );

}

}}
