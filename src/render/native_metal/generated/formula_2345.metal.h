// Generated from Ads/_tospherical.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_2345 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__TOSPHERICAL = 2345;
 template < > FormulaVec3 execute_formula < FORMULA__TOSPHERICAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x , y2 = y * y , z2 = z * z;
 float r = sqrt ( x2 + y2 + z2 );
 float rxy = sqrt ( x2 + y2 );
 float theta = atan2 ( rxy , z );
 float phi = atan2 ( y , x );
 return FormulaVec3 ( r , theta , phi );

}
 FormulaVec3 _tospherical_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float x = p . x , y = p . y , z = p . z;
 float r = native_formula_length3 ( x , y , z );
 return FormulaVec3 ( r , atan2 ( sqrt ( x * x + y * y ) , z ) , atan2 ( y , x ) );

}

}}
