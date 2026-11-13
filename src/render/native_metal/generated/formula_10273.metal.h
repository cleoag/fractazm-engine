// Generated from 3D/BulboxP-2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10273 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BULBOXP2 = 10273;
 template < > FormulaVec3 execute_formula < FORMULA_BULBOXP2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 struct NativeClosure0 {
 thread decltype ( params ) & params;
 auto operator ( ) ( E v ) const {
 return E ( float ( abs ( v + E ( params [ 1 ] ) ) + ( - abs ( v - E ( params [ 1 ] ) ) - v ) ) );

}

}
;
 const auto fold = NativeClosure0 {
 params
}
;
 E x = fold ( E ( p . x ) ) , y = fold ( E ( p . y ) ) , z = fold ( E ( p . z ) );
 const E square = x * x + ( y * y + z * z );
 const E savedSquare = float ( square ) , radius = float ( sqrt ( square ) );
 x = float ( x * params [ 0 ] - E ( params [ 5 ] ) );
 y = float ( y * params [ 0 ] - E ( params [ 6 ] ) );
 z = float ( z * params [ 0 ] - E ( params [ 7 ] ) );
 if ( int ( params [ 9 ] ) == 0 && radius >= E ( 1 ) ) {
 x = float ( x + E ( params [ 10 ] ) );
 y = float ( y + E ( params [ 11 ] ) );
 z = float ( z + E ( params [ 12 ] ) );

}
 else {
 const bool interpolate = ! ( E ( params [ 2 ] ) >= radius );
 E blend ( 0 ) , complement ( 1 );
 if ( interpolate ) {
 const E fraction = ( radius - E ( params [ 2 ] ) ) / ( E ( 1 ) - E ( params [ 2 ] ) );
 blend = float ( fraction );
 complement = float ( E ( 1 ) - fraction );

}
 const E rxy = float ( sqrt ( ( y * y + x * x ) + abs ( E ( params [ 4 ] ) ) ) );
 const E inverse = float ( E ( params [ 3 ] ) / ( savedSquare * savedSquare ) );
 const E zr = z / rxy;
 const E angular = float ( - ( zr * zr ) + E ( 1 ) );
 const E nx = ( ( x * x + ( - ( y * y ) ) ) * angular ) * inverse;
 const E ny = ( - ( x * y ) * angular ) * inverse;
 E nz = - ( z + z );
 if ( interpolate ) {
 nz = ( ( nz * params [ 8 ] ) * rxy ) * inverse;
 z = float ( ( nz * complement + z * blend ) + E ( params [ 12 ] ) );
 y = float ( ( ny * complement + y * blend ) + E ( params [ 11 ] ) );
 x = float ( ( nx * complement + x * blend ) + E ( params [ 10 ] ) );

}
 else {
 nz = ( ( nz * rxy ) * inverse ) * params [ 8 ];
 z = float ( nz + E ( params [ 12 ] ) );
 y = float ( ny + E ( params [ 11 ] ) );
 x = float ( nx + E ( params [ 10 ] ) );

}

}
 return FormulaVec3 ( float ( x + E ( params [ 5 ] ) ) , float ( y + E ( params [ 6 ] ) ) , float ( z + E ( params [ 7 ] ) ) );

}

}}
