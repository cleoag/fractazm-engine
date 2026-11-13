// Generated from Ads/_BT1_4D_clampXYZ.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4267 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BT1_4D_CLAMPXYZ = 4267;
 template < > FormulaVec3 execute_formula < FORMULA__BT1_4D_CLAMPXYZ > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 ( void ) de;
 const float A = params [ 2 ];
 const float B = params [ 3 ];
 const float C = params [ 4 ];
 const float D = params [ 5 ];
 const float E = params [ 6 ];
 const float Scale = params [ 0 ];
 const float Offset = params [ 1 ];
 float x = p . x , y = p . y , z = p . z;
 float w = params [ FORMULA_STATE_W_SLOT ];
 float r_orig = native_formula_length3 ( x , y , z );
 float T1 = fabs ( x * E + w * D );
 float T2 = x * D - w * E;
 float T3 = ( T2 * A - z * B ) * C;
 float z_temp = fabs ( T2 * B + z * A );
 float x_minus = fabs ( T3 - y * C );
 float x_plus = fabs ( T3 + y * C );
 float T4 = ( x_minus + x_plus ) * C;
 float y_new = ( x_plus - x_minus ) * C;
 float T5 = T4 * A + z_temp * B;
 float z_new = z_temp * A - T4 * B;
 params [ FORMULA_STATE_W_SLOT ] = T1 * D - T5 * E;
 float x_pre = T5 * D + T1 * E;
 float r_new = native_formula_length3 ( x_pre , y_new , z_new );
 float norm = ( r_new > 1e-30f ) ? ( r_orig / r_new ) : 1.0f;
 float x_out = x_pre * norm * Scale - Offset;
 float y_out = y_new * norm * Scale;
 float z_out = z_new * norm * Scale;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
