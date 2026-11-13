// Generated from Ads/_LinCombineXYZ.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7032 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__LINCOMBINEXYZ = 7032;
 template < > FormulaVec3 execute_formula < FORMULA__LINCOMBINEXYZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Xx = params [ 0 ] , Xy = params [ 1 ] , Xz = params [ 2 ];
 const float Yx = params [ 3 ] , Yy = params [ 4 ] , Yz = params [ 5 ];
 const float Zx = params [ 6 ] , Zy = params [ 7 ] , Zz = params [ 8 ];
 float new_x = Xx * p . x + Xy * p . y + Xz * p . z;
 float new_y = Yx * p . x + Yy * p . y + Yz * p . z;
 float new_z = Zx * p . x + Zy * p . y + Zz * p . z;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
