// Generated from dIFS/loxodromeIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10211 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_LOXODROMEIFS = 10211;
 template < > FormulaVec3 execute_formula < FORMULA_LOXODROMEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Angle = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float SpiralSpeed = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float RepeatAngle = params [ 7 ];
 const float AtanAdd = params [ 8 ];
 const float Thickness = params [ 9 ];
 const float OTrapOnIts = params [ 10 ];
 const float OTrapOffset = params [ 11 ];
 const float OTrapSides = params [ 12 ];
 const float K_360_OVER_PI = 114.591559f;
 const float K_PI_OVER_360 = 0.008726646f;
 const float K_180 = 180.0f;
 const float K_NEG01 = - 0.1f;
 const float K_ALMOST1 = 0.999999999f;
 const float K_ONE = 1.0f;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 float abs_z = fabs ( z );
 int sides_count = 0;
 float spiral_de;
 if ( abs_z > K_ALMOST1 ) {
 spiral_de = K_NEG01;

}
 else {
 float one_minus_absz = 1.0f - abs_z;
 float log_val = SpiralSpeed * log2 ( one_minus_absz );
 if ( z < 0.0f ) {
 log_val = - log_val;

}
 float angle_input = log_val + AtanAdd;
 float sc = sin ( angle_input );
 float cc = cos ( angle_input );
 float term1 = x * sc - y * cc;
 float term2 = x * cc + y * sc;
 float angle = atan2 ( term1 , term2 );
 angle *= K_360_OVER_PI;
 float sector = angle / RepeatAngle;
 const float rounded_sector = x87_frndint ( sector );
 sides_count = metal :: isfinite ( rounded_sector ) && rounded_sector >= - 2147483648.0f && rounded_sector <= 2147483647.0f ? int ( rounded_sector ) : native_std :: numeric_limits < int > :: min ( );
 float frac = sector - float ( sides_count );
 float frac_angle = frac * RepeatAngle;
 float abs_frac = fabs ( frac_angle );
 float tooth = abs_frac / Angle - 1.0f;
 tooth *= Angle;
 tooth *= K_PI_OVER_360;
 spiral_de = - tooth;

}
 float r = native_formula_length3 ( x , y , z );
 float r_dist = r - K_ONE;
 float sphere_de = fabs ( r_dist + Thickness ) - Thickness;
 float shape_de = sphere_de >= spiral_de ? sphere_de : spiral_de;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 params [ 63 ] = float ( fabs ( ( static_cast < float > ( params [ ITER_COUNT_SLOT ] ) * OTrapOnIts + OTrapOffset ) + static_cast < float > ( sides_count ) * OTrapSides ) );
 return Apply_scale_add ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 loxodromeIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_LOXODROMEIFS > ( p , params , de );

}

}}
