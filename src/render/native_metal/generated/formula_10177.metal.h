// Generated from 3Da/MengerHyper.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10177 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MENGERHYPER = 10177;
 template < > FormulaVec3 execute_formula < FORMULA_MENGERHYPER > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E x = abs ( E ( p . x ) ) , y = abs ( E ( p . y ) ) , z = abs ( E ( p . z ) );
 const E rr = ( x * x + y * y ) + z * z , maximum = E ( params [ 7 ] ) * params [ 9 ] , minimum = E ( params [ 8 ] ) * params [ 9 ];
 E factor = 1;
 if ( maximum >= rr ) factor = maximum / ( minimum >= rr ? minimum : rr );
 E a = z * factor , b = x * factor , c = y * factor;
 struct NativeClosure0 {
 auto operator ( ) ( thread E & u , thread E & v ) const {
 const E saved = u;
 u = v;
 v = saved;

}

}
;
 const auto swap = NativeClosure0 {

}
;
 struct NativeClosure1 {
 thread decltype ( swap ) & swap;
 auto operator ( ) ( thread E & u , thread E & v , thread E & w ) const {
 if ( ! ( u >= v ) ) swap ( u , v );
 if ( ! ( u >= w ) ) swap ( u , w );
 if ( ! ( v >= w ) ) swap ( v , w );

}

}
;
 const auto sort = NativeClosure1 {
 swap
}
;
 sort ( a , b , c );
 E mid = b + params [ 5 ] , high = a + params [ 4 ] , low = c + params [ 6 ];
 const E rr2 = ( mid * mid + high * high ) + low * low;
 factor = rr2 >= E ( params [ 7 ] ) ? E ( 1 ) : E ( params [ 7 ] ) / ( rr2 >= E ( params [ 8 ] ) ? rr2 : E ( params [ 8 ] ) );
 mid = mid * factor - params [ 5 ];
 high = high * factor - params [ 4 ];
 low = low * factor - params [ 6 ];
 if ( int ( params [ 10 ] ) != 0 ) sort ( mid , high , low );
 else swap ( mid , high );
 const E scale = params [ 0 ] , k = scale - E ( 1 ) , kz = ( k / scale ) * params [ 3 ];
 de = float ( E ( de ) * scale );
 mid = mid * scale - k * params [ 1 ];
 high = high * scale - k * params [ 2 ];
 low = ( kz - abs ( low - kz ) ) * scale;
 return FormulaVec3 ( float ( mid ) , float ( high ) , float ( low ) );

}

}}
