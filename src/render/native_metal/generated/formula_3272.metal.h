// Generated from Ads/_QuadrayTransform2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3272 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__QUADRAYTRANSFORM2 = 3272;
 template < > FormulaVec3 execute_formula < FORMULA__QUADRAYTRANSFORM2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Offset = params [ 0 ];
 const float Scale = params [ 1 ];
 float x = p . x , y = p . y , z = p . z;
 FormulaVec3 result;
 result . x = Scale * abs ( x + y + z ) + Offset;
 result . y = Scale * abs ( - x - y + z ) + Offset;
 result . z = Scale * abs ( - x + y - z ) + Offset;
 params [ FORMULA_STATE_W_SLOT ] = Scale * abs ( x - y - z ) + Offset;
 const float cx = params [ 2 ] , cy = params [ 3 ] , cz = params [ 4 ];
 params [ WRITE_J1_SLOT ] = Scale * abs ( cx + cy + cz ) + Offset;
 params [ WRITE_J2_SLOT ] = Scale * abs ( cz - cx - cy ) + Offset;
 params [ WRITE_J3_SLOT ] = Scale * abs ( cy - cx - cz ) + Offset;
 params [ WRITE_J4_SLOT ] = Scale * abs ( cx - cy - cz ) + Offset;
 return result;

}
 FormulaVec3 _QuadrayTransform2_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float O = params [ 0 ] , S = params [ 1 ];
 float x = p . x , y = p . y , z = p . z;
 de = S * abs ( x - y - z ) + O;
 return FormulaVec3 ( S * abs ( x + y + z ) + O , S * abs ( - x - y + z ) + O , S * abs ( - x + y - z ) + O );

}

}}
