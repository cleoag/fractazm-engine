// Generated from dIFS/gnarly2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10089 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_GNARLY2IFS = 10089;
 template < > FormulaVec3 execute_formula < FORMULA_GNARLY2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
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
 const float Step = params [ 10 ];
 const float Gamma = params [ 11 ];
 const float OTrapClrMul = params [ 12 ];
 const float const_1_5 = 1.5f;
 const float const_0_1 = 0.1f;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 float xx , yy;
 float ref_axis;
 if ( ( Flip_XZ & 1 ) != 0 ) {
 xx = z;
 yy = y;
 ref_axis = x;

}
 else {
 xx = x;
 yy = y;
 ref_axis = z;

}
 if ( ( Flip_XZ & 1 ) == 0 && ! ( ( fabs ( Amplitude ) + 0.1f ) * 1.5f + params [ DIFS_ROLD_SLOT ] >= fabs ( z ) ) ) {
 de = Apply_scale_add != 0 ? - z : - z / Scale;
 return Apply_scale_add != 0 ? FormulaVec3 ( x , y , z ) : p;

}
 int iter_count = metal :: max ( 1 , Iters );
 for ( int i = 0;
 i < iter_count;
 i ++ ) {
 float s1 = sin ( Alpha * ( xx + Beta * cos ( xx ) ) );
 float yy_new = yy + Step * sin ( Gamma * ( xx - yy ) + s1 );
 float s2 = sin ( Alpha * ( yy + Beta * cos ( yy ) ) );
 float xx_new = xx + Step * sin ( Gamma * ( xx + yy ) + s2 );
 xx = xx_new;
 yy = yy_new;

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
 float otrap_val;
 if ( OTrapClrMul > 0.0f ) {
 otrap_val = fabs ( shape_de_raw * OTrapClrMul );

}
 else {
 otrap_val = fabs ( sqrt ( fabs ( shape_de_raw + 2.0f ) ) * OTrapClrMul );

}
 params [ MAX_FORMULA_PARAMS - 1 ] = otrap_val;
 float shape_de = shape_de_raw * Amplitude - ref_axis;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return Apply_scale_add != 0 ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 gnarly2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_GNARLY2IFS > ( p , params , de );

}

}}
