// Generated from 4D/_coordvspow.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10336 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_COORDVSPOW = 10336;
 template < > FormulaVec3 execute_formula < FORMULA_COORDVSPOW > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E x = p . x , y = p . y , z = p . z , scale = params [ 7 ];
 const int powerPct = int ( params [ 0 ] ) , cylPct = int ( params [ 2 ] ) , sphPct = int ( params [ 3 ] );
 const int icylPct = int ( params [ 4 ] ) , isphPct = int ( params [ 5 ] ) , expPct = int ( params [ 6 ] );
 const E power = params [ 1 ];
 struct NativeClosure0 {
 auto operator ( ) ( E v ) const {
 const unsigned exponent = numeric :: signExponent ( v ) & 0x7fff;
 return exponent >= 16383 + 63 && exponent != 0x7fff;

}

}
;
 const auto outsideTrigRange = NativeClosure0 {

}
;
 struct NativeClosure1 {
 thread decltype ( outsideTrigRange ) & outsideTrigRange;
 auto operator ( ) ( E v ) const {
 return outsideTrigRange ( v ) ? v : extendedSinCos ( v ) . sin;

}

}
;
 const auto scalarSin = NativeClosure1 {
 outsideTrigRange
}
;
 struct NativeClosure2 {
 thread decltype ( outsideTrigRange ) & outsideTrigRange;
 auto operator ( ) ( E v ) const {
 return outsideTrigRange ( v ) ? v : extendedSinCos ( v ) . cos;

}

}
;
 const auto scalarCos = NativeClosure2 {
 outsideTrigRange
}
;
 struct NativeClosure3 {
 thread decltype ( scale ) & scale;
 auto operator ( ) ( E value , int weight , E previous ) const {
 return E ( float ( ( value * E ( weight ) ) * scale + previous ) );

}

}
;
 const auto accumulate = NativeClosure3 {
 scale
}
;
 E rx ( 0 ) , ry ( 0 ) , rz ( 0 );
 if ( powerPct != 0 ) {
 if ( power == E ( 1 ) || power == E ( 0 ) ) {
 rx = float ( ( x * E ( powerPct ) ) * scale );
 ry = float ( ( y * E ( powerPct ) ) * scale );
 rz = float ( ( z * E ( powerPct ) ) * scale );

}
 else {
 const E radius = native_formula_length3 ( x , y , z );
 const E savedRadius = float ( radius );
 const E radiusPower = float ( extendedExp2 ( power * extendedLog2 ( abs ( radius ) ) , false ) );
 const E theta = extendedAtan2 ( y , x ) * power;
 const E zr = z / savedRadius;
 const E polarNumerator = sqrt ( - ( zr * zr ) + E ( 1 ) ) + E ( 1 );
 const E halfPhi = extendedAtan2Wide ( E ( 1 ) , zr / polarNumerator );
 const E phi = ( halfPhi + halfPhi ) * power;
 const auto polar = extendedSinCos ( phi ) , azimuth = extendedSinCos ( theta );
 const bool missingPolarPush = outsideTrigRange ( phi );
 const E pc = float ( polar . cos );
 const E ps = float ( missingPolarPush ? theta : polar . sin );
 const E ac = float ( missingPolarPush ? extendedNaN ( ) : azimuth . cos );
 const E as = float ( missingPolarPush ? extendedNaN ( ) : azimuth . sin );
 rx = float ( ( ( ( radiusPower * ac ) * pc ) * E ( powerPct ) ) * scale );
 ry = float ( ( ( ( radiusPower * as ) * pc ) * E ( powerPct ) ) * scale );
 rz = float ( ( ( radiusPower * ps ) * E ( powerPct ) ) * scale );

}

}
 if ( cylPct != 0 ) {
 const E radius = sqrt ( x * x + y * y );
 ry = accumulate ( extendedAtan2 ( y , x ) , cylPct , ry );
 rx = accumulate ( radius , cylPct , rx );

}
 if ( sphPct != 0 ) {
 const E square = x * x + y * y;
 const E radius = sqrt ( z * z + square );
 const E polar = extendedAtan2 ( sqrt ( square ) , z );
 rz = accumulate ( extendedAtan2 ( y , x ) , sphPct , rz );
 ry = accumulate ( polar , sphPct , ry );
 rx = accumulate ( radius , sphPct , rx );

}
 if ( icylPct != 0 ) {
 const E cosine = x * scalarCos ( y ) , sine = x * scalarSin ( y );
 ry = accumulate ( sine , icylPct , ry );
 rx = accumulate ( cosine , icylPct , rx );

}
 if ( isphPct != 0 ) {
 const auto polar = extendedSinCos ( y ) , azimuth = extendedSinCos ( z );
 const bool missingY = outsideTrigRange ( y ) , missingZ = outsideTrigRange ( z );
 const E zLane = missingY ? ( missingZ ? extendedNaN ( ) : y ) : ( missingZ ? polar . sin : polar . cos );
 rz = accumulate ( zLane * x , isphPct , rz );
 if ( missingY || missingZ ) {
 ry = accumulate ( extendedNaN ( ) , isphPct , ry );
 rx = accumulate ( extendedNaN ( ) , isphPct , rx );

}
 else {
 const E xLane = ( azimuth . cos * polar . sin ) * x;
 const E yLane = azimuth . sin * ( polar . sin * x );
 ry = accumulate ( yLane , isphPct , ry );
 rx = accumulate ( xLane , isphPct , rx );

}

}
 if ( expPct != 0 ) {
 const E log2e = numeric :: fromBits ( 0x3fff , 0xb8aa3b295c17f0bcULL );
 const E radius = sqrt ( x * x + y * y ) , ratio = z / radius;
 const E savedRatio = float ( ratio );
 const E exponential = float ( extendedExp2 ( ( - ( ratio * y ) + x ) * log2e , false ) * params [ 8 ] );
 const auto first = extendedSinCos ( savedRatio * x + y );
 const auto second = extendedSinCos ( savedRatio * x - y );
 const E c1 = float ( first . cos ) , s1 = float ( first . sin );
 const E c2 = float ( second . cos ) , s2 = float ( second . sin );
 const E product = savedRatio * y;
 const E twice = float ( extendedExp2 ( ( product + product ) * log2e , false ) );
 rx = accumulate ( ( twice * c2 + c1 ) * exponential , expPct , rx );
 ry = accumulate ( ( - ( twice * s2 ) + s1 ) * exponential , expPct , ry );
 rz = accumulate ( extendedExp2 ( radius * log2e , false ) * scalarSin ( z ) , expPct , rz );

}
 return FormulaVec3 ( float ( rx ) , float ( ry ) , float ( rz ) );

}

}}
