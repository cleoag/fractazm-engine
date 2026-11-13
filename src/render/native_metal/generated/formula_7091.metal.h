// Generated from Ads/_hopalm1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7091 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__HOPALM1 = 7091;
 template < > FormulaVec3 execute_formula < FORMULA__HOPALM1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Scale_xy = params [ 0 ];
 const float Center = params [ 1 ];
 const float Scale_x = params [ 2 ];
 const float Scale_z = params [ 3 ];
 const float Offset = params [ 4 ];
 const float Scale_yz = params [ 5 ];
 float x = p . x , y = p . y , z = p . z;
 float val = x * Scale_x - z * Scale_z + Offset;
 float abs_val = abs ( val );
 float sq = sqrt ( abs_val );
 float hop;
 if ( abs_val < 1.0f ) {
 hop = sq;

}
 else {
 hop = ( sq > 1e-30f ) ? ( 1.0f / sq ) : 0.0f;

}
 hop = copysign ( hop , val );
 float new_x = y + hop;
 float new_y = ( Center - x ) * Scale_xy;
 float new_z = z + ( Center * y - x * Scale_x ) * Scale_yz;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
