// Generated from 4D/Beth1661dual.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10341 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH1661DUAL = 10341;
 template < > FormulaVec3 execute_formula < FORMULA_BETH1661DUAL > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float w_add = params [ 0 ];
 const float c0 = - 0.39506172839506172839f;
 const float c1 = - 42.0f;
 const float c2 = - 70.0f;
 const float c3 = 28.0f;
 const float c4 = - 105.0f;
 const float c5 = 6.0f;
 const float x = p . x;
 const float y = p . y;
 const float z = p . z;
 const float w = de;
 const float x2 = x * x;
 const float y2 = y * y;
 const float z2 = z * z;
 const float w2 = w * w;
 const float x4 = x2 * x2;
 const float y4 = y2 * y2;
 const float z4 = z2 * z2;
 const float w4 = w2 * w2;
 const float x6 = x4 * x2;
 const float y6 = y4 * y2;
 const float z6 = z4 * z2;
 const float w6 = w4 * w2;
 const float yz_pairs = y2 * z2 + z2 * w2 + w2 * y2;
 const float zx_pairs = z2 * w2 + w2 * x2 + x2 * z2;
 const float xy_pairs = w2 * x2 + x2 * y2 + y2 * w2;
 const float wx_pairs = x2 * y2 + y2 * z2 + z2 * x2;
 const float x_cross = y4 * z2 + z4 * w2 + w4 * y2 + y2 * z4 + z2 * w4 + w2 * y4;
 const float y_cross = z4 * w2 + w4 * x2 + x4 * z2 + z2 * w4 + w2 * x4 + x2 * z4;
 const float z_cross = w4 * x2 + x4 * y2 + y4 * w2 + w2 * x4 + x2 * y4 + y2 * w4;
 const float w_cross = x4 * y2 + y4 * z2 + z4 * x2 + x2 * y4 + y2 * z4 + z2 * x4;
 const float nx = c0 * x * ( c3 * ( y6 + z6 + w6 ) + c4 * x_cross + c2 * x2 * ( ( y4 + z4 + w4 ) - c5 * yz_pairs ) + c1 * x4 * ( y2 + z2 + w2 ) + c5 * x6 );
 const float ny = c0 * y * ( c3 * ( z6 + w6 + x6 ) + c4 * y_cross + c2 * y2 * ( ( z4 + w4 + x4 ) - c5 * zx_pairs ) + c1 * y4 * ( z2 + w2 + x2 ) + c5 * y6 );
 const float nz = c0 * z * ( c3 * ( w6 + x6 + y6 ) + c4 * z_cross + c2 * z2 * ( ( w4 + x4 + y4 ) - c5 * xy_pairs ) + c1 * z4 * ( w2 + x2 + y2 ) + c5 * z6 );
 const float nw = c0 * w * ( c3 * ( x6 + y6 + z6 ) + c4 * w_cross + c2 * w2 * ( ( x4 + y4 + z4 ) - c5 * wx_pairs ) + c1 * w4 * ( x2 + y2 + z2 ) + c5 * w6 ) + w_add;
 de = nw;
 return FormulaVec3 ( nx , ny , nz );

}

}}
