// Generated from 3D/BT1Pinehedron.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10267 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BT1PINEHEDRON = 10267;
 template < > FormulaVec3 execute_formula < FORMULA_BT1PINEHEDRON > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float scale = params [ 0 ] , offset = params [ 1 ];
 const float s23 = params [ 13 ] , s13 = params [ 14 ] , s12 = params [ 15 ];
 const unsigned countByte = static_cast < unsigned > ( static_cast < int > ( params [ 8 ] ) ) & 255u;
 const unsigned count = countByte ? countByte : 256u;
 const unsigned iteration = static_cast < unsigned > ( static_cast < int > ( params [ ITER_COUNT_SLOT ] ) ) & 255u;
 struct NativeClosure0 {
 thread decltype ( params ) & params;
 auto operator ( ) ( int index ) const {
 return static_cast < unsigned > ( static_cast < int > ( params [ index ] ) ) & 255u;

}

}
;
 auto lowByte = NativeClosure0 {
 params
}
;
 struct NativeClosure1 {
 auto operator ( ) ( unsigned value ) const {
 return value < 128u ? int ( value ) : int ( value ) - 256;

}

}
;
 auto signedByte = NativeClosure1 {

}
;
 float x = p . x , y = p . y , z = p . z;
 for ( unsigned i = 0;
 i < count;
 ++ i ) {
 external_pine_t1 ( x , y , z , s23 , s13 , s12 , scale , offset );

}
 if ( iteration == lowByte ( 6 ) || signedByte ( iteration ) >= signedByte ( lowByte ( 7 ) ) ) {
 const float c = cos ( params [ 5 ] ) , s = sin ( params [ 5 ] );
 const float nz = z * s + y * c;
 y = y * s - z * c;
 z = nz;

}
 if ( iteration == lowByte ( 9 ) ) {
 float ratio = ( float ) y * y / ( ( float ) z * z );
 if ( ratio >= 1 ) ratio = 1 / ratio;
 const float f1 = 1 / metal :: sqrt ( 1 + ratio );
 const float fy = y * f1 , fz = z * f1;
 const float fy2 = fy * fy;
 ratio = ( fz * fz + fy2 ) / ( ( float ) x * x );
 if ( ratio >= 1 ) ratio = 1 / ratio;
 const float f2 = 1 / metal :: sqrt ( 1 + ratio );
 x *= f2;
 y = float ( fy ) * f2;
 z = float ( fz ) * f2;

}
 const float xx = x * x , yy = y * y , zz = z * z;
 const float t = 2 * x / sqrt ( yy + zz );
 return FormulaVec3 ( xx - yy - zz + params [ 17 ] * params [ 2 ] , 2 * t * z * y + params [ 18 ] * params [ 4 ] , t * ( yy - zz ) + params [ 19 ] * params [ 3 ] );

}

}}
