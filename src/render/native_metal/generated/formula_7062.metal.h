// Generated from Ads/_RotateC.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7062 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__ROTATEC = 7062;
 template < > FormulaVec3 execute_formula < FORMULA__ROTATEC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 RotMatrix3 R = rotMatrix3FromFloats ( params , 0 );
 const float Cx = params [ 9 ];
 const float Cy = params [ 10 ];
 const float Cz = params [ 11 ];
 params [ WRITE_J1_SLOT ] = Cx * R . m [ 0 ] [ 0 ] + Cy * R . m [ 0 ] [ 1 ] + Cz * R . m [ 0 ] [ 2 ];
 params [ WRITE_J2_SLOT ] = Cx * R . m [ 1 ] [ 0 ] + Cy * R . m [ 1 ] [ 1 ] + Cz * R . m [ 1 ] [ 2 ];
 params [ WRITE_J3_SLOT ] = Cx * R . m [ 2 ] [ 0 ] + Cy * R . m [ 2 ] [ 1 ] + Cz * R . m [ 2 ] [ 2 ];
 return p;

}

}}
