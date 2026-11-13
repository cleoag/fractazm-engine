// Generated from Ads/_FlipXYc.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7004 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FLIPXYC = 7004;
 template < > FormulaVec3 execute_formula < FORMULA__FLIPXYC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 if ( params [ 0 ] != 0.0f ) {
 params [ WRITE_J1_SLOT ] = params [ 2 ];
 params [ WRITE_J2_SLOT ] = params [ 1 ];

}
 return FormulaVec3 ( p . y , p . x , p . z );

}
 FormulaVec3 _FlipXYc_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( p . y , p . x , p . z );

}

}}
