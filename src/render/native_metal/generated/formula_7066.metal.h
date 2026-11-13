// Generated from Ads/_poincare.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7066 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__POINCARE = 7066;
 template < > FormulaVec3 execute_formula < FORMULA__POINCARE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Xctr = params [ 0 ];
 const float Yctr = params [ 1 ];
 const float Zctr = params [ 2 ];
 const float Radius = params [ 3 ];
 const float P_Radius = params [ 4 ];
 float x = p . x + Xctr;
 float y = p . y + Yctr;
 float z = p . z + Zctr;
 float r2 = x * x + y * y + z * z;
 float rho = abs ( P_Radius - r2 );
 float new_x = ( abs ( x ) / rho ) * Radius - Xctr;
 float new_y = ( abs ( y ) / rho ) * Radius - Yctr;
 float new_z = ( abs ( z ) / rho ) * Radius - Zctr;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
