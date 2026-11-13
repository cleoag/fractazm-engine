// Generated from 4D/Beth1241.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10329 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH1241 = 10329;
 template < > FormulaVec3 execute_formula < FORMULA_BETH1241 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float w_add = params [ 0 ];
 const float c0 = - 0.5f;
 const float c1 = 3.0f;
 const float c2 = 10.0f;
 const float c3 = 15.0f;
 const float c4 = 30.0f;
 float x = p . x , y = p . y , z = p . z , w = de;
 float a2 = x * x;
 float b2 = y * y;
 float c2_ = z * z;
 float d2 = w * w;
 float A4 = a2 * a2;
 float B4 = b2 * b2;
 float C4 = c2_ * c2_;
 float D4 = d2 * d2;
 float nx = c0 * x * ( c3 * ( B4 + C4 + D4 ) - c4 * ( b2 * c2_ + c2_ * d2 + d2 * b2 ) + c2 * a2 * ( b2 + c2_ + d2 ) + c1 * A4 );
 float ny = c0 * y * ( c3 * ( C4 + D4 + A4 ) - c4 * ( c2_ * d2 + d2 * a2 + a2 * c2_ ) + c2 * b2 * ( c2_ + d2 + a2 ) + c1 * B4 );
 float nz = c0 * z * ( c3 * ( D4 + A4 + B4 ) - c4 * ( d2 * a2 + a2 * b2 + b2 * d2 ) + c2 * c2_ * ( d2 + a2 + b2 ) + c1 * C4 );
 float nw = c0 * w * ( c3 * ( A4 + B4 + C4 ) - c4 * ( a2 * b2 + b2 * c2_ + c2_ * a2 ) + c2 * d2 * ( a2 + b2 + c2_ ) + c1 * D4 ) + w_add;
 de = nw;
 return FormulaVec3 ( nx , ny , nz );

}

}}
