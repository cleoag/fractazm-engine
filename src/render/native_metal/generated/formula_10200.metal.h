// Generated from dIFS/ScherkFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10200 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_SCHERKFS = 10200;
 template < > FormulaVec3 execute_formula < FORMULA_SCHERKFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float LogBase = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Saddle_str = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 RotMatrix3 Rot = rotMatrix3FromFloats ( params , 7 );
 float rx = x * Rot . col0 . x + y * Rot . col1 . x + z * Rot . col2 . x;
 float ry = x * Rot . col0 . y + y * Rot . col1 . y + z * Rot . col2 . y;
 float rz = x * Rot . col0 . z + y * Rot . col1 . z + z * Rot . col2 . z;
 float ratio = cos ( rx ) / cos ( ry );
 float log_val = LogBase * log2 ( fabs ( ratio ) );
 float scherk_val = Saddle_str * log_val - rz;
 float max_xyz = max ( fabs ( rx ) , max ( fabs ( ry ) , fabs ( rz ) ) );
 float cube_dist = max_xyz - 1.5f;
 float shape_de = max ( cube_dist , scherk_val );
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 ScherkFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_SCHERKFS > ( p , params , de );

}

}}
