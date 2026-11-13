// Generated from Ads/_reciprocalZ2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7009 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__RECIPROCALZ2 = 7009;
 template < > FormulaVec3 execute_formula < FORMULA__RECIPROCALZ2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Limiter = params [ 0 ];
 float new_z = copysign ( 1.0f , p . z ) / ( abs ( p . z ) + Limiter );
 return FormulaVec3 ( p . x , p . y , new_z );

}
 FormulaVec3 _reciprocalZ2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( p . x , p . y , copysign ( 1.0f , p . z ) / ( abs ( p . z ) + params [ 0 ] ) );

}

}}
