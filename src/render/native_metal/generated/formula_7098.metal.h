// Generated from 3D/MandelView.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7098 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MANDELVIEW = 7098;
 template < > FormulaVec3 execute_formula < FORMULA_MANDELVIEW > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const E x ( p . x ) , y ( p . y ) , z ( p . z ) , power ( params [ 0 ] );
 struct NativeClosure0 {
 thread decltype ( power ) & power;
 auto operator ( ) ( E a , E b ) const {
 return numeric :: extendedSinCos ( numeric :: extendedAtan2 ( a , b ) * power );

}

}
;
 auto pair = NativeClosure0 {
 power
}
;
 const auto first = pair ( y , x ) , second = pair ( z , y ) , third = pair ( z , x );
 const E c1 ( static_cast < float > ( first . cos ) ) , s1 ( static_cast < float > ( first . sin ) );
 const E c2 ( static_cast < float > ( second . cos ) ) , s2 ( static_cast < float > ( second . sin ) );
 const E c3 ( static_cast < float > ( third . cos ) ) , s3 ( static_cast < float > ( third . sin ) );
 const E radius ( static_cast < float > ( numeric :: extendedExp2 ( power * numeric :: extendedLog2 ( native_formula_length3 ( z , y , x ) ) , false ) ) );
 return FormulaVec3 ( static_cast < float > ( ( c1 * c3 - ( s2 * s3 ) * s1 ) * radius + E ( params [ 2 ] ) ) , static_cast < float > ( s1 * c2 * radius + E ( params [ 3 ] ) ) , static_cast < float > ( ( ( s2 * c3 ) * s1 + c1 * s3 ) * radius * E ( params [ 1 ] ) + E ( params [ 4 ] ) ) );

}

}}
