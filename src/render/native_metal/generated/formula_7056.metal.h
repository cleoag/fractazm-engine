// Generated from 3D/Kalisets1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7056 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KALISETS1 = 7056;
 template < > FormulaVec3 execute_formula < FORMULA_KALISETS1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float radius = p . x * p . x + p . y * p . y + p . z * p . z;
 const float factor = params [ 0 ] / ( radius + params [ 1 ] );
 return FormulaVec3 ( abs ( p . x ) * factor + params [ 2 ] , abs ( p . y ) * factor + params [ 3 ] , abs ( p . z ) * factor + params [ 4 ] );

}

}}
