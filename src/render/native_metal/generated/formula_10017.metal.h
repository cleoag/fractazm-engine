// Generated from Ads/_Fold45double.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10017 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLD45DOUBLE = 10017;
 template < > FormulaVec3 execute_formula < FORMULA__FOLD45DOUBLE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const bool opposite [ ] = {
 cpu_bitcast_to_int ( params [ 2 ] ) != 0 , cpu_bitcast_to_int ( params [ 3 ] ) != 0 , cpu_bitcast_to_int ( params [ 4 ] ) != 0
}
;
 float v [ ] = {
 p . x , p . y , p . z
}
;
 const E factor ( params [ 6 ] );
 if ( cpu_bitcast_to_int ( params [ 1 ] ) ) {
 const E shift = ( ( opposite [ 0 ] ? E ( v [ 0 ] ) : - E ( v [ 0 ] ) ) + ( opposite [ 1 ] ? E ( v [ 1 ] ) : - E ( v [ 1 ] ) ) + ( opposite [ 2 ] ? E ( v [ 2 ] ) : - E ( v [ 2 ] ) ) ) * factor;
 for ( int i = 0;
 i < 3;
 ++ i ) v [ i ] = float ( - ( opposite [ i ] ? E ( v [ i ] ) - shift : E ( v [ i ] ) + shift ) );

}
 const E k = abs ( E ( params [ 0 ] ) );
 E delta = ( ( opposite [ 0 ] ? - E ( v [ 0 ] ) : E ( v [ 0 ] ) ) + ( opposite [ 1 ] ? - E ( v [ 1 ] ) : E ( v [ 1 ] ) ) + ( opposite [ 2 ] ? - E ( v [ 2 ] ) : E ( v [ 2 ] ) ) ) - k;
 if ( ! ( delta >= E ( 0 ) ) ) {
 delta = ( delta + k ) + ( k + E ( params [ 5 ] ) );
 if ( delta > E ( 0 ) ) return FormulaVec3 ( v [ 0 ] , v [ 1 ] , v [ 2 ] );

}
 delta *= factor;
 for ( int i = 0;
 i < 3;
 ++ i ) v [ i ] = float ( opposite [ i ] ? E ( v [ i ] ) + delta : E ( v [ i ] ) - delta );
 return FormulaVec3 ( v [ 0 ] , v [ 1 ] , v [ 2 ] );

}

}}
