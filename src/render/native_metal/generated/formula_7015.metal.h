// Generated from Ads/_updateC_4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7015 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__UPDATEC_4D = 7015;
 template < > FormulaVec3 execute_formula < FORMULA__UPDATEC_4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 params [ WRITE_J1_SLOT ] = p . x;
 params [ WRITE_J2_SLOT ] = p . y;
 params [ WRITE_J3_SLOT ] = p . z;
 params [ WRITE_J4_SLOT ] = params [ FORMULA_STATE_W_SLOT ];
 return p;

}
 FormulaVec3 _updateC_4d_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return p;

}

}}
