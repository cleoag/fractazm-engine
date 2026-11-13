// Generated from Ads/_DEComb_ISph_Plane.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10259 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_DECOMB_ISPH_PLANE = 10259;
 template < > FormulaVec3 execute_formula < FORMULA_DECOMB_ISPH_PLANE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float inv_x_center = params [ 4 ];
 const float inv_y_center = params [ 5 ];
 const float inv_z_center = params [ 6 ];
 const float radius = params [ 3 ];
 const float move_x = params [ 0 ];
 const float move_y = params [ 1 ];
 const float move_z = params [ 2 ];
 const int iteration = ( int ) native_formula_counter ( params );
 float x = p . x , y = p . y , z = p . z;
 if ( iteration == 0 ) {
 native_formula_counter ( params ) = 1.0f;
 float dx = x - inv_x_center;
 float dy = y - inv_y_center;
 float dz = z - inv_z_center;
 float r2 = dx * dx + dy * dy + dz * dz;
 float scale = radius / r2;
 x = dx * scale - move_x;
 y = dy * scale - move_y;
 z = dz * scale - move_z;

}
 else {
 const float r2 = params [ FORMULA_ROUT_SLOT ];
 float s = sqrt ( r2 );
 x = x * s;
 y = y * s;
 z = z * s;

}
 return FormulaVec3 ( x , y , z );

}

}}
