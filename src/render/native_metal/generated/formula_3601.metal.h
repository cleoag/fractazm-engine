// Generated from 3D/MagVsXYZabs.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3601 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MAGVSXYZABS = 3601;
 template < > FormulaVec3 execute_formula < FORMULA_MAGVSXYZABS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Power = params [ 0 ];
 const float Axiom_1 = params [ 1 ];
 const float Axiom_2 = params [ 2 ];
 const float CxM = params [ 3 ];
 const float CyM = params [ 4 ];
 const float CzM = params [ 5 ];
 const float Cx = params [ 8 ];
 const float Cy = params [ 9 ];
 const float Cz = params [ 10 ];
 float x = p . x , y = p . y , z = p . z;
 const float qnan = NAN;
 float a = Axiom_2;
 float b = Power * Axiom_1;
 float r_sq = x * x + y * y + z * z;
 if ( r_sq == 0.0f || ! metal :: isfinite ( r_sq ) ) {
 return FormulaVec3 ( qnan , qnan , qnan );

}
 float r = sqrt ( r_sq );
 if ( ! metal :: isfinite ( r ) ) {
 return FormulaVec3 ( qnan , qnan , qnan );

}
 float n_x = sqrt ( a * x * x + r_sq );
 float n_y = sqrt ( a * y * y + r_sq );
 float n_z = sqrt ( a * z * z + r_sq );
 n_x = pow ( n_x , Power );
 n_y = pow ( n_y , Power );
 n_z = pow ( n_z , Power );
 if ( ! metal :: isfinite ( n_x ) || ! metal :: isfinite ( n_y ) || ! metal :: isfinite ( n_z ) ) {
 return FormulaVec3 ( qnan , qnan , qnan );

}
 float theta = atan2 ( r , b * fabs ( x ) );
 float phi = atan2 ( r , b * fabs ( y ) );
 float tango = atan2 ( r , b * fabs ( z ) );
 if ( ! metal :: isfinite ( theta ) || ! metal :: isfinite ( phi ) || ! metal :: isfinite ( tango ) ) {
 return FormulaVec3 ( qnan , qnan , qnan );

}
 float out_x = fabs ( n_x * cos ( Axiom_1 * theta + CxM ) + Cx * CxM );
 float out_y = fabs ( n_y * cos ( Axiom_1 * phi + CxM ) + Cy * CyM );
 float out_z = fabs ( n_z * cos ( Axiom_1 * tango + CxM ) + Cz * CzM );
 if ( ! metal :: isfinite ( out_x ) || ! metal :: isfinite ( out_y ) || ! metal :: isfinite ( out_z ) ) {
 return FormulaVec3 ( qnan , qnan , qnan );

}
 return FormulaVec3 ( out_x , out_y , out_z );

}

}}
