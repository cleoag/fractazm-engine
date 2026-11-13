// Generated from 3Da/Amazing_Surf_2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10186 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_AMAZING_SURF_2 = 10186;
 static float asm_infinitized_fold ( float v , int fiddler , int smooth_bit , float const0 , float const1 , float const_upper , float const_lower , float smooth_val ) {
 constexpr ulong exponent_sign = 0xfff0000000000000UL;
 constexpr ulong fraction = 0x000fffffffffffffUL;
 constexpr ulong sign = 0x8000000000000000UL;
 constexpr ulong one = 0x3ff0000000000000UL;
 ulong bits;
 native_std :: memcpy ( & bits , & v , sizeof ( bits ) );
 ulong scale_bits = bits & exponent_sign;
 if ( ( const_upper >= v ) && ( const_lower < v ) ) {
 if ( fiddler & smooth_bit ) {
 v = v + smooth_val * ( v / fabs ( v ) );
 native_std :: memcpy ( & bits , & v , sizeof ( bits ) );
 scale_bits = bits & exponent_sign;

}
 bits = ( bits & fraction ) ^ one;
 native_std :: memcpy ( & v , & bits , sizeof ( v ) );

}
 else {
 scale_bits = ( scale_bits & sign ) ^ one;

}
 const float folded = - fabs ( fabs ( v ) + const0 ) + const1;
 float scale;
 native_std :: memcpy ( & scale , & scale_bits , sizeof ( scale ) );
 float result = scale * folded;
 native_std :: memcpy ( & bits , & result , sizeof ( bits ) );
 bits ^= ( scale_bits & 0x0010000000000000UL ) << 11;
 native_std :: memcpy ( & result , & bits , sizeof ( result ) );
 return result;

}
 template < > FormulaVec3 execute_formula < FORMULA_AMAZING_SURF_2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 const E fold ( params [ 3 ] );
 const int fiddler = int ( params [ 13 ] );
 E scaledX = fold * E ( p . x ) , scaledY = fold * E ( p . y );
 if ( fiddler & 1 ) scaledX = abs ( scaledX );
 if ( fiddler & 2 ) scaledY = abs ( scaledY );
 struct NativeClosure0 {
 thread decltype ( fiddler ) & fiddler;
 thread decltype ( params ) & params;
 auto operator ( ) ( float v , int smoothBit ) const {
 constexpr ulong exponentSign = 0xfff0000000000000UL;
 constexpr ulong fraction = 0x000fffffffffffffUL;
 constexpr ulong sign = 0x8000000000000000UL;
 constexpr ulong one = 0x3ff0000000000000UL;
 ulong bits = native_std :: bit_cast < ulong > ( v ) , scaleBits = bits & exponentSign;
 if ( E ( params [ 16 ] ) >= E ( v ) && ! ( E ( params [ 17 ] ) >= E ( v ) ) ) {
 if ( fiddler & smoothBit ) {
 const E value ( v );
 v = static_cast < float > ( value + ( E ( params [ 18 ] ) * value ) / abs ( value ) );
 bits = native_std :: bit_cast < ulong > ( v );
 scaleBits = bits & exponentSign;

}
 bits = ( bits & fraction ) ^ one;
 v = native_std :: bit_cast < float > ( bits );

}
 else {
 scaleBits = ( scaleBits & sign ) ^ one;

}
 const float folded = static_cast < float > ( - abs ( abs ( E ( v ) ) + E ( params [ 14 ] ) ) + E ( params [ 15 ] ) );
 const float result = static_cast < float > ( E ( native_std :: bit_cast < float > ( scaleBits ) ) * E ( folded ) );
 bits = native_std :: bit_cast < ulong > ( result );
 bits ^= ( scaleBits & 0x0010000000000000UL ) << 11;
 return native_std :: bit_cast < float > ( bits );

}

}
;
 auto infinitized = NativeClosure0 {
 fiddler , params
}
;
 const E x = E ( infinitized ( static_cast < float > ( scaledX ) , 4 ) ) / fold;
 const E y = E ( infinitized ( static_cast < float > ( scaledY ) , 8 ) ) / fold;
 const E z ( p . z );
 E matrix [ 9 ];
 for ( int i = 0;
 i < 9;
 ++ i ) matrix [ i ] = E ( static_cast < float > ( params [ 4 + i ] ) );
 const E rz = ( z * matrix [ 0 ] + y * matrix [ 1 ] ) + x * matrix [ 2 ];
 const E ry = ( z * matrix [ 3 ] + y * matrix [ 4 ] ) + x * matrix [ 5 ];
 const E rx = ( z * matrix [ 6 ] + y * matrix [ 7 ] ) + x * matrix [ 8 ];
 const E rr = ( rz * rz + ry * ry ) + rx * rx;
 const E m = native_box_inner_unordered ( float ( rr ) , params ) ? E ( native_aboxmod1_inner_scale ( params ) ) : ( E ( 1 ) >= rr ? E ( params [ 0 ] ) / rr : E ( params [ 0 ] ) );
 de = static_cast < float > ( E ( de ) * m );
 return FormulaVec3 ( static_cast < float > ( rx * m + E ( params [ 19 ] ) ) , static_cast < float > ( ry * m + E ( params [ 20 ] ) ) , static_cast < float > ( rz * m + E ( params [ 21 ] ) ) );

}
 FormulaVec3 Amazing_Surf_2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_AMAZING_SURF_2 > ( p , params , de );

}

}}
