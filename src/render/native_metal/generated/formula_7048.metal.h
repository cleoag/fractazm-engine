// Generated from Ads/_LinCombineCxyz.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7048 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__LINCOMBINECXYZ = 7048;
 template < > FormulaVec3 execute_formula < FORMULA__LINCOMBINECXYZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float cx = params [ 9 ] , cy = params [ 10 ] , cz = params [ 11 ];
 params [ WRITE_J1_SLOT ] = ( params [ 0 ] * cx + params [ 1 ] * cy ) + params [ 2 ] * cz;
 params [ WRITE_J2_SLOT ] = ( params [ 3 ] * cx + params [ 4 ] * cy ) + params [ 5 ] * cz;
 params [ WRITE_J3_SLOT ] = ( params [ 6 ] * cx + params [ 7 ] * cy ) + params [ 8 ] * cz;
 return p;

}

}}
