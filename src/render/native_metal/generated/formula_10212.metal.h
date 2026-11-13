// Generated from dIFS/MitreIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10212 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MITREIFS = 10212;
 template < > FormulaVec3 execute_formula < FORMULA_MITREIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float YZ_add = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float X2R_mul = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float r00 = params [ 7 ];
 const float r10 = params [ 8 ];
 const float r20 = params [ 9 ];
 const float r01 = params [ 10 ];
 const float r11 = params [ 11 ];
 const float r21 = params [ 12 ];
 const float r02 = params [ 13 ];
 const float r12 = params [ 14 ];
 const float r22 = params [ 15 ];
 const float K_SCALE_CONST = params [ 16 ];
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
 float pos_acc = 0.0f;
 float neg_acc = 0.0f;
 float rx = x * r00 + y * r10 + z * r20;
 float ry = x * r01 + y * r11 + z * r21;
 float rz = x * r02 + y * r12 + z * r22;
 x = rx;
 y = ry;
 z = rz;
 float x_sq = x * x;
 float y_sq = y * y;
 float z_sq = z * z;
 float r_sq = x_sq + y_sq + z_sq;
 pos_acc = X2R_mul * r_sq * x * x;
 float bracket = YZ_add - y_sq - z_sq;
 float val = bracket * y * y;
 if ( val < 0.0f ) {
 pos_acc = pos_acc - val;

}
 else {
 neg_acc = neg_acc + val;

}
 float raw_de = sqrt ( sqrt ( pos_acc ) ) - sqrt ( sqrt ( neg_acc ) );
 float r_correction = sqrt ( r_sq ) - YZ_add;
 float clamped = r_correction + fabs ( r_correction );
 clamped = clamped + clamped;
 clamped = clamped + clamped;
 float corrected = raw_de + clamped;
 float shape_de = corrected * K_SCALE_CONST;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 MitreIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_MITREIFS > ( p , params , de );

}

}}
