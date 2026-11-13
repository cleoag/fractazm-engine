// Generated from dIFS/sliceIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8107 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SLICEIFS = 8107;
 template < > FormulaVec3 execute_formula < FORMULA_SLICEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const int apply = int ( params [ 6 ] );
 const FormulaVec3 q ( p . x * params [ 1 ] + params [ 4 ] , p . y * params [ 1 ] + params [ 3 ] , p . z * params [ 1 ] + params [ 2 ] );
 if ( apply ) params [ DIFS_VARYSCALE_SLOT ] *= params [ 1 ];
 const E x ( q . x ) , y ( q . y ) , z ( q . z );
 const E sector = numeric :: extendedAtan2 ( y , x ) * E ( params [ 9 ] ) / E ( params [ 7 ] );
 const E tooth = - ( ( abs ( ( sector - round ( sector ) ) * E ( params [ 7 ] ) ) / E ( params [ 0 ] ) - E ( 1 ) ) * E ( params [ 0 ] ) * E ( params [ 10 ] ) );
 const E shell = abs ( native_formula_length3 ( x , y , z ) - E ( params [ 5 ] ) + E ( params [ 8 ] ) ) - E ( params [ 8 ] );
 E distance = shell >= tooth ? shell : tooth;
 if ( ! apply ) distance /= E ( params [ 1 ] );
 de = static_cast < float > ( distance );
 return apply ? q : p;

}
 FormulaVec3 sliceIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SLICEIFS > ( p , params , de );

}

}}
