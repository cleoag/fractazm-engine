// Generated from dIFSa/Apollo3D-IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10316 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_APOLLO3DIFS = 10316;
 template < > FormulaVec3 execute_formula < FORMULA_APOLLO3DIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float AddX1 = params [ 0 ];
 const float AddX2 = params [ 1 ];
 const float MulZ = params [ 2 ];
 RotMatrix3 R = rotMatrix3FromFloats ( params , 3 );
 const int PolyOrder = int ( params [ 12 ] );
 const float GScale = params [ 13 ];
 const float FinalAddX = params [ 14 ];
 const float Volcano = params [ 15 ];
 const float c_inv2pi = 0.159154943f;
 const float c_2pi = 6.283185307f;
 float x = p . x;
 float y = p . y;
 float z = p . z;
 float sx = x * GScale + AddX1;
 float sy = y * GScale;
 float angle = - atan2 ( sy , sx );
 float angle_norm = angle * c_inv2pi;
 float sector_f = round ( angle_norm * float ( PolyOrder ) );
 int sector_i = int ( sector_f );
 float sector_angle = ( sector_f / float ( PolyOrder ) ) * c_2pi;
 float cs = cos ( sector_angle );
 float sn = sin ( sector_angle );
 float rot1 = sy * cs + sx * sn;
 if ( ( sector_i & 1 ) == 0 ) {
 rot1 = - rot1;

}
 float rot2 = sx * cs - sy * sn;
 float a = rot2 + AddX2;
 float b = rot1;
 float c_val = z;
 FormulaVec3 rv = R * FormulaVec3 ( a , b , c_val );
 float ox = rv . x;
 float oy = rv . y;
 float oz = rv . z;
 float xp = - oy;
 float yp = ox;
 float zp = oz;
 float s2_xy = xp * xp + yp * yp;
 float A_val = s2_xy - 1.0f;
 float B_val = s2_xy + 1.0f + 2.0f * yp;
 float new_x_raw = A_val / B_val + FinalAddX;
 float new_y_raw = - 2.0f * xp / B_val;
 float out_x = new_x_raw / GScale;
 float out_y = new_y_raw / GScale;
 float de_factor = sqrt ( new_x_raw * new_x_raw + new_y_raw * new_y_raw + Volcano );
 float scale_factor = MulZ / de_factor;
 de *= fabs ( scale_factor );
 float out_z = zp * scale_factor;
 return FormulaVec3 ( out_x , out_y , out_z );

}
 FormulaVec3 Apollo3DIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_APOLLO3DIFS > ( p , params , de );

}

}}
