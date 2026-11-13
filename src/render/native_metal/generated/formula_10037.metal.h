// Generated from 3Da/MengerKochV2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10037 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGERKOCHV2 = 10037;
 template < > FormulaVec3 execute_formula < FORMULA_MENGERKOCHV2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 struct NativeClosure0 {
 thread decltype ( params ) & params;
 auto operator ( ) ( E x , E y , E z , int first ) const {
 return ( x * E ( params [ first ] ) + y * E ( params [ first + 1 ] ) ) + z * E ( params [ first + 2 ] );

}

}
;
 const auto dot = NativeClosure0 {
 params
}
;
 const E x = abs ( E ( p . x ) ) , y = abs ( E ( p . y ) ) , z = abs ( E ( p . z ) );
 E maximum = dot ( x , y , z , 4 ) , middle = dot ( x , y , z , 7 ) , minimum = dot ( x , y , z , 10 );
 if ( ! ( maximum >= middle ) ) native_std :: swap ( maximum , middle );
 if ( ! ( maximum >= minimum ) ) native_std :: swap ( maximum , minimum );
 if ( ! ( middle >= minimum ) ) native_std :: swap ( middle , minimum );
 if ( ! ( params [ 22 ] <= 0 ) ) maximum = E ( params [ 22 ] ) - abs ( E ( params [ 22 ] ) - maximum );
 if ( ! ( params [ 23 ] <= 0 ) ) middle = E ( params [ 23 ] ) - abs ( E ( params [ 23 ] ) - middle );
 const E scale ( params [ 0 ] ) , k = scale - E ( 1 );
 const E outX = dot ( maximum , middle , minimum , 13 ) * scale - k * E ( params [ 1 ] );
 const E outY = dot ( maximum , middle , minimum , 16 ) * scale - k * E ( params [ 2 ] );
 E outZ = dot ( maximum , middle , minimum , 19 ) * scale;
 const E kz = k * E ( params [ 3 ] );
 if ( ! ( kz * E ( params [ 24 ] ) > outZ ) ) outZ -= kz;
 de = float ( E ( de ) * scale );
 return FormulaVec3 ( float ( outX ) , float ( outY ) , float ( outZ ) );

}

}}
