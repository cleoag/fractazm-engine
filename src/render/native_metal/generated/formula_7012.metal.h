// Generated from Ads/_sqr_3D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7012 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SQR_3D = 7012;
 template < > FormulaVec3 execute_formula < FORMULA__SQR_3D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float fixX = params [ 1 ];
 const float fixY = params [ 2 ];
 const float fixZ = params [ 3 ];
 float new_x = p . x * p . x * Scale + fixX;
 float new_y = p . y * p . y * Scale + fixY;
 float new_z = p . z * p . z * Scale + fixZ;
 return FormulaVec3 ( new_x , new_y , new_z );

}
 FormulaVec3 _sqr_3D_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA__SQR_3D > ( p , params , de );

}

}}
