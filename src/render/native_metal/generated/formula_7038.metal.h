// Generated from 3D/KaliDucks.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7038 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_KALIDUCKS = 7038;
 template < > FormulaVec3 execute_formula < FORMULA_KALIDUCKS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float radius = params [ FORMULA_ROUT_SLOT ];
 const float factor = params [ 1 ] > radius ? 1.0f / ( params [ 1 ] * params [ 1 ] ) : params [ 0 ] / radius;
 return FormulaVec3 ( abs ( p . x ) * factor + params [ 2 ] , abs ( p . y ) * factor + params [ 3 ] , abs ( p . z ) * factor + params [ 4 ] );

}

}}
