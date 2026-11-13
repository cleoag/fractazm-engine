// Generated from Ads/_toruspinv2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7068 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__TORUSPINV2 = 7068;
 template < > FormulaVec3 execute_formula < FORMULA__TORUSPINV2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Xctr = params [ 0 ];
 const float Yctr = params [ 1 ];
 const float Zctr = params [ 2 ];
 const float Radius = params [ 3 ];
 const float R = params [ 4 ];
 float x = p . x + Xctr;
 float y = p . y + Yctr;
 float z = p . z + Zctr;
 float t = R - ( x * x + y * y );
 float d = sqrt ( t * t + z * z );
 float new_x = abs ( x ) * Radius / d - Xctr;
 float new_y = abs ( y ) * Radius / d - Yctr;
 float new_z = Radius * d * abs ( z ) - Zctr;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
