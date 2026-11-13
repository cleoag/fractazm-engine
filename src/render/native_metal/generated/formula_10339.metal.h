// Generated from 4D/QuatPow.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10339 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_QUATPOW = 10339;
 template < > FormulaVec3 execute_formula < FORMULA_QUATPOW > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 E x = p . x , y = p . y , z = p . z , w = de;
 const E radius = native_formula_length3 ( w , y , z ) , epsilon = params [ 9 ];
 struct NativeClosure0 {
 auto operator ( ) ( E v ) const {
 const E infinity = native_std :: numeric_limits < float > :: infinity ( );
 return v == E ( 0 ) ? - infinity : v == infinity ? infinity : extendedLog2 ( v );

}

}
;
 const auto logarithm = NativeClosure0 {

}
;
 if ( radius > E ( 0 ) || radius < E ( 0 ) ) {
 const E direction = extendedAtan2 ( radius , x ) / radius;
 const E square = x * x + radius * radius;
 x = float ( epsilon > square ? E ( params [ 11 ] ) : ( extendedLn2 ( ) * logarithm ( square ) ) * params [ 10 ] );
 y = float ( y * direction );
 z = float ( z * direction );
 w = float ( w * direction );

}
 else x = float ( epsilon > x ? E ( params [ 11 ] ) : extendedLn2 ( ) * logarithm ( x ) );
 x = float ( x * params [ 4 ] );
 y = float ( y * params [ 5 ] );
 z = float ( z * params [ 6 ] );
 w = float ( w * params [ 7 ] );
 const E qx = float ( ( ( x * params [ 0 ] + ( - ( y * params [ 1 ] ) ) ) + ( - ( z * params [ 2 ] ) ) ) + ( - ( w * params [ 3 ] ) ) );
 const E qy = float ( ( ( x * params [ 1 ] + y * params [ 0 ] ) + z * params [ 3 ] ) + ( - ( w * params [ 2 ] ) ) );
 const E qz = float ( ( ( x * params [ 2 ] + ( - ( y * params [ 3 ] ) ) ) + z * params [ 0 ] ) + w * params [ 1 ] );
 const E qw = float ( ( ( x * params [ 3 ] + y * params [ 2 ] ) + ( - ( z * params [ 1 ] ) ) ) + w * params [ 0 ] );
 const E vectorRadius = native_formula_length3 ( qw , qy , qz );
 const auto sc = extendedSinCos ( vectorRadius );
 const E log2e = numeric :: fromBits ( 0x3fff , 0xb8aa3b295c17f0bcULL );
 const E exponential = extendedExp2 ( qx * log2e , false );
 x = float ( exponential * sc . cos );
 E factor = exponential * sc . sin;
 if ( ! ( epsilon > vectorRadius ) ) factor /= vectorRadius;
 y = float ( qy * factor );
 z = float ( qz * factor );
 w = float ( qw * factor );
 de = float ( w );
 return FormulaVec3 ( float ( x + E ( params [ 12 ] ) * params [ 8 ] ) , float ( y + E ( params [ 13 ] ) * params [ 8 ] ) , float ( z + E ( params [ 14 ] ) * params [ 8 ] ) );

}

}}
