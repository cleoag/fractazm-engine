// Generated from Ads/_PolyFolding.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7065 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__POLYFOLDING = 7065;
 template < > FormulaVec3 execute_formula < FORMULA__POLYFOLDING > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Order = params [ 0 ];
 const float Shift_deg = params [ 1 ];
 const float Shift_x = params [ 2 ];
 const float Shift_y = params [ 3 ];
 const float Skew_deg = params [ 4 ];
 const float TWOPI = 6.283185307f;
 const float DEG2RAD = 0.017453293f;
 float x = p . x , y = p . y;
 float angle = atan2 ( y + Shift_y , x + Shift_x );
 float combined = angle + ( Skew_deg + Shift_deg ) * DEG2RAD;
 float sector = rint ( combined / TWOPI * Order );
 float rot = - sector / Order * TWOPI + Shift_deg * DEG2RAD;
 float c = cos ( rot );
 float s = sin ( rot );
 float new_x = y * s - x * c;
 float new_y = x * s + y * c;
 return FormulaVec3 ( new_x , new_y , p . z );

}

}}
