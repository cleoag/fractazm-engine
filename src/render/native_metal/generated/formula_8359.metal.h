// Generated from 3D/MsltoeSym4.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8359 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MSLTOESYM4 = 8359;
 template < > FormulaVec3 execute_formula < FORMULA_MSLTOESYM4 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 E x = p . x , y = p . y , z = p . z;
 if ( ! ( abs ( x ) > abs ( z ) * params [ 0 ] ) ) native_std :: swap ( x , z );
 if ( ! ( abs ( x ) > abs ( y ) * params [ 1 ] ) ) native_std :: swap ( x , y );
 if ( ! ( abs ( z ) > abs ( z ) * params [ 2 ] ) ) native_std :: swap ( y , z );
 if ( x * z >= E ( 0 ) ) z = - z;
 if ( x * y >= E ( 0 ) ) y = - y;
 const E xy = x * y , xz = x * z;
 return FormulaVec3 ( float ( ( x * x - y * y ) - z * z + params [ 3 ] ) , float ( xy + xy + params [ 4 ] ) , float ( xz + xz + params [ 5 ] ) );

}
 FormulaVec3 MsltoeSym4_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_MSLTOESYM4 > ( p , params , de );

}

}}
