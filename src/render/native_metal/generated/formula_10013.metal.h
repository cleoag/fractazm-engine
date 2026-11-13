// Generated from Ads/_RotateXYonItsP.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10013 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
inline int incrementFormulaCounter ( FormulaParams params ) {
 return native_increment_formula_counter ( params );

}
 inline uint formulaCounterRandomWord ( int count ) {
 const uint low = ( static_cast < uint > ( count ) - 0x3D07u ) & 0xFFFFu;
 return ( low * low ) & 0xFFFFu;

}
 inline FormulaVec3 formulaIterationRotation ( FormulaVec3 p , FormulaParams params , bool powerLaw ) {
 const int count = incrementFormulaCounter ( params );
 float xyz [ ] = {
 p . x , p . y , p . z
}
;
 if ( ! powerLaw ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 for ( int row = 0;
 row < 3;
 ++ row ) {
 thread const float * m = params + row * 3;
 xyz [ row ] = float ( ( E ( p . x ) * float ( m [ 0 ] ) + E ( p . y ) * float ( m [ 1 ] ) ) + E ( p . z ) * float ( m [ 2 ] ) );

}
 const E angle = ( E ( count ) * params [ 9 ] ) * params [ 10 ];
 const auto sc = extendedSinCos ( angle );
 const E x = xyz [ 0 ] , y = xyz [ 1 ];
 const bool missing = ! true && abs ( angle ) >= numeric :: fromBits ( 16383 + 63 , 0x8000000000000000ULL );
 const E rx = missing ? extendedNaN ( ) : E ( float ( sc . cos * x + sc . sin * y ) );
 const E ry = missing ? extendedNaN ( ) : E ( float ( sc . cos * y - sc . sin * x ) );
 const E z = xyz [ 2 ];
 for ( int col = 0;
 col < 3;
 ++ col ) xyz [ col ] = float ( ( rx * float ( params [ col ] ) + ry * float ( params [ 3 + col ] ) ) + z * float ( params [ 6 + col ] ) );
 return FormulaVec3 ( xyz [ 0 ] , xyz [ 1 ] , xyz [ 2 ] );

}
 for ( int row = 0;
 row < 3;
 ++ row ) {
 thread const float * m = params + row * 3;
 xyz [ row ] = float ( ( static_cast < float > ( p . x ) * m [ 0 ] + static_cast < float > ( p . y ) * m [ 1 ] ) + static_cast < float > ( p . z ) * m [ 2 ] );

}
 float angle = count;
 if ( powerLaw ) angle = pow ( fabs ( angle - params [ 10 ] ) , static_cast < float > ( params [ 11 ] ) );
 angle = ( angle * params [ 9 ] ) * params [ powerLaw ? 12 : 10 ];
 const float c = cos ( angle ) , s = sin ( angle );
 const float x = float ( xyz [ 0 ] * c + xyz [ 1 ] * s );
 const float y = float ( xyz [ 1 ] * c - xyz [ 0 ] * s );
 for ( int col = 0;
 col < 3;
 ++ col ) xyz [ col ] = float ( ( static_cast < float > ( x ) * params [ col ] + static_cast < float > ( y ) * params [ 3 + col ] ) + static_cast < float > ( xyz [ 2 ] ) * params [ 6 + col ] );
 return FormulaVec3 ( xyz [ 0 ] , xyz [ 1 ] , xyz [ 2 ] );

}
 constant constexpr unsigned int FORMULA__ROTATEXYONITSP = 10013;
 template < > FormulaVec3 execute_formula < FORMULA__ROTATEXYONITSP > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return formulaIterationRotation ( p , params , true );

}

}}
