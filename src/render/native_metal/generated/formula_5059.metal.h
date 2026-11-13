// Generated from dIFS/Knots2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5059 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KNOTS2IFS = 5059;
 template < > FormulaVec3 execute_formula < FORMULA_KNOTS2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 FormulaWorkBudget work ( params , "Knots2IFS" );
 const float Radius_1 = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Radius_2 = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const int Order = int ( params [ 7 ] );
 const int Mobius_X = int ( params [ 8 ] );
 const int Mobius_Y = int ( params [ 9 ] );
 const float Radius_3 = params [ 10 ];
 const int Change_this_to_close = int ( params [ 11 ] );
 const float two_pi = params [ 13 ];
 const float huge_min = params [ 14 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 float theta = atan2 ( y , x );
 float r_xy = sqrt ( x * x + y * y );
 float min_dist = huge_min;
 float theta_shifted = theta;
 int count = Change_this_to_close;
 for (;
;
 ) {
 if ( ! work . beforeStep ( ) ) return NAN;
 theta_shifted += two_pi;
 float t = theta_shifted / float ( Order );
 float dx = r_xy + Radius_3 * cos ( float ( Mobius_X ) * t ) - Radius_1;
 float dz_val = z + Radius_3 * sin ( float ( Mobius_Y ) * t ) - Radius_1;
 float dist = sqrt ( dx * dx + dz_val * dz_val );
 if ( ! ( dist >= min_dist ) ) min_dist = dist;
 if ( count <= 1 ) break;
 -- count;

}
 float shape_de = min_dist - Radius_2;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 Knots2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_KNOTS2IFS > ( p , params , de );

}

}}
