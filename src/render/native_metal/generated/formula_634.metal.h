// Generated from Ads/_kamtorcut.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_634 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__KAMTORCUT = 634;
 template < > FormulaVec3 execute_formula < FORMULA__KAMTORCUT > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Soft1 = params [ 0 ];
 const float Soft2 = params [ 1 ];
 const float A1 = params [ 2 ];
 const float B1 = params [ 3 ];
 const float C1 = params [ 4 ];
 const float D1 = params [ 5 ];
 const float E1 = params [ 6 ];
 const float F1 = params [ 7 ];
 const float A2 = params [ 8 ];
 const float B2 = params [ 9 ];
 const float C2 = params [ 10 ];
 const float D2 = params [ 11 ];
 const float E2 = params [ 12 ];
 const float F2 = params [ 13 ];
 const float Bailout = params [ 14 ];
 const float Cutoff = params [ 15 ];
 float x = p . x , y = p . y , z = p . z;
 float r = sqrt ( x * x + y * y );
 z = Soft1 * z + Soft2 * r;
 if ( ! isfinite ( z ) || fabs ( z ) >= Cutoff ) {
 return FormulaVec3 ( Bailout , Bailout , z );

}
 float t1 = z;
 float t2 = sqrt ( 1.0f - z * z );
 float poly1 = A1 * x + B1 * y + C1 * x * x + D1 * y * y + E1 * x * y + F1 * r;
 float poly2 = A2 * x + B2 * y + C2 * x * x + D2 * y * y + E2 * x * y + F2 * r;
 float new_x = poly1 * t1 + poly2 * t2;
 float new_y = poly1 * t2 + poly2 * t1;
 return FormulaVec3 ( new_x , new_y , z );

}

}}
