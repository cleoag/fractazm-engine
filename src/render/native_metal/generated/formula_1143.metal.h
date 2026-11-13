// Generated from Ads/_boxtiling.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1143 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BOXTILING = 1143;
 template < > FormulaVec3 execute_formula < FORMULA__BOXTILING > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float x_tile = params [ 0 ];
 const float y_tile = params [ 1 ];
 const float z_tile = params [ 2 ];
 float x = p . x , y = p . y , z = p . z;
 if ( x_tile != 0.0f ) {
 x = x - rint ( x / x_tile ) * x_tile;

}
 if ( y_tile != 0.0f ) {
 y = y - rint ( y / y_tile ) * y_tile;

}
 if ( z_tile != 0.0f ) {
 z = z - rint ( z / z_tile ) * z_tile;

}
 return FormulaVec3 ( x , y , z );

}

}}
