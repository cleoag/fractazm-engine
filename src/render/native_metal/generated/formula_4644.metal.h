// Generated from dIFSa/PolyFoldIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4644 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_POLYFOLDIFS = 4644;
 template < > FormulaVec3 execute_formula < FORMULA_POLYFOLDIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Add_X = params [ 0 ];
 const float Add_Y = params [ 1 ];
 const float Add_Z = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const int PolyFold_Order = int ( params [ 12 ] );
 const float Atan2_fixer = params [ 13 ];
 const float INV_2PI = 0.159154943f;
 const float TWO_PI = 6.283185307f;
 float ax = p . x + Add_X;
 float ay = p . y + Add_Y;
 float az = p . z + Add_Z;
 FormulaVec3 rv = R * FormulaVec3 ( ax , ay , az );
 float rot_x = rv . x;
 float rot_y = rv . y;
 float rot_z = rv . z;
 float angle = Atan2_fixer - atan2 ( rot_y , rot_x );
 float n_float = rint ( angle * INV_2PI * float ( PolyFold_Order ) );
 float sector = n_float / float ( PolyFold_Order ) * TWO_PI;
 float cs = cos ( sector );
 float ss = sin ( sector );
 float new_x = rot_x * cs - rot_y * ss;
 float new_y = rot_y * cs + rot_x * ss;
 return FormulaVec3 ( new_x , new_y , rot_z );

}
 FormulaVec3 PolyFoldIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_POLYFOLDIFS > ( p , params , de );

}

}}
