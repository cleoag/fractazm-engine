// Generated from 4D/Beth641.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5341 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH641 = 5341;
 template < > FormulaVec3 execute_formula < FORMULA_BETH641 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float W_add = params [ 0 ];
 const float K0 = 3.0f;
 const float K1 = 5.0f;
 const float K2 = - 10.0f;
 float x = p . x , y = p . y , z = p . z , w = de;
 float A = x * x;
 float B = y * y;
 float C = z * z;
 float D = w * w;
 float A2 = A * A;
 float B2 = B * B;
 float C2 = C * C;
 float D2 = D * D;
 float x_out = x * ( K0 * A2 + K1 * ( B2 + C2 + D2 ) + K2 * A * ( B + C + D ) );
 float y_out = y * ( K0 * B2 + K1 * ( A2 + C2 + D2 ) + K2 * B * ( A + C + D ) );
 float z_out = z * ( K0 * C2 + K1 * ( A2 + B2 + D2 ) + K2 * C * ( A + B + D ) );
 float w_out = w * ( K0 * D2 + K1 * ( A2 + B2 + C2 ) + K2 * D * ( A + B + C ) ) + W_add;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
