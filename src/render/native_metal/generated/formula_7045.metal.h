// Generated from Ads/_invtorical2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7045 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__INVTORICAL2 = 7045;
 template < > FormulaVec3 execute_formula < FORMULA__INVTORICAL2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float R = params [ 0 ];
 const float Gamma = params [ 1 ];
 float sy , cy;
 sy = sincos ( p . y + Gamma , cy );
 float sz , cz;
 sz = sincos ( p . z , cz );
 float t = R + p . x * cy;
 float new_x = t * cz;
 float new_y = t * sz;
 float new_z = p . x * sy;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
