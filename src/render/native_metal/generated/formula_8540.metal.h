// Generated from 3D/_mulC3D.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8540 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__MULC3D = 8540;
 template < > FormulaVec3 execute_formula < FORMULA__MULC3D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float CAdd_Scale = params [ 0 ];
 const float xyzAdd_Scale = params [ 1 ];
 const float Const_real_x = params [ 2 ];
 const float Const_imag_y = params [ 3 ];
 const float Const_imag_z = params [ 4 ];
 RotMatrix3 Rot = rotMatrix3FromFloats ( params , 5 );
 const float Mx0 = Rot . col0 . x , Mx1 = Rot . col1 . x , Mx2 = Rot . col2 . x;
 const float My0 = Rot . col0 . y , My1 = Rot . col1 . y , My2 = Rot . col2 . y;
 const float Mz0 = Rot . col0 . z , Mz1 = Rot . col1 . z , Mz2 = Rot . col2 . z;
 const float Z_mul = params [ 14 ];
 const float Cx = params [ 15 ];
 const float Cy = params [ 16 ];
 const float Cz = params [ 17 ];
 float x = p . x , y = p . y , z = p . z;
 float term_x = Cx * CAdd_Scale + x * xyzAdd_Scale;
 float term_y = Cy * CAdd_Scale + y * xyzAdd_Scale;
 float term_z = Cz * CAdd_Scale + z * xyzAdd_Scale;
 float Rx = Mx0 * term_x + Mx1 * term_y + Mx2 * term_z + Const_real_x;
 float Ry = My0 * term_x + My1 * term_y + My2 * term_z + Const_imag_y;
 float Rz = Mz0 * term_x + Mz1 * term_y + Mz2 * term_z + Const_imag_z;
 float r_xy = sqrt ( x * x + y * y );
 float R_xy = sqrt ( Rx * Rx + Ry * Ry );
 float z_out = ( r_xy * Rz + R_xy * z ) * Z_mul;
 float scale = 1.0f - ( z * Rz ) / ( r_xy * R_xy );
 float x_out = ( x * Rx - y * Ry ) * scale;
 float y_out = ( Rx * y + x * Ry ) * scale;
 return FormulaVec3 ( x_out , y_out , z_out );

}

}}
