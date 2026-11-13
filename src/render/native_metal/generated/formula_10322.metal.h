// Generated from 4D/_setw.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10322 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SETW = 10322;
 template < > FormulaVec3 execute_formula < FORMULA_SETW > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int powerMode = int ( params [ 0 ] );
 const float power = params [ 1 ];
 const float xMul = params [ 2 ];
 const float yMul = params [ 3 ];
 const float zMul = params [ 4 ];
 float x = p . x , y = p . y , z = p . z;
 float w_val;
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 const E weights = ( E ( xMul ) + yMul ) + zMul;
 struct NativeClosure0 {
 thread decltype ( power ) & power;
 auto operator ( ) ( E value ) const {
 return numeric :: extendedExp2 ( E ( power ) * numeric :: extendedLog2 ( value ) , false );

}

}
;
 auto pow = NativeClosure0 {
 power
}
;
 if ( powerMode == 0 ) {
 w_val = float ( ( E ( x ) * xMul + ( E ( y ) * yMul + E ( z ) * zMul ) ) / weights );

}
 else {
 const float first = float ( pow ( abs ( E ( x ) ) ) * xMul );
 const float second = float ( pow ( abs ( E ( y ) ) ) * yMul + first );
 w_val = float ( pow ( ( pow ( abs ( E ( z ) ) ) * zMul + second ) / weights ) );

}
 de = w_val;
 params [ WRITE_J4_SLOT ] = w_val;
 return FormulaVec3 ( x , y , z );

}

}}
