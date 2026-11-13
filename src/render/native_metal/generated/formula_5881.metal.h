// Generated from 3D/MagVsXYZabs3.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5881 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAGVSXYZABS3 = 5881;
 template < > FormulaVec3 execute_formula < FORMULA_MAGVSXYZABS3 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Power = params [ 0 ];
 const float Axiom_1 = params [ 1 ];
 const float Axiom_2 = params [ 2 ];
 const float CxM = params [ 3 ];
 const float CyM = params [ 4 ];
 const float CzM = params [ 5 ];
 const float Cx = params [ 7 ];
 const float Cy = params [ 8 ];
 const float Cz = params [ 9 ];
 float x = p . x , y = p . y , z = p . z;
 float a = Axiom_2;
 float b = Power * Axiom_1;
 float r_sq = x * x + y * y + z * z;
 float r = sqrt ( r_sq );
 float n_x = sqrt ( a * x * x + r_sq );
 float n_y = sqrt ( a * y * y + r_sq );
 float n_z = sqrt ( a * z * z + r_sq );
 n_x = x87_log_power ( fabs ( n_x ) , Power );
 n_y = x87_log_power ( fabs ( n_y ) , Power );
 n_z = x87_log_power ( fabs ( n_z ) , Power );
 float theta = atan2 ( r , b * fabs ( x ) );
 float phi = atan2 ( r , b * fabs ( y ) );
 float tango = atan2 ( r , b * fabs ( z ) );
 float cos_x = n_x * cos ( Power * theta + CxM );
 float cos_y = n_y * cos ( Power * phi + CxM );
 float cos_z = n_z * cos ( Power * tango + CxM );
 float out_x = ( x >= 0.0f ? - cos_x : cos_x ) + Cx * CxM;
 float out_y = ( y >= 0.0f ? - cos_y : cos_y ) + Cy * CyM;
 float out_z = ( z >= 0.0f ? - cos_z : cos_z ) + Cz * CzM;
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
