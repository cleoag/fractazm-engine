// Generated from Ads/_BSkewXmaxV1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3912 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BSKEWXMAXV1 = 3912;
 template < > FormulaVec3 execute_formula < FORMULA__BSKEWXMAXV1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float xMin = params [ 0 ];
 const float xScale = params [ 1 ];
 const float xMax = params [ 2 ];
 const float yzScale = params [ 4 ];
 const float xPixelAdd = params [ 6 ];
 const float xPixelScale = params [ 7 ];
 const float Cx = params [ 9 ];
 float x = p . x , y = p . y , z = p . z;
 float ax = fabs ( x );
 float ss1 = ( ax - xMin ) * ( xMax - ax ) * xScale;
 float acx = fabs ( Cx );
 float ss2 = acx * ( acx + xPixelAdd ) * xPixelScale;
 float r_term = sqrt ( y * y + z * z ) * yzScale;
 float new_x = - ( r_term + ss1 + ss2 ) + x;
 return FormulaVec3 ( new_x , y , z );

}

}}
