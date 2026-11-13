// Generated from 3D/TgladTetra.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10269 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TGLADTETRA = 10269;
 template < > FormulaVec3 execute_formula < FORMULA_TGLADTETRA > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
#pragma clang fp contract(off)
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
 const int outerCount = native_int32_parameter ( params , 0 ) & 7 , innerCount = native_int32_parameter ( params , 1 ) & 7;
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
 x = store ( ( magnitude + magnitude ) * omegaStored );
 y = store ( magnitude * ( E ( 1 ) - omegaSquareStored ) );
 for ( int inner = 0;
 inner < innerIterations;
 ++ inner ) {
 const E s = x * ( x * x ) + ( ( y * y ) * E ( params [ 3 ] ) ) * x;
 if ( s >= limit ) return FormulaVec3 ( params [ 8 ] , params [ 8 ] , params [ 8 ] );
 const E ss = store ( s ) , t = y * ( y * y ) + ( ( x * x ) * E ( params [ 3 ] ) ) * y;
 const E b = store ( t * E ( params [ 6 ] ) ) , ts = store ( t ) , tSquare = store ( ts * ts );
 const E sk = ss + E ( params [ 5 ] );
 const E a = store ( sk * ( E ( params [ 4 ] ) - ss ) - tSquare );
 const E c = store ( - E ( params [ 5 ] ) / ( sk * sk + tSquare ) );
 const E nextX = store ( ( a * x + - ( b * y ) ) * c );
 y = store ( ( a * y + b * x ) * c );
 x = nextX;
 radialPower = store ( radialPower * radialPower );

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
 z = store ( ( angleFactorStored - E ( 1 ) ) * radialPower );

}
 return FormulaVec3 ( float ( x + E ( params [ 9 ] ) ) , float ( y + E ( params [ 10 ] ) ) , float ( z + E ( params [ 11 ] ) ) );

}

}}
