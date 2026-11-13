// Generated from Ads/_QuadrayTransform.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7059 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__QUADRAYTRANSFORM = 7059;
 template < > FormulaVec3 execute_formula < FORMULA__QUADRAYTRANSFORM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 ( void ) de;
 float x = p . x , y = p . y , z = p . z;
 const float Cx = params [ 0 ];
 const float Cy = params [ 1 ];
 const float Cz = params [ 2 ];
 float new_x = x + y + z;
 float new_y = z - x - y;
 float new_z = y - x - z;
 params [ FORMULA_STATE_W_SLOT ] = x - y - z;
 params [ WRITE_J1_SLOT ] = Cx + Cy + Cz;
 params [ WRITE_J2_SLOT ] = Cz - Cx - Cy;
 params [ WRITE_J3_SLOT ] = Cy - Cx - Cz;
 params [ WRITE_J4_SLOT ] = Cx - Cy - Cz;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
