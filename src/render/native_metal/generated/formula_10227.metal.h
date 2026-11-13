// Generated from dIFS/tritgrid2IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10227 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TRITGRID2IFS = 10227;
 template < > FormulaVec3 execute_formula < FORMULA_TRITGRID2IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float RadiusMod = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Grid_styling = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float c_half = params [ 16 ];
 const float c_s32 = params [ 17 ];
 const float c_one = params [ 18 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 const FormulaVec3 committed = Apply_scale_add ? FormulaVec3 ( x , y , z ) : p;
 RotMatrix3 Rot = rotMatrix3FromFloats ( params , 7 );
 float rx = x * Rot . col0 . x + y * Rot . col1 . x + z * Rot . col2 . x;
 float ry = x * Rot . col0 . y + y * Rot . col1 . y + z * Rot . col2 . y;
 float rz = x * Rot . col0 . z + y * Rot . col1 . z + z * Rot . col2 . z;
 x = rx;
 y = ry;
 z = rz;
 float v1 = Grid_styling - x;
 int iv1 = x87_fistp_int32 ( v1 );
 int iv1_adj = x87_fistp_int32 ( v1 + c_half );
 float frac1 = fabs ( v1 - float ( iv1 ) );
 float v2 = x * c_half + y * c_s32;
 int iv2 = x87_fistp_int32 ( v2 );
 int iv2_adj = x87_fistp_int32 ( v2 + c_half );
 float frac2 = fabs ( v2 - float ( iv2 ) );
 float v3 = y * c_s32 - x * c_half;
 int iv3 = x87_fistp_int32 ( v3 );
 int iv3_adj = x87_fistp_int32 ( v3 + c_half );
 float frac3 = fabs ( v3 - float ( iv3 ) );
 if ( ( ( static_cast < uint > ( iv3_adj ) + 0x1000u ) % 3u ) == 0 ) {
 frac1 = c_one;

}
 if ( ( ( static_cast < uint > ( iv1_adj ) + 0x1000u ) % 3u ) == 0 ) {
 frac2 = c_one;

}
 if ( ( ( static_cast < uint > ( iv2_adj ) + 0x1000u ) % 3u ) == 0 ) {
 frac3 = c_one;

}
 float d_min = ! ( frac3 >= frac2 ) ? frac3 : frac2;
 d_min = ! ( d_min >= frac1 ) ? d_min : frac1;
 float shape_de = sqrt ( d_min * d_min + z * z ) - RadiusMod;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return committed;

}
 FormulaVec3 tritgrid2IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TRITGRID2IFS > ( p , params , de );

}

}}
