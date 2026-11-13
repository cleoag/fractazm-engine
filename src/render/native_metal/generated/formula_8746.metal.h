// Generated from 3D/Benesi2Pow6.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_8746 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BENESI2POW6 = 8746;
 template < > FormulaVec3 execute_formula < FORMULA_BENESI2POW6 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float coeff = params [ 0 ];
 const float Cx = params [ 1 ];
 const float Cy = params [ 2 ];
 const float Cz = params [ 3 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float rxy2 = x2 + y2;
 float B = x2 - y2 - z2;
 float C_term = B * B - coeff * x2 * ( y2 + z2 );
 float new_x = B * C_term + Cx;
 float E = rxy2 - z2;
 float rxy = sqrt ( rxy2 );
 float F = 2.0f * z * rxy;
 float E2 = E * E;
 float F2 = F * F;
 float K1 = E * ( E2 - 3.0f * F2 );
 float K2 = F * ( 3.0f * E2 - F2 );
 float L = x2 - y2;
 float N = 2.0f * x * y;
 float P = L * L - N * N;
 float Q = 2.0f * N * L;
 float S = N * P + L * Q;
 float rxy6 = rxy2 * rxy2 * rxy2;
 float sin6theta;
 if ( rxy6 == 0.0f && ( x != 0.0f || y != 0.0f ) ) {
 sin6theta = 0.0f;

}
 else {
 sin6theta = S / rxy6;

}
 float new_y = fabs ( Cy ) - fabs ( K2 );
 float new_z = fabs ( Cz ) - fabs ( sin6theta * K1 );
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
