// Generated from 3D/Beth1032.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10266 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BETH1032 = 10266;
 template < > FormulaVec3 execute_formula < FORMULA_BETH1032 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float K1 = 0.69135802469135802f;
 const float K2 = 21.416407864998738f;
 const float K3 = - 5.4164078649987382f;
 const float K4 = - 25.623058987490536f;
 const float K5 = 175.62305898749054f;
 const float K6 = 27.124611797498107f;
 const float K7 = - 13.124611797498107f;
 const float K8 = 7.7082039324993691f;
 const float K9 = - 5.7082039324993691f;
 const float K10 = 2.0f;
 const float K11 = 3.0f;
 const float K12 = 15.0f;
 const float K13 = 20.0f;
 const float Cx_val = params [ 13 ];
 const float Cy_val = params [ 14 ];
 const float Cz_val = params [ 15 ];
 float x = p . x , y = p . y , z = p . z;
 float a = x * x;
 float b = y * y;
 float c = z * z;
 float A = a * a;
 float B = b * b;
 float C = c * c;
 float inner_x = K2 * B * b + K3 * C * c - K4 * B * c - K5 * b * C + K12 * a * ( K8 * B + K9 * C - K13 * b * c ) - K11 * A * ( K7 * b + K6 * c ) + K10 * A * a;
 float ix = K2 * B * b + K3 * C * c - K4 * B * c - K5 * b * C - K12 * a * ( K8 * B + K9 * C - K13 * b * c ) - K11 * A * ( K7 * b + K6 * c ) + K10 * A * a;
 float out_x = K1 * x * ix + Cx_val;
 float iy = K2 * C * c + K3 * A * a - K4 * C * a - K5 * c * A - K12 * b * ( K8 * C + K9 * A - K13 * c * a ) - K11 * B * ( K7 * c + K6 * a ) + K10 * B * b;
 float out_y = K1 * y * iy + Cy_val;
 float iz = K2 * A * a + K3 * B * b - K4 * A * b - K5 * a * B - K12 * c * ( K8 * A + K9 * B - K13 * a * b ) - K11 * C * ( K7 * a + K6 * b ) + K10 * C * c;
 float out_z = K1 * z * iz + Cz_val;
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
