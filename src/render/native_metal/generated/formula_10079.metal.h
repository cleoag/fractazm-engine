// Generated from dIFS/PSphereIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10079 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_PSPHEREIFS = 10079;
 template < > FormulaVec3 execute_formula < FORMULA_PSPHEREIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float PseudoRadius = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float MaxRadius = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float Thickness = params [ 7 ];
 const float MinRadius = params [ 8 ];
 const int Flip_XZ_YZ = int ( params [ 9 ] );
 const int Upside_down = int ( params [ 10 ] );
 const float const0 = params [ 11 ];
 const float const1 = params [ 12 ];
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
 if ( Flip_XZ_YZ == 1 ) {
 float tmp = x;
 x = z;
 z = tmp;

}
 if ( Flip_XZ_YZ == - 1 ) {
 float tmp = y;
 y = z;
 z = tmp;

}
 if ( Upside_down != 0 ) {
 z = - z;

}
 float R_sq = x * x + y * y;
 float R = sqrt ( R_sq );
 float a = PseudoRadius;
 float diff = a * a - R_sq;
 float clamped = diff + fabs ( diff );
 float n = sqrt ( clamped );
 float ln_val = log ( ( n + a ) / R );
 float tractrix = a * ln_val - n;
 float tractrix_de = fabs ( tractrix - z ) - Thickness;
 float de_fix = fabs ( R * const0 + const1 );
 float shape_de = tractrix_de * de_fix;
 float clip_lo = R - MaxRadius;
 shape_de = max ( shape_de , clip_lo );
 float clip_hi = MinRadius - R;
 shape_de = max ( shape_de , clip_hi );
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 PSphereIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_PSPHEREIFS > ( p , params , de );

}

}}
