// Generated from 4Da/_ScaleC4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9525 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SCALEC4D = 9525;
 template < > FormulaVec3 execute_formula < FORMULA__SCALEC4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float C_Scale = params [ 1 ];
 params [ 2 ] *= C_Scale;
 params [ 3 ] *= C_Scale;
 params [ 4 ] *= C_Scale;
 params [ 5 ] *= C_Scale;
 params [ WRITE_J1_SLOT ] = params [ 2 ];
 params [ WRITE_J2_SLOT ] = params [ 3 ];
 params [ WRITE_J3_SLOT ] = params [ 4 ];
 params [ WRITE_J4_SLOT ] = params [ 5 ];
 FormulaVec3 result = p * Scale;
 de = de * Scale;
 return result;

}
 FormulaVec3 _ScaleC4d_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale = params [ 0 ];
 const float C_Scale = params [ 1 ];
 params [ 2 ] *= C_Scale;
 params [ 3 ] *= C_Scale;
 params [ 4 ] *= C_Scale;
 params [ 5 ] *= C_Scale;
 params [ WRITE_J1_SLOT ] = params [ 2 ];
 params [ WRITE_J2_SLOT ] = params [ 3 ];
 params [ WRITE_J3_SLOT ] = params [ 4 ];
 params [ WRITE_J4_SLOT ] = params [ 5 ];
 de = de * Scale;
 return p * Scale;

}

}}
