// Generated from Ads/_SinhY.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7028 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SINHY = 7028;
 template < > FormulaVec3 execute_formula < FORMULA__SINHY > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fixY = params [ 0 ];
 const float fixLg = params [ 1 ];
 const float Mul = params [ 2 ];
 const float Base = params [ 3 ];
 float A = Mul * p . y + fixY;
 float arg = A + sqrt ( A * A + 1.0f );
 float result = Base * log2 ( arg ) - fixLg;
 return FormulaVec3 ( p . x , result , p . z );

}

}}
