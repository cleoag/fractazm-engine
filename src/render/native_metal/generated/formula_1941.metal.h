// Generated from Ads/_hyc3Dinv.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1941 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__HYC3DINV = 1941;
 template < > FormulaVec3 execute_formula < FORMULA__HYC3DINV > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: X87Scope extendedScope;
 const float X_center = params [ 0 ];
 const float Y_center = params [ 1 ];
 const float Z_center = params [ 2 ];
 const float Radius = params [ 3 ];
 float a = p . x + X_center;
 float b = p . y + Y_center;
 float c = p . z + Z_center;
 const numeric :: X87Real aa = a , bb = b , cc = c;
 const numeric :: X87Real product = aa * cc * bb;
 const numeric :: X87Real sum = aa * aa * aa + bb * bb * bb + cc * cc * cc;
 const numeric :: X87Real det = - ( product - ( ( sum - product ) - product ) );
 const numeric :: X87Real new_x = ( aa * aa - bb * cc ) / det * Radius - X_center;
 const numeric :: X87Real new_y = ( bb * bb - aa * cc ) / det * Radius - Y_center;
 const numeric :: X87Real new_z = ( cc * cc - aa * bb ) / det;
 return FormulaVec3 ( float ( new_z ) , float ( new_x ) , float ( new_y * Radius - Z_center ) );

}

}}
