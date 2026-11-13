// Generated from 3Da/Mandalay-KIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10046 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MANDALAY_KIFS = 10046;
 template < > FormulaVec3 execute_formula < FORMULA_MANDALAY_KIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 numeric :: ExtendedScope scope;
 E x = abs ( E ( p . x ) ) , y = abs ( E ( p . y ) ) , z = abs ( E ( p . z ) );
 if ( ! ( x >= y ) ) native_std :: swap ( x , y );
 if ( ! ( x >= z ) ) native_std :: swap ( x , z );
 if ( ! ( y >= z ) ) native_std :: swap ( y , z );
 const E fold = params [ 3 ] , tower = params [ 13 ] , ztower = params [ 14 ];
 const E fx = float ( x - ( fold + fold ) );
 E xf = float ( fold - abs ( x - fold ) ) , yf = float ( fold - abs ( y - fold ) );
 const E zf = float ( ! ( E ( 0 ) >= ztower ) ? ztower - abs ( z - ztower ) : ztower + z );
 if ( ! ( E ( 0 ) >= fx ) && ! ( y >= fx ) ) {
 if ( tower + y >= fx ) {
 xf = - y;
 yf = float ( fold - abs ( ( ( x - fold ) - fold ) - fold ) );

}
 else {
 xf = float ( xf + tower );
 yf = float ( fold - abs ( ( tower - fold ) + y ) );

}

}
 const E rx = ( zf * params [ 10 ] + yf * params [ 11 ] ) + xf * params [ 12 ];
 const E ry = ( zf * params [ 7 ] + yf * params [ 8 ] ) + xf * params [ 9 ];
 const E rz = ( zf * params [ 4 ] + yf * params [ 5 ] ) + xf * params [ 6 ];
 const E rr = ( ( rz * rz + ry * ry ) + rx * rx ) * params [ 15 ];
 const E m = native_box_inner_unordered ( float ( rr ) , params ) ? E ( native_aboxmod1_inner_scale ( params ) ) : E ( 1 ) >= rr ? E ( params [ 0 ] ) / rr : E ( params [ 0 ] );
 de = float ( E ( de ) * m );
 return FormulaVec3 ( float ( rx * m + params [ 16 ] ) , float ( ry * m + params [ 17 ] ) , float ( rz * m + params [ 18 ] ) );

}

}}
