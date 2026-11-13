// Generated from Ads/_scaleC.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6003 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SCALEC = 6003;
 template < > FormulaVec3 execute_formula < FORMULA__SCALEC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float ScaleCx = params [ 0 ];
 const float ScaleCy = params [ 1 ];
 const float ScaleCz = params [ 2 ];
 const float Cx = params [ 3 ];
 const float Cy = params [ 4 ];
 const float Cz = params [ 5 ];
 params [ WRITE_J1_SLOT ] = Cx * ScaleCx;
 params [ WRITE_J2_SLOT ] = Cy * ScaleCy;
 params [ WRITE_J3_SLOT ] = Cz * ScaleCz;
 return p;

}
 FormulaVec3 _scaleC_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return p;

}

}}
