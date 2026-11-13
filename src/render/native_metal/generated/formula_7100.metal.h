// Generated from 3D/Riemann.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7100 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_RIEMANN = 7100;
 template < > FormulaVec3 execute_formula < FORMULA_RIEMANN > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 const ExtendedScope scope;
 const E invRadius = E ( 1 ) / sqrt ( E ( params [ FORMULA_ROUT_SLOT ] ) );
 const E x = E ( p . x ) * invRadius , y = E ( p . y ) * invRadius , z = E ( p . z ) * invRadius;
 const E projection = E ( 1 ) / ( y - E ( 1 ) ) , a = x * projection , b = z * projection;
 const E angleA = extendedAtan2 ( a + a , a * a - E ( 1 ) ) * E ( params [ 0 ] );
 const E angleB = extendedAtan2 ( b + b , b * b - E ( 1 ) ) * E ( params [ 0 ] );
 struct NativeClosure0 {
 auto operator ( ) ( E angle ) const {
 return ( numeric :: signExponent ( angle ) & 0x7fff ) != 0x7fff && abs ( angle ) >= E ( 0x1p63f );

}

}
;
 const auto missingPush = NativeClosure0 {

}
;
 if ( missingPush ( angleA ) || missingPush ( angleB ) ) return FormulaVec3 ( float ( extendedNaN ( ) ) , float ( extendedNaN ( ) ) , float ( extendedNaN ( ) ) );
 const auto trigA = extendedSinCos ( angleA ) , trigB = extendedSinCos ( angleB );
 const E c1 = trigA . sin / ( trigA . cos + E ( 1 ) );
 const E c2 = trigB . sin / ( trigB . cos + E ( 1 ) );
 const E d = c1 * c1 + c2 * c2 , invD = E ( 1 ) / ( d + E ( 1 ) );
 const E radius = extendedLogPower ( E ( params [ FORMULA_ROUT_SLOT ] ) , E ( params [ 0 ] ) * E ( params [ 1 ] ) );
 const E nx = ( c1 * invD ) * radius , ny = ( ( d - E ( 1 ) ) * invD ) * radius , nz = ( c2 * invD ) * radius;
 const FormulaVec3 original ( float ( ( nx + nx ) + E ( params [ 2 ] ) ) , float ( ny + E ( params [ 3 ] ) ) , float ( ( nz + nz ) + E ( params [ 4 ] ) ) );
 return FormulaVec3 ( mb3d_native_riemann :: reconstruct ( float3 ( original ) , angleA , angleB , radius , float3 ( params [ 2 ] , params [ 3 ] , params [ 4 ] ) , all ( isfinite ( float4 ( x , y , z , projection ) ) ) && all ( isfinite ( float4 ( a , b , a + a , b + b ) ) ) && all ( isfinite ( float2 ( a * a - 1.f , b * b - 1.f ) ) ) ) . value );

}

}}
