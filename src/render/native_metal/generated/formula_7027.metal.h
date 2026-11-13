// Generated from 3D/Aex-Octo.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7027 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_AEX_OCTO = 7027;
 template < > FormulaVec3 execute_formula < FORMULA_AEX_OCTO > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float XZmul = params [ 0 ];
 const float Sqmul = params [ 1 ];
 float new_x = XZmul * p . x * p . z;
 float new_y = Sqmul * ( p . z * p . z - p . x * p . x );
 float new_z = p . y;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
