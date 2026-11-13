// Generated from 3D/TgladDihed2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10272 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TGLADDIHED2 = 10272;
 template < > FormulaVec3 execute_formula < FORMULA_TGLADDIHED2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 const numeric :: ExtendedScope scope;
 struct NativeClosure0 {
 auto operator ( ) ( E value ) const {
 return E ( float ( value ) );

}

}
;
 const auto store = NativeClosure0 {

}
;
 const E two ( params [ 7 ] ) , limit ( params [ 8 ] );
 const int outerCount = int ( params [ 0 ] ) & 7 , innerCount = int ( params [ 1 ] ) & 7;
 const int outerIterations = outerCount == 0 ? 1 : outerCount;
 const int innerIterations = innerCount == 0 ? 1 : innerCount;
 E x ( p . x ) , y ( p . y ) , z ( p . z );
 for ( int outer = 0;
 outer < outerIterations;
 ++ outer ) {
 const E xySquare = x * x + y * y , square = xySquare + z * z;
 if ( square >= limit ) return FormulaVec3 ( params [ 8 ] , params [ 8 ] , params [ 8 ] );
 const E radius = sqrt ( square );
 E radialPower = store ( radius );
 const E cosTheta = store ( z / radius );
 const E omega = ( sqrt ( xySquare ) - y ) / x , omegaSquare = omega * omega;
 const E omegaStored = store ( omega ) , omegaSquareStored = store ( omegaSquare );
 const E invOmega = E ( 1 ) / ( omegaSquare + E ( 1 ) );
 const E magnitude = invOmega * sqrt ( ( E ( 1 ) - cosTheta ) / ( E ( 1 ) + cosTheta ) );
 x = store ( magnitude * ( omegaSquareStored - E ( 1 ) ) );
 y = store ( ( magnitude + magnitude ) * omegaStored );
 for ( int inner = 0;
 inner < innerIterations;
 ++ inner ) {
 const E jExtended = x * x + y * y;
 if ( jExtended >= limit ) return FormulaVec3 ( params [ 8 ] , params [ 8 ] , params [ 8 ] );
 const E j = store ( jExtended );
 const E imaginary = store ( ( ( - y ) * ( j + E ( 1 ) ) ) * ( ( j - x * two ) - E ( 1 ) ) * ( ( j + x * two ) - E ( 1 ) ) );
 const E twiceY = y * two;
 const E real = store ( ( x * ( j - E ( 1 ) ) ) * ( ( ( j * j + j * two ) + twiceY * twiceY ) + E ( 1 ) ) );
 const E denominator = ( ( j - twiceY ) + E ( 1 ) ) * ( ( j + twiceY ) + E ( 1 ) );
 const E factor = two / ( denominator * denominator );
 x = store ( factor * real );
 y = store ( factor * imaginary );
 radialPower = store ( radialPower * radialPower );

}
 {
 const E old = x;
 x = y;
 y = old;

}
 const E mappedSquare = x * x + y * y , mappedRadius = sqrt ( mappedSquare );
 const E omega2 = ( mappedRadius - y ) / x , omega2Square = omega2 * omega2;
 const E omega2Stored = store ( omega2 ) , omega2SquareStored = store ( omega2Square );
 const E invOmega2 = E ( 1 ) / ( omega2Square + E ( 1 ) ) , invOmega2Stored = store ( invOmega2 );
 const E cosOmega = store ( ( E ( 1 ) - omega2SquareStored ) * invOmega2 );
 const E sinOmega = store ( ( omega2Stored + omega2Stored ) * invOmega2Stored );
 const E angleFactor = two / ( E ( 1 ) + store ( mappedSquare ) );
 const E angleFactorStored = store ( angleFactor ) , sinA = store ( angleFactor * store ( mappedRadius ) );
 x = store ( ( radialPower * sinOmega ) * sinA );
 y = store ( ( radialPower * cosOmega ) * sinA );
 z = store ( ( E ( 1 ) - angleFactorStored ) * radialPower );

}
 return FormulaVec3 ( float ( x + E ( params [ 9 ] ) ) , float ( y + E ( params [ 10 ] ) ) , float ( z + E ( params [ 11 ] ) ) );

}

}}
