// Generated from 3D/Makin-NSR-tri.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7099 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAKIN_NSR_TRI = 7099;
 template < > FormulaVec3 execute_formula < FORMULA_MAKIN_NSR_TRI > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Power = params [ 0 ];
 const float zScale = params [ 1 ];
 float x = p . x , y = p . y , z = p . z;
 if ( x == 0.0f && y == 0.0f && z == 0.0f ) {
 return FormulaVec3 ( NAN , NAN , NAN );

}
 if ( ! isfinite ( x ) || ! isfinite ( y ) || ! isfinite ( z ) ) {
 return FormulaVec3 ( NAN , NAN , NAN );

}
 float phi_yx = Power * atan2 ( y , x );
 float cos_yx = cos ( phi_yx );
 float sin_yx = sin ( phi_yx );
 float phi_zy = Power * atan2 ( z , y );
 float cos_zy = cos ( phi_zy );
 float sin_zy = sin ( phi_zy );
 float phi_xz = Power * atan2 ( x , z );
 float cos_xz = cos ( phi_xz );
 float sin_xz = sin ( phi_xz );
 float x_dir = cos_yx * sin_xz;
 float y_dir = sin_yx * cos_zy;
 float z_dir = sin_zy * cos_xz;
 float r = native_formula_length3 ( x , y , z );
 float r_P = pow ( abs ( r ) , Power );
 if ( ! isfinite ( r ) || ! isfinite ( r_P ) ) {
 return FormulaVec3 ( NAN , NAN , NAN );

}
 float r_dir = native_formula_length3 ( x_dir , y_dir , z_dir );
 if ( ! isfinite ( r_dir ) || r_dir == 0.0f ) {
 return FormulaVec3 ( NAN , NAN , NAN );

}
 float scale = ( r_dir > 1e-30f ) ? ( r_P / r_dir ) : 0.0f;
 if ( ! isfinite ( scale ) ) {
 return FormulaVec3 ( NAN , NAN , NAN );

}
 float new_x = x_dir * scale;
 float new_y = y_dir * scale;
 float new_z = z_dir * scale * zScale;
 if ( ! isfinite ( new_x ) || ! isfinite ( new_y ) || ! isfinite ( new_z ) ) {
 return FormulaVec3 ( NAN , NAN , NAN );

}
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
