// Generated from 4D/_Scalingplus.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_371 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SCALINGPLUS = 371;
 template < > FormulaVec3 execute_formula < FORMULA__SCALINGPLUS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_scale = params [ 0 ];
 const float Y_scale = params [ 1 ];
 const float Z_scale = params [ 2 ];
 const float W_scale = params [ 3 ];
 FormulaVec3 result;
 result . x = p . x * X_scale;
 result . y = p . y * Y_scale;
 result . z = p . z * Z_scale;
 de *= W_scale;
 return result;

}
 FormulaVec3 _Scalingplus_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_scale = params [ 0 ];
 const float Y_scale = params [ 1 ];
 const float Z_scale = params [ 2 ];
 const float W_scale = params [ 3 ];
 FormulaVec3 result;
 result . x = p . x * X_scale;
 result . y = p . y * Y_scale;
 result . z = p . z * Z_scale;
 de *= W_scale;
 return result;

}

}}
