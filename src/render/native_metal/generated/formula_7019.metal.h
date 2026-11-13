// Generated from Ads/_LogX.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7019 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__LOGX = 7019;
 template < > FormulaVec3 execute_formula < FORMULA__LOGX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixX = params [ 0 ];
 const float fixLg = params [ 1 ];
 const float Mul = params [ 2 ];
 const float Base = params [ 3 ];
 float xp = Mul * p . x + fixX;
 float s = metal :: signbit ( xp ) ? - 1.0f : 1.0f;
 float result = s * Base * log2 ( abs ( xp ) + abs ( fixLg ) );
 return FormulaVec3 ( result , p . y , p . z );

}
 FormulaVec3 _LogX_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__LOGX > ( p , params , de );

}

}}
