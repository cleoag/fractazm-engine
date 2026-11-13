// Generated from Ads/_addC.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7280 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ADDC = 7280;
 template < > FormulaVec3 execute_formula < FORMULA__ADDC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float Cx = params [ 3 ];
 const float Cy = params [ 4 ];
 const float Cz = params [ 5 ];
 FormulaVec3 result;
 result . x = p . x + Cx * X_mul;
 result . y = p . y + Cy * Y_mul;
 result . z = p . z + Cz * Z_mul;
 return result;

}
 FormulaVec3 _addC_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul = params [ 2 ];
 const float Cx = params [ 3 ];
 const float Cy = params [ 4 ];
 const float Cz = params [ 5 ];
 FormulaVec3 result;
 result . x = p . x + Cx * X_mul;
 result . y = p . y + Cy * Y_mul;
 result . z = p . z + Cz * Z_mul;
 return result;

}

}}
