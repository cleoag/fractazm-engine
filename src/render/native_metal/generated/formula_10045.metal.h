// Generated from 3Da/AmazingBox2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10045 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_AMAZINGBOX2 = 10045;
 template < > FormulaVec3 execute_formula < FORMULA_AMAZINGBOX2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const E fold ( params [ 3 ] ) , px ( p . x ) , py ( p . y ) , pz ( p . z );
 struct NativeClosure0 {
 thread decltype ( fold ) & fold;
 auto operator ( ) ( E v ) const {
 return abs ( v + fold ) - ( abs ( v - fold ) + v );

}

}
;
 auto box = NativeClosure0 {
 fold
}
;
 const E ax ( static_cast < float > ( px * E ( params [ 20 ] ) - pz * E ( params [ 19 ] ) ) );
 const E az ( static_cast < float > ( px * E ( params [ 19 ] ) + pz * E ( params [ 20 ] ) ) );
 const E ay ( static_cast < float > ( ax * E ( params [ 17 ] ) + py * E ( params [ 18 ] ) ) );
 E x = ( box ( ax * E ( params [ 18 ] ) - py * E ( params [ 17 ] ) ) * E ( params [ 18 ] ) - ay * E ( params [ 17 ] ) ) * E ( params [ 20 ] ) - az * E ( params [ 19 ] );
 E y = box ( py ) , z = box ( pz );
 z += E ( params [ 15 ] );
 y += E ( params [ 14 ] );
 x += E ( params [ 13 ] );
 const E rr = ( ( z * z + y * y ) + x * x ) * E ( params [ 16 ] );
 const E scale ( params [ 0 ] );
 const E m = ! ( rr >= E ( params [ 2 ] ) ) ? E ( params [ 1 ] ) : ( E ( 1 ) >= rr ? scale / rr : scale );
 de = static_cast < float > ( E ( de ) * m );
 z = z * m - E ( params [ 15 ] ) * scale;
 y = y * m - E ( params [ 14 ] ) * scale;
 x = x * m - E ( params [ 13 ] ) * scale;
 E matrix [ 9 ];
 for ( int i = 0;
 i < 9;
 ++ i ) matrix [ i ] = E ( static_cast < float > ( params [ 4 + i ] ) );
 return FormulaVec3 ( static_cast < float > ( ( x * matrix [ 0 ] + y * matrix [ 1 ] ) + z * matrix [ 2 ] + E ( params [ 21 ] ) ) , static_cast < float > ( ( x * matrix [ 3 ] + y * matrix [ 4 ] ) + z * matrix [ 5 ] + E ( params [ 22 ] ) ) , static_cast < float > ( ( x * matrix [ 6 ] + y * matrix [ 7 ] ) + z * matrix [ 8 ] + E ( params [ 23 ] ) ) );

}

}}
