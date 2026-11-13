// Generated from 3D/Msltoe_Sym.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_2758 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MSLTOE_SYM = 2758;
 template < > FormulaVec3 execute_formula < FORMULA_MSLTOE_SYM > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float half_range = params [ 0 ];
 const float half_step = params [ 1 ];
 const float coeff_c = params [ 2 ];
 const float coeff_d = params [ 3 ];
 const float angle_offset = params [ 4 ];
 float x = p . x , y = p . y , z = p . z;
 float theta = atan2 ( z , y ) + angle_offset;
 float acc = 0.0f;
 while ( theta > half_range ) {
 theta -= half_step;
 acc -= half_step;

}
 float cos_acc = cos ( acc );
 float sin_acc = sin ( acc );
 float y_rot = y * cos_acc - z * sin_acc;
 float z_rot = y * sin_acc + z * cos_acc;
 float r2 = x * x + y_rot * y_rot + z_rot * z_rot;
 float factor = coeff_c - ( z_rot * z_rot ) / r2;
 float term1 = ( x * x - y_rot * y_rot ) * factor;
 float term2 = coeff_d * x * y_rot * factor;
 float rho = sqrt ( x * x + y_rot * y_rot );
 float term3 = coeff_d * z_rot * rho;
 float new_x = term1;
 float new_y = term2 * cos_acc + term3 * sin_acc;
 float new_z = term3 * cos_acc - term2 * sin_acc;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
