// Generated from Ads/_FlipXZc.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7005 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FLIPXZC = 7005;
 template < > FormulaVec3 execute_formula < FORMULA__FLIPXZC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 if ( params [ 0 ] != 0.0f ) {
 params [ WRITE_J1_SLOT ] = params [ 3 ];
 params [ WRITE_J3_SLOT ] = params [ 1 ];

}
 return FormulaVec3 ( p . z , p . y , p . x );

}
 FormulaVec3 _FlipXZc_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( p . z , p . y , p . x );

}

}}
