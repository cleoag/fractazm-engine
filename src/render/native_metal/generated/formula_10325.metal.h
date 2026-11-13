// Generated from 4Da/ABoxVaryScale4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10325 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXVARYSCALE4D = 10325;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXVARYSCALE4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 E previous ( params [ DIFS_VARYSCALE_SLOT ] );
 if ( native_formula_counter ( params ) <= 0 ) {
 previous = E ( params [ 0 ] );
 native_increment_formula_counter ( params );

}
 const float vary = static_cast < float > ( previous + ( abs ( previous ) - E ( 1 ) ) * E ( params [ 4 ] ) );
 params [ DIFS_VARYSCALE_SLOT ] = vary;
 const E fold ( params [ 3 ] );
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 auto operator ( ) ( E v ) const {
 return abs ( v + fold ) - ( abs ( v - fold ) + v );

}

}
;
 auto folded = NativeClosure0 {
 fold
}
;
 const E x = folded ( E ( p . x ) ) , y = folded ( E ( p . y ) ) , z = folded ( E ( p . z ) ) , w = folded ( E ( de ) );
 E rr = z * z + y * y + x * x + w * w;
 const float power = static_cast < float > ( params [ 5 ] );
 if ( power != 1.0f ) rr = numeric :: extendedLogPower ( rr , E ( power ) );
 const E scale = ! ( rr >= E ( params [ 2 ] ) ) ? E ( vary ) / E ( params [ 2 ] ) : ( rr <= E ( 1 ) ? E ( vary ) / rr : E ( vary ) );
 params [ FORMULA_STATE_X_SLOT ] = static_cast < float > ( E ( params [ FORMULA_STATE_X_SLOT ] ) * scale );
 de = static_cast < float > ( w * scale + E ( params [ 6 ] ) + E ( params [ 10 ] ) );
 return FormulaVec3 ( static_cast < float > ( x * scale + E ( params [ 7 ] ) ) , static_cast < float > ( y * scale + E ( params [ 8 ] ) ) , static_cast < float > ( z * scale + E ( params [ 9 ] ) ) );

}

}}
