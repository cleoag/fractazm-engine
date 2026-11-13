// Generated from 4D/Aexion1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10323 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_AEXION1 = 10323;
 template < > FormulaVec3 execute_formula < FORMULA_AEXION1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int first = int ( native_formula_counter ( params ) );
 float state [ ] = {
 p . x , p . y , p . z , de , params [ 1 ] , params [ 2 ] , params [ 3 ] , params [ 4 ] , params [ 0 ]
}
;
 const numeric :: X87Scope extendedScope;
 if ( first == 0 ) {
 for ( int base = 0;
 base <= 4;
 base += 4 ) {
 const numeric :: X87Real x = state [ base ] , y = state [ base + 1 ] , z = state [ base + 2 ];
 state [ base ] = float ( numeric :: x87Abs ( x + y + z ) + state [ 8 ] );
 state [ base + 1 ] = float ( numeric :: x87Abs ( z - x - y ) + state [ 8 ] );
 state [ base + 2 ] = float ( numeric :: x87Abs ( y - x - z ) + state [ 8 ] );
 state [ base + 3 ] = float ( numeric :: x87Abs ( x - y - z ) + state [ 8 ] );

}

}
 const numeric :: X87Real x = state [ 0 ] , y = state [ 1 ] , z = state [ 2 ] , w = state [ 3 ];
 const numeric :: X87Real xy = x * y , zw = z * w , a = x * x - y * y , b = z * z - w * w;
 state [ 0 ] = float ( a + ( zw + zw ) + state [ 4 ] );
 state [ 1 ] = float ( ( zw + zw ) - a + state [ 5 ] );
 state [ 2 ] = float ( b + ( xy + xy ) + state [ 6 ] );
 state [ 3 ] = float ( ( xy + xy ) - b + state [ 7 ] );
 if ( first == 0 ) native_formula_counter ( params ) = 1.f;
 params [ WRITE_J1_SLOT ] = state [ 4 ];
 params [ WRITE_J2_SLOT ] = state [ 5 ];
 params [ WRITE_J3_SLOT ] = state [ 6 ];
 params [ WRITE_J4_SLOT ] = state [ 7 ];
 de = state [ 3 ];
 return FormulaVec3 ( state [ 0 ] , state [ 1 ] , state [ 2 ] );

}

}}
