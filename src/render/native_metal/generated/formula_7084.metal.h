// Generated from 3D/Benesi2Pow2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7084 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BENESI2POW2 = 7084;
 template < > FormulaVec3 execute_formula < FORMULA_BENESI2POW2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Cx = params [ 0 ];
 const float Cy = params [ 1 ];
 const float Cz = params [ 2 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float rho_xy2 = x2 + y2;
 float rho_xy = sqrt ( rho_xy2 );
 float new_x = x2 - y2 - z2 + Cx;
 float new_y = fabs ( Cy ) - fabs ( 2.0f * z * rho_xy );
 float z_numer = 2.0f * x * y * ( rho_xy2 - z2 );
 float z_term;
 if ( rho_xy2 == 0.0f && ( x != 0.0f || y != 0.0f ) ) {
 z_term = 0.0f;

}
 else {
 z_term = z_numer / rho_xy2;

}
 float new_z = fabs ( Cz ) - fabs ( z_term );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
