// Generated from dIFSa/PolyFoldExIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8765 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_POLYFOLDEXIFS = 8765;
 template < > FormulaVec3 execute_formula < FORMULA_POLYFOLDEXIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const int PolyFold_Order = int ( params [ 12 ] );
 const float Atan2_fixer = params [ 13 ];
 const int PolyFoldZ_Order = int ( params [ 14 ] );
 const float Atan2Z_fixer = params [ 15 ];
 const float INV_2PI = 0.159154943f;
 const float TWO_PI = 6.283185307f;
 float ax = p . x + Add_X;
 float ay = p . y + Add_Y;
 float az = p . z + Add_Z;
 FormulaVec3 rv = R * FormulaVec3 ( ax , ay , az );
 float rot_x = rv . x;
 float rot_y = rv . y;
 float rot_z = rv . z;
 float angle_xy = Atan2_fixer - atan2 ( rot_y , rot_x );
 float n_xy = rint ( angle_xy * INV_2PI * float ( PolyFold_Order ) );
 float sector_xy = n_xy / float ( PolyFold_Order ) * TWO_PI;
 float cs1 = cos ( sector_xy );
 float ss1 = sin ( sector_xy );
 float new_y = rot_y * cs1 + rot_x * ss1;
 float new_x = rot_x * cs1 - rot_y * ss1;
 float new_z = rot_z;
 float angle_xz = Atan2Z_fixer - atan2 ( new_z , new_x );
 float n_xz = rint ( angle_xz * INV_2PI * float ( PolyFoldZ_Order ) );
 float sector_xz = n_xz / float ( PolyFoldZ_Order ) * TWO_PI;
 float cs2 = cos ( sector_xz );
 float ss2 = sin ( sector_xz );
 float out_z = new_z * cs2 + new_x * ss2;
 float out_x = new_x * cs2 - new_z * ss2;
 return FormulaVec3 ( out_x , new_y , out_z );

}
 FormulaVec3 PolyFoldExIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_POLYFOLDEXIFS > ( p , params , de );

}

}}
