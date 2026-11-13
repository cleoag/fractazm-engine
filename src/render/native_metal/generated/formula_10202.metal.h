// Generated from dIFS/hextgrid2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10202 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HEXTGRID2IFS = 10202;
 template < > FormulaVec3 execute_formula < FORMULA_HEXTGRID2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float RadiusMod = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Grid_style = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float const_neg2 = params [ 16 ];
 const float const_half = params [ 17 ];
 const float cos30 = params [ 18 ];
 const float const_neg3 = params [ 19 ];
 const float const_inv2s3 = params [ 20 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 const FormulaVec3 transformed ( x , y , z );
 float rx = params [ 7 ] * x + params [ 8 ] * y + params [ 9 ] * z;
 float ry = params [ 10 ] * x + params [ 11 ] * y + params [ 12 ] * z;
 float rz = params [ 13 ] * x + params [ 14 ] * y + params [ 15 ] * z;
 float val_a = fabs ( ry - 2.0f * x87_frndint ( ry * 0.5f ) );
 float val_b = fabs ( rx * cos30 + x87_frndint ( rx * const_inv2s3 ) * const_neg3 ) + val_a * const_half;
 float grid_max = max ( val_a , val_b );
 float grid_val = grid_max - Grid_style;
 float final_grid = min ( val_a , grid_val );
 float shape_de = sqrt ( final_grid * final_grid + rz * rz ) - RadiusMod;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return Apply_scale_add != 0 ? transformed : p;

}
 FormulaVec3 hextgrid2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HEXTGRID2IFS > ( p , params , de );

}

}}
