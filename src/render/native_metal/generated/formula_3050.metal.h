// Generated from 3D/MagVsXYZmsw.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3050 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAGVSXYZMSW = 3050;
 template < > FormulaVec3 execute_formula < FORMULA_MAGVSXYZMSW > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
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
 if ( Cx >= 0.0f && x >= 0.0f ) n_x = - n_x;
 if ( Cy >= 0.0f && y >= 0.0f ) n_y = - n_y;
 if ( Cz >= 0.0f && z >= 0.0f ) n_z = - n_z;
 float theta = atan2 ( r , b * fabs ( x ) );
 float phi = atan2 ( r , b * fabs ( y ) );
 float tango = atan2 ( r , b * fabs ( z ) );
 float out_x = n_x * cos ( Power * theta + CxM ) + Cx * CxM;
 float out_y = n_y * cos ( Power * phi + CxM ) + Cy * CyM;
 float out_z = n_z * cos ( Power * tango + CxM ) + Cz * CzM;
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
