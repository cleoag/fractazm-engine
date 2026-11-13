// Generated from Ads/_YplusSinZ2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7011 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__YPLUSSINZ2 = 7011;
 template < > FormulaVec3 execute_formula < FORMULA__YPLUSSINZ2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float h = params [ 0 ];
 const float k = params [ 1 ];
 float wave = 0.5f * ( 2.0f - k + k * cos ( M_PI * h * p . z ) );
 float new_y = p . y + wave * sin ( p . z );
 return FormulaVec3 ( p . x , new_y , p . z );

}
 FormulaVec3 _YplusSinZ2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__YPLUSSINZ2 > ( p , params , de );

}

}}
