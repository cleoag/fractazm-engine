// Generated from 3D/KaliLinCombSSE2sm.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10352 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KALILINCOMBSSE2SM = 10352;
 inline void kaliSquare ( thread float2 & re , thread float2 & im ) {
 const float2 oldIm = im;
 im = im * re;
 re = re * re - oldIm * oldIm;
 im = im + im;

}
 inline void kaliMultiply ( thread float2 & re , thread float2 & im , float2 baseRe , float2 baseIm ) {
 const float2 newIm = im * baseRe + re * baseIm;
 re = re * baseRe - im * baseIm;
 im = newIm;

}
 template < > FormulaVec3 execute_formula < FORMULA_KALILINCOMBSSE2SM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 ( void ) de;
 float2 im ( params [ FORMULA_STATE_X_SLOT ] , params [ FORMULA_STATE_Y_SLOT ] );
 float2 re ( params [ FORMULA_STATE_Z_SLOT ] , params [ FORMULA_STATE_W_SLOT ] );
 if ( native_formula_counter ( params ) == 0.f ) {
 im = float2 ( p . x , p . y );
 re = float2 ( p . z , p . z );
 p . z = 0.f;
 native_formula_counter ( params ) = 1.f;

}
 const float2 baseRe = re , baseIm = im;
 const uint power = as_type < uint > ( native_int32_parameter ( params , 0 ) );
 uint formula_half = power >> 1;
 kaliSquare ( re , im );
 if ( formula_half == 3 ) kaliMultiply ( re , im , baseRe , baseIm );
 formula_half -= 2;
 if ( ( formula_half & 0x80000000u ) == 0 ) kaliSquare ( re , im );
 formula_half -= 2;
 if ( ( formula_half & 0x80000000u ) == 0 ) kaliSquare ( re , im );
 if ( power & 1 ) kaliMultiply ( re , im , baseRe , baseIm );
 im += float2 ( params [ 3 ] , params [ 4 ] );
 re += float2 ( params [ 5 ] , params [ 5 ] );
 params [ FORMULA_STATE_X_SLOT ] = im . x;
 params [ FORMULA_STATE_Y_SLOT ] = im . y;
 params [ FORMULA_STATE_Z_SLOT ] = re . x;
 params [ FORMULA_STATE_W_SLOT ] = re . y;
 const float realSum = re . y + re . x , imagSum = im . y + im . x;
 const float2 mod2 = re * re + im * im;
 const float numerator = ( mod2 . x + mod2 . y ) * params [ 2 ];
 const float denominator = ( imagSum * imagSum + realSum * realSum ) + params [ 1 ];
 const float scale = sqrt ( numerator / denominator );
 return FormulaVec3 ( realSum * scale , imagSum * scale , p . z );

}

}}
