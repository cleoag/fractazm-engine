// Generated from 3Da/ASurfSmoothFold.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10183 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ASURFSMOOTHFOLD = 10183;
 static float asurf_fast_int_pow ( float base , unsigned int exp ) {
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
 static float asurf_smooth_box_fold ( float v , float Fold , int sharpness , float fix ) {
 float av = abs ( v );
 float xp = fix * asurf_fast_int_pow ( av , sharpness );
 float result = ( av + ( 2.0f * Fold - av ) * xp ) / ( xp + 1.0f );
 return metal :: signbit ( v ) ? - result : result;

}
 template < > FormulaVec3 execute_formula < FORMULA_ASURFSMOOTHFOLD > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float ScaleOverMinR2_unused = params [ 1 ];
 const float minR2 = params [ 2 ];
 const float Fold = params [ 3 ];
 const float Scale_vary = params [ 4 ];
 const int Sharpness_box = int ( params [ 5 ] );
 const float Fix_box = params [ 6 ];
 const int Sharpness_ball = int ( params [ 7 ] );
 const float Fix_ball = params [ 8 ];
 RotMatrix3 RM = rotMatrix3FromFloats ( params , 9 );
 const float R00 = RM . col0 . x , R01 = RM . col1 . x , R02 = RM . col2 . x;
 const float R10 = RM . col0 . y , R11 = RM . col1 . y , R12 = RM . col2 . y;
 const float R20 = RM . col0 . z , R21 = RM . col1 . z , R22 = RM . col2 . z;
 const float const_0_99 = 0.99f;
 float prevVaryScale = params [ DIFS_VARYSCALE_SLOT ];
 if ( native_formula_counter ( params ) <= 0.0f ) {
 prevVaryScale = Scale;
 native_increment_formula_counter ( params );

}
 float VaryScale = prevVaryScale + Scale_vary * ( abs ( prevVaryScale ) - 1.0f );
 params [ DIFS_VARYSCALE_SLOT ] = VaryScale;
 float x = asurf_smooth_box_fold ( p . x , Fold , Sharpness_box , Fix_box );
 float y = asurf_smooth_box_fold ( p . y , Fold , Sharpness_box , Fix_box );
 float z = p . z;
 float r_eff;
 if ( minR2 >= const_0_99 ) {
 r_eff = 1.0f;

}
 else {
 float rr = x * x + y * y + z * z;
 float m_half = ( 1.0f + minR2 ) * 0.5f;
 float n_half = ( 1.0f - minR2 ) * 0.5f;
 float r1_raw = rr - m_half;
 float rs = ( r1_raw < 0.0f ) ? - 1.0f : 1.0f;
 float r1_abs_norm = abs ( r1_raw ) / n_half;
 float rp;
 if ( Sharpness_ball & 1 ) {
 rp = asurf_fast_int_pow ( sqrt ( r1_abs_norm ) , Sharpness_ball ) * Fix_ball;

}
 else {
 rp = asurf_fast_int_pow ( r1_abs_norm , static_cast < unsigned int > ( Sharpness_ball ) >> 1 ) * Fix_ball;

}
 float ratio = ( r1_abs_norm + rp ) / ( 1.0f + rp );
 r_eff = m_half + rs * ratio * n_half;

}
 float m_final = VaryScale / r_eff;
 de = de * m_final;
 float sx = x * m_final;
 float sy = y * m_final;
 float sz = z * m_final;
 float z_rot = sz * R00 + sy * R01 + sx * R02;
 float y_rot = sz * R10 + sy * R11 + sx * R12;
 float x_rot = sz * R20 + sy * R21 + sx * R22;
 FormulaVec3 result;
 result . z = z_rot;
 result . y = y_rot;
 result . x = x_rot;
 return result;

}
 FormulaVec3 ASurfSmoothFold_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ASURFSMOOTHFOLD > ( p , params , de );

}

}}
