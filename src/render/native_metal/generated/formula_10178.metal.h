// Generated from 3Da/SierpinskiFull.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10178 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SIERPINSKIFULL = 10178;
 template < > FormulaVec3 execute_formula < FORMULA_SIERPINSKIFULL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 FormulaVec3 r = rotMatrix3FromFloats ( params , 4 ) * p;
 float a = r . x , b = r . y , c = r . z;
 struct NativeClosure0 {
 auto operator ( ) ( thread float & x , thread float & y ) const {
 float t = x;
 x = y;
 y = t;

}

}
;
 auto exchange = NativeClosure0 {

}
;
 struct NativeClosure1 {
 thread decltype ( a ) & a;
 thread decltype ( b ) & b;
 thread decltype ( c ) & c;
 thread decltype ( exchange ) & exchange;
 auto operator ( ) ( ) const {
 if ( ! ( a >= b ) ) exchange ( a , b );
 if ( ! ( a >= c ) ) exchange ( a , c );
 exchange ( a , b );
 if ( ! ( a >= c ) ) exchange ( a , c );

}

}
;
 auto positive = NativeClosure1 {
 a , b , c , exchange
}
;
 struct NativeClosure2 {
 thread decltype ( a ) & a;
 thread decltype ( b ) & b;
 thread decltype ( c ) & c;
 thread decltype ( exchange ) & exchange;
 auto operator ( ) ( ) const {
 a = - a;
 if ( a >= b ) exchange ( a , b );
 if ( a >= c ) exchange ( a , c );
 a = - a;
 exchange ( a , b );
 a = - a;
 if ( a >= c ) exchange ( a , c );
 a = - a;

}

}
;
 auto negative = NativeClosure2 {
 a , b , c , exchange
}
;
 if ( int ( params [ 24 ] ) == 0 ) {
 exchange ( a , b );
 positive ( );
 negative ( );

}
 else {
 negative ( );
 exchange ( a , b );
 positive ( );

}
 if ( ! ( 0.0f >= params [ 22 ] ) ) b = params [ 22 ] - fabs ( params [ 22 ] - b );
 if ( ! ( 0.0f >= params [ 23 ] ) ) a = params [ 23 ] - fabs ( params [ 23 ] - a );
 r = rotMatrix3FromFloats ( params , 13 ) * FormulaVec3 ( a , b , c );
 const float scale = params [ 0 ] , k = scale - 1.0f;
 de *= scale;
 return FormulaVec3 ( r . x * scale - k * params [ 1 ] , r . y * scale - k * params [ 2 ] , r . z * scale - k * params [ 3 ] );

}

}}
