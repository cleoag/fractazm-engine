// Generated from 4Da/ABoxMod4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3857 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXMOD4D = 3857;
 template < > FormulaVec3 execute_formula < FORMULA_ABOXMOD4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const E fold ( params [ 1 ] );
 const E x = abs ( E ( p . x ) + fold ) - fold;
 const E y = abs ( E ( p . y ) + fold ) - fold;
 const E z = abs ( E ( p . z ) + fold ) - fold;
 const E w = abs ( E ( de ) + fold ) - fold;
 const E rr = w * w + z * z + y * y + x * x + E ( params [ 3 ] );
 const E scale ( params [ 0 ] );
 const E m = ( rr < E ( 1 ) || rr == E ( 1 ) ) ? scale / rr : scale;
 params [ FORMULA_STATE_X_SLOT ] = static_cast < float > ( E ( params [ FORMULA_STATE_X_SLOT ] ) * m );
 de = static_cast < float > ( w * m + E ( params [ 7 ] ) + E ( params [ 2 ] ) );
 return FormulaVec3 ( static_cast < float > ( x * m + E ( params [ 4 ] ) ) , static_cast < float > ( y * m + E ( params [ 5 ] ) ) , static_cast < float > ( z * m + E ( params [ 6 ] ) ) );

}
 FormulaVec3 ABoxMod4d_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXMOD4D > ( p , params , de );

}

}}
