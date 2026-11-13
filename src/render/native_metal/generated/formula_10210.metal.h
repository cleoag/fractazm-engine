// Generated from dIFS/heartKluchIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10210 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HEARTKLUCHIFS = 10210;
 template < > FormulaVec3 execute_formula < FORMULA_HEARTKLUCHIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float OTrapOffset = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const int Apply_scale_add = int ( params [ 5 ] );
 const float OTrapOnIts = params [ 6 ];
 const float r00 = params [ 7 ];
 const float r10 = params [ 8 ];
 const float r20 = params [ 9 ];
 const float r01 = params [ 10 ];
 const float r11 = params [ 11 ];
 const float r21 = params [ 12 ];
 const float r02 = params [ 13 ];
 const float r12 = params [ 14 ];
 const float r22 = params [ 15 ];
 const float K_Y_SHIFT = 1.4f;
 const float K_EPSILON = 0.01f;
 const float K_QUARTER = 0.25f;
 const float K_Y_MUL = - 1.1f;
 const float K_SQRT_MUL = - 0.8f;
 const float K_X_MUL = 6.25f;
 const float K_RADIUS = 1.445f;
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
 float rx = x * r00 + y * r10 + z * r20;
 float ry = x * r01 + y * r11 + z * r21;
 float rz = x * r02 + y * r12 + z * r22;
 x = rx;
 y = ry;
 z = rz;
 float pp = fabs ( y - K_Y_SHIFT );
 float x_sq = x * x;
 float z_sq = z * z;
 float y_sq = y * y;
 float r_sq = x_sq + y_sq + z_sq;
 float r = sqrt ( r_sq );
 float pp4 = pp * pp * pp * pp;
 float denom = pp4 + K_EPSILON;
 float inner_frac = ( K_QUARTER * z_sq ) / denom;
 float inner = inner_frac + x_sq;
 float fourth_root_inner = sqrt ( sqrt ( inner ) );
 float shape_val = K_SQRT_MUL * fourth_root_inner;
 float combined = y * K_Y_MUL + shape_val;
 float abs_combined = fabs ( combined );
 float abs_sq = abs_combined * abs_combined;
 float final_inner = abs_sq + x_sq + K_X_MUL * z_sq;
 float heart_de = sqrt ( final_inner );
 heart_de = heart_de - 1.0f;
 float r_dist = r - K_RADIUS;
 float shape_de = max ( r_dist , heart_de );
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 params [ MAX_FORMULA_PARAMS - 1 ] = fabs ( OTrapOffset + params [ ITER_COUNT_SLOT ] * OTrapOnIts );
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 heartKluchIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HEARTKLUCHIFS > ( p , params , de );

}

}}
