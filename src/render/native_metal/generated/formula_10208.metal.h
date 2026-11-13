// Generated from dIFS/gnarly3IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10208 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GNARLY3IFS = 10208;
 template < > FormulaVec3 execute_formula < FORMULA_GNARLY3IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Beta = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Amplitude = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const int Iters = int ( params [ 7 ] );
 const int Flip_XZ = int ( params [ 8 ] );
 const float Alpha = params [ 9 ];
 const float Step_Y = params [ 10 ];
 const float Step_X = params [ 11 ];
 const float OTrapClrMul = params [ 12 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 float xx , yy;
 float h_axis;
 if ( ( Flip_XZ & 1 ) != 0 ) {
 xx = z;
 yy = y;
 h_axis = x;

}
 else {
 xx = x;
 yy = y;
 h_axis = z;

}
 if ( ( Flip_XZ & 1 ) == 0 && ! ( ( fabs ( Amplitude ) + 0.1f ) * 1.5f + params [ DIFS_ROLD_SLOT ] >= fabs ( z ) ) ) {
 de = Apply_scale_add != 0 ? - z : - z / Scale;
 return Apply_scale_add != 0 ? FormulaVec3 ( x , y , z ) : p;

}
 int iter_count = metal :: max ( 1 , Iters );
 for ( int i = 0;
 i < iter_count;
 i ++ ) {
 float sin_inner1 = sin ( xx * Beta );
 float combined1 = sin ( sin_inner1 + xx );
 float sign_sqrt_xx = xx * sqrt ( fabs ( xx ) );
 float yy_new = yy - Step_Y * sin ( sign_sqrt_xx - Alpha * combined1 );
 float sin_inner2 = sin ( yy * Beta );
 float combined2 = sin ( Alpha * ( sin_inner2 + yy ) );
 float sign_sqrt_yy = yy * sqrt ( fabs ( yy ) );
 float xx_new = xx + Step_X * sin ( sign_sqrt_yy + combined2 );
 xx = yy_new;
 yy = xx_new;

}
 float gnarly_r = sqrt ( xx * xx + yy * yy );
 float ref_other;
 if ( ( Flip_XZ & 1 ) != 0 ) {
 ref_other = z;

}
 else {
 ref_other = x;

}
 float ref_r = sqrt ( y * y + ref_other * ref_other );
 float shape_de_raw = gnarly_r - ref_r;
 float otrap_val = shape_de_raw;
 if ( OTrapClrMul <= 0.0f ) {
 otrap_val = sqrt ( fabs ( shape_de_raw + 2.0f ) );

}
 params [ MAX_FORMULA_PARAMS - 1 ] = fabs ( otrap_val * OTrapClrMul );
 float shape_de = shape_de_raw * Amplitude - h_axis;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return Apply_scale_add != 0 ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 gnarly3IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_GNARLY3IFS > ( p , params , de );

}

}}
