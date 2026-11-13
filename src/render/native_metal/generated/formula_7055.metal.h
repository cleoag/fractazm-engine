// Generated from Ads/_updateC.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7055 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__UPDATEC = 7055;
 template < > FormulaVec3 execute_formula < FORMULA__UPDATEC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float xyz_mul = params [ 0 ];
 const float C_mul = params [ 1 ];
 const float Cx = params [ 2 ];
 const float Cy = params [ 3 ];
 const float Cz = params [ 4 ];
 params [ WRITE_J1_SLOT ] = p . x * xyz_mul + Cx * C_mul;
 params [ WRITE_J2_SLOT ] = p . y * xyz_mul + Cy * C_mul;
 params [ WRITE_J3_SLOT ] = p . z * xyz_mul + Cz * C_mul;
 return p;

}

}}
