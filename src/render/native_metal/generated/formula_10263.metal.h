// Generated from 3D/Riemann2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10263 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_RIEMANN2 = 10263;
 template < > FormulaVec3 execute_formula < FORMULA_RIEMANN2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 RotMatrix3 Rot = rotMatrix3FromFloats ( params , 0 );
 const float R00 = Rot . col0 . x , R01 = Rot . col1 . x , R02 = Rot . col2 . x;
 const float R10 = Rot . col0 . y , R11 = Rot . col1 . y , R12 = Rot . col2 . y;
 const float R20 = Rot . col0 . z , R21 = Rot . col1 . z , R22 = Rot . col2 . z;
 const float Scale = params [ 9 ];
 const float RPow = params [ 10 ];
 const float r_shift = params [ 11 ];
 const float s_shift = params [ 12 ];
 const float t_shift = params [ 13 ];
 const float x_shift = params [ 14 ];
 const float sin_addfold = params [ 15 ];
 const float smallest = params [ 16 ];
 float x = p . x , y = p . y , z = p . z;
 float rx = x * R00 + y * R01 + z * R02;
 float ry = x * R10 + y * R11 + z * R12;
 float rz = x * R20 + y * R21 + z * R22;
 x = rx;
 y = ry;
 z = rz;
 float r = native_formula_length3 ( x , y , z );
 float sf = Scale / r;
 x *= sf;
 y *= sf;
 z *= sf;
 float denom = 1.0f - x;
 float s_raw , t_raw;
 if ( denom > smallest ) {
 float inv_denom = 1.0f / denom;
 s_raw = z * inv_denom;
 t_raw = y * inv_denom;

}
 else {
 s_raw = z;
 t_raw = y;

}
 float w_power = 1.0f + s_raw * s_raw + t_raw * t_raw;
 float s_fold = abs ( abs ( sin ( params [ 17 ] * s_raw + s_shift ) ) + sin_addfold );
 float t_fold = abs ( abs ( sin ( params [ 17 ] * t_raw + t_shift ) ) + sin_addfold );
 float r_new = r_shift + x87_log_power ( abs ( r ) , RPow * w_power );
 float w_new = params [ 18 ] / ( 1.0f + s_fold * s_fold + t_fold * t_fold );
 float out_x = r_new * ( 1.0f - w_new ) + x_shift;
 float out_y = r_new * t_fold * w_new;
 float out_z = r_new * s_fold * w_new;
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
