// Generated from Ads/_flipC-xyz.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6004 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FLIPCXYZ = 6004;
 template < > FormulaVec3 execute_formula < FORMULA__FLIPCXYZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 params [ WRITE_J1_SLOT ] = p . x;
 params [ WRITE_J2_SLOT ] = p . y;
 params [ WRITE_J3_SLOT ] = p . z;
 const float Cx = params [ 0 ];
 const float Cy = params [ 1 ];
 const float Cz = params [ 2 ];
 return FormulaVec3 ( Cx , Cy , Cz );

}
 FormulaVec3 _flipCxyz_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( params [ 0 ] , params [ 1 ] , params [ 2 ] );

}

}}
