// Generated from Ads/_BPolyhedronFromSphere.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7094 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BPOLYHEDRONFROMSPHERE = 7094;
 template < > FormulaVec3 execute_formula < FORMULA__BPOLYHEDRONFROMSPHERE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Order1 = params [ 0 ];
 const float Order2 = params [ 1 ];
 const float PI = 3.14159265358979f;
 float x = p . x , y = p . y , z = p . z;
 float halfA = PI / Order1;
 float doubleA = 2.0f * halfA;
 float halfB = PI / Order2;
 float doubleB = 2.0f * halfB;
 float angle_yz = abs ( atan2 ( z , y ) );
 if ( doubleA > 1e-30f ) {
 angle_yz = fmod ( angle_yz , doubleA );
 if ( angle_yz > halfA ) {
 angle_yz -= doubleA;

}

}
 float cos_a = cos ( angle_yz );
 float proj_y = cos_a * y;
 float proj_z = cos_a * z;
 float r_yz = sqrt ( proj_y * proj_y + proj_z * proj_z );
 float angle_xr = abs ( atan2 ( r_yz , x ) );
 if ( doubleB > 1e-30f ) {
 angle_xr = fmod ( angle_xr , doubleB );
 if ( angle_xr > halfB ) {
 angle_xr -= doubleB;

}

}
 float cos_b = cos ( angle_xr );
 float new_x = cos_b * x;
 float new_y = cos_b * proj_y;
 float new_z = cos_b * proj_z;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
