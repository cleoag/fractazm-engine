// Generated from Ads/_Folding1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6001 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDING1 = 6001;
 template < > FormulaVec3 execute_formula < FORMULA__FOLDING1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float fold = params [ 0 ];
 float x = abs ( p . x + fold ) - abs ( p . x - fold ) - p . x;
 float y = abs ( p . y + fold ) - abs ( p . y - fold ) - p . y;
 float z = abs ( p . z + fold ) - abs ( p . z - fold ) - p . z;
 return FormulaVec3 ( x , y , z );

}
 FormulaVec3 _Folding1_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float f = params [ 0 ];
 return FormulaVec3 ( abs ( p . x + f ) - abs ( p . x - f ) - p . x , abs ( p . y + f ) - abs ( p . y - f ) - p . y , abs ( p . z + f ) - abs ( p . z - f ) - p . z );

}

}}
