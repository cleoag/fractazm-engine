// Generated from 3Da/ABoxMod2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5079 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXMOD2 = 5079;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXMOD2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 struct NativeClosure0 {
 auto operator ( ) ( E v , E f ) const {
 return abs ( v + f ) - ( abs ( v - f ) + v );

}

}
;
 auto box = NativeClosure0 {

}
;
 const E x = box ( E ( p . x ) , E ( params [ 3 ] ) ) , y = box ( E ( p . y ) , E ( params [ 3 ] ) ) , z = box ( E ( p . z ) , E ( params [ 4 ] ) );
 E rr;
 if ( abs ( z ) >= E ( params [ 5 ] ) ) {
 const E cap = abs ( z ) - params [ 5 ];
 rr = ( cap * cap + y * y ) + x * x;

}
 else rr = y * y + x * x;
 const E m = ! ( rr >= params [ 2 ] ) ? E ( params [ 1 ] ) : E ( 1 ) >= rr ? E ( params [ 0 ] ) / rr : E ( params [ 0 ] );
 de = float ( E ( de ) * m );
 return FormulaVec3 ( float ( x * m + params [ 6 ] ) , float ( y * m + params [ 7 ] ) , float ( z * m + params [ 8 ] ) );

}
 FormulaVec3 ABoxMod2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXMOD2 > ( p , params , de );

}

}}
