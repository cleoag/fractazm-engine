// Generated from 4D/MandyCousin.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6473 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MANDYCOUSIN = 6473;
 template < > FormulaVec3 execute_formula < FORMULA_MANDYCOUSIN > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float X_mul = params [ 0 ];
 const float Y_mul = params [ 1 ];
 const float Z_mul1 = params [ 2 ];
 const float Z_mul2 = params [ 3 ];
 const float W_mul = params [ 4 ];
 const float W_add = params [ 5 ];
 float x = p . x , y = p . y , z = p . z , w = de;
 float x_out = X_mul * ( x * x - y * y - z * z + w * w );
 float y_out = Y_mul * ( x * y + z * w );
 float z_out = Z_mul1 * x * z + Z_mul2 * y * w;
 float w_out = W_mul * ( x * w + y * z ) + W_add;
 de = w_out;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
