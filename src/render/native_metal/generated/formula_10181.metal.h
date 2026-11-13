// Generated from 3Da/ABoxSmoothFold.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10181 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ABOXSMOOTHFOLD = 10181;
 static float fast_int_pow ( float base , unsigned int exp ) {
 if ( exp <= 1u ) return base;
 float result = 1.0f;
 float b = base;
 while ( exp > 0u ) {
 if ( exp & 1u ) result *= b;
 exp >>= 1;
 if ( exp > 0u ) b *= b;

}
 return result;

}
 static float smooth_box_fold ( float v , float Fold , int sharpness , float fix ) {
 float av = abs ( v );
 float xp = fix * fast_int_pow ( av , sharpness );
 float result = ( av + ( 2.0f * Fold - av ) * xp ) / ( xp + 1.0f );
 return metal :: signbit ( v ) ? - result : result;

}
 template < > FormulaVec3 execute_formula < FORMULA_ABOXSMOOTHFOLD > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: ExtendedScope scope;
 using E = numeric :: Extended80;
 struct NativeClosure0 {
 auto operator ( ) ( E base , uint exponent ) const {
 if ( exponent <= 1 ) return base;
 E result ( 1 );
 while ( exponent ) {
 if ( exponent & 1 ) result *= base;
 exponent >>= 1;
 if ( exponent ) base *= base;

}
 return result;

}

}
;
 const auto power = NativeClosure0 {

}
;
 struct NativeClosure1 {
 thread decltype ( params ) & params;
 thread decltype ( power ) & power;
 auto operator ( ) ( float coordinate ) const {
 const E a = abs ( E ( coordinate ) );
 const E xp = power ( a , uint ( int ( params [ 5 ] ) ) ) * E ( params [ 6 ] );
 const E result = ( a + ( E ( params [ 3 ] ) + E ( params [ 3 ] ) - a ) * xp ) / ( xp + E ( 1 ) );
 return metal :: signbit ( coordinate ) ? - result : result;

}

}
;
 const auto fold = NativeClosure1 {
 params , power
}
;
 const E x = fold ( p . x ) , y = fold ( p . y ) , z = fold ( p . z );
 float previous = params [ DIFS_VARYSCALE_SLOT ];
 if ( native_formula_counter ( params ) <= 0 ) {
 previous = params [ 0 ];
 native_increment_formula_counter ( params );

}
 const float scale = float ( E ( previous ) + ( abs ( E ( previous ) ) - E ( 1 ) ) * E ( params [ 4 ] ) );
 params [ DIFS_VARYSCALE_SLOT ] = scale;
 E radius ( 1 );
 if ( ! ( params [ 2 ] >= params [ 9 ] ) ) {
 const E middle = ( E ( 1 ) + E ( params [ 2 ] ) ) * E ( 0.5f );
 const E halfWidth = ( E ( 1 ) - E ( params [ 2 ] ) ) * E ( 0.5f );
 const E delta = ( x * x + y * y + z * z ) - middle;
 const E normalized = abs ( delta ) / halfWidth;
 const uint sharpness = uint ( int ( params [ 7 ] ) );
 const E rp = power ( ( sharpness & 1 ) ? sqrt ( normalized ) : normalized , ( sharpness & 1 ) ? sharpness : sharpness / 2 ) * E ( params [ 8 ] );
 E offset = ( ( normalized + rp ) / ( E ( 1 ) + rp ) ) * halfWidth;
 if ( ! ( delta >= E ( 0 ) ) ) offset = - offset;
 radius = offset + middle;

}
 const E multiplier = E ( scale ) / radius;
 de = float ( E ( de ) * multiplier );
 return FormulaVec3 ( float ( x * multiplier + E ( params [ 10 ] ) ) , float ( y * multiplier + E ( params [ 11 ] ) ) , float ( z * multiplier + E ( params [ 12 ] ) ) );

}
 FormulaVec3 ABoxSmoothFold_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ABOXSMOOTHFOLD > ( p , params , de );

}

}}
