// Generated from Ads/_Translate.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8065 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__TRANSLATE = 8065;
 template < > FormulaVec3 execute_formula < FORMULA__TRANSLATE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_add = params [ 0 ];
 const float Y_add = params [ 1 ];
 const float Z_add = params [ 2 ];
 return FormulaVec3 ( native_translate_add ( p . x , X_add ) , native_translate_add ( p . y , Y_add ) , native_translate_add ( p . z , Z_add ) );

}
 FormulaVec3 _Translate_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return FormulaVec3 ( native_translate_add ( p . x , params [ 0 ] ) , native_translate_add ( p . y , params [ 1 ] ) , native_translate_add ( p . z , params [ 2 ] ) );

}

}}
