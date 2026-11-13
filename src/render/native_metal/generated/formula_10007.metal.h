// Generated from Ads/_HexaTransform.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10007 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__HEXATRANSFORM = 10007;
 template < > FormulaVec3 execute_formula < FORMULA__HEXATRANSFORM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Offset = params [ 0 ];
 const float k1 = params [ 1 ];
 const float k2 = params [ 2 ];
 float new_x = fabs ( p . x ) - Offset;
 float new_y = fabs ( p . y * k1 + p . x * k2 ) - Offset;
 float new_z = fabs ( p . z ) - Offset;
 params [ FORMULA_STATE_W_SLOT ] = fabs ( p . x * k2 - p . y * k1 ) - Offset;
 const float cx = params [ 3 ] , cy = params [ 4 ] , cz = params [ 5 ];
 params [ WRITE_J1_SLOT ] = fabs ( cx ) - Offset;
 params [ WRITE_J2_SLOT ] = fabs ( cy * k1 + cx * k2 ) - Offset;
 params [ WRITE_J3_SLOT ] = fabs ( cz ) - Offset;
 params [ WRITE_J4_SLOT ] = fabs ( cx * k2 - cy * k1 ) - Offset;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
