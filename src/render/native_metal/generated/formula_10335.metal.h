// Generated from 4D/_Tanh4D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10335 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TANH4D = 10335;
 template < > FormulaVec3 execute_formula < FORMULA_TANH4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Mx = params [ 0 ];
 const float My = params [ 1 ];
 const float Mz = params [ 2 ];
 const float Mw = params [ 3 ];
 const float fmul = params [ 4 ];
 const float eps = 1e-200f;
 float x = p . x , y = p . y , z = p . z , w = de;
 float qx = x * Mx - y * My - z * Mz - w * Mw;
 float qy = x * My + y * Mx + z * Mw - w * Mz;
 float qz = x * Mz - y * Mw + z * Mx + w * My;
 float qw = x * Mw + y * Mz - z * My + w * Mx;
 float imag_len = native_formula_length3 ( qw , qy , qz );
 float cos_v = cos ( imag_len );
 float sin_v = sin ( imag_len );
 float exp_a = exp ( qx );
 float e_re = exp_a * cos_v;
 float scale = exp_a * sin_v;
 if ( imag_len > eps ) {
 scale /= imag_len;

}
 float e_y = qy * scale;
 float e_z = qz * scale;
 float e_w = qw * scale;
 float denom = ( e_re + 1.0f ) * ( e_re + 1.0f ) + e_y * e_y + e_z * e_z + e_w * e_w;
 float inv_d = 1.0f / denom;
 float t_re = ( ( e_re + 1.0f ) * ( e_re - 1.0f ) + e_y * e_y + e_z * e_z + e_w * e_w );
 float t_y = ( e_re + 1.0f ) * e_y - e_y * ( e_re - 1.0f );
 float t_z = ( e_re + 1.0f ) * e_z - e_z * ( e_re - 1.0f );
 float t_w = ( e_re + 1.0f ) * e_w - e_w * ( e_re - 1.0f );
 float nx = inv_d * t_re * fmul;
 float ny = inv_d * t_y * fmul;
 float nz = inv_d * t_z * fmul;
 float nw = inv_d * t_w * fmul;
 de = nw;
 return FormulaVec3 ( nx , ny , nz );

}

}}
