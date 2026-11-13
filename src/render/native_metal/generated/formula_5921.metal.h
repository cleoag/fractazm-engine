// Generated from 3D/CosinePow8.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5921 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_COSINEPOW8 = 5921;
 template < > FormulaVec3 execute_formula < FORMULA_COSINEPOW8 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Z_multiplier = params [ 0 ];
 const float Cx = params [ 6 ];
 const float Cy = params [ 7 ];
 const float Cz = params [ 8 ];
 float x = p . x , y = p . y , z = p . z;
 float x2 = x * x;
 float y2 = y * y;
 float z2 = z * z;
 float rxy2 = x2 + y2;
 float z4 = z2 * z2;
 float rxy4 = rxy2 * rxy2;
 float x4 = x2 * x2;
 float y4 = y2 * y2;
 float z_poly = z4 * z4 + rxy4 * rxy4 + 70.0f * z4 * rxy4 - 28.0f * rxy2 * z2 * ( rxy4 + z4 );
 float new_z = Z_multiplier * z_poly + Cz;
 float z6 = z2 * z4;
 float rxy6 = rxy2 * rxy4;
 float Q2 = z6 - 7.0f * z4 * rxy2 + 7.0f * z2 * rxy4 - rxy6;
 float rxy = sqrt ( rxy2 );
 float rxy7 = rxy4 * rxy2 * rxy;
 float ratio = ( 8.0f * z * Q2 ) / ( rxy7 + 1e-40f );
 float P = x4 * ( x2 - 7.0f * y2 ) + y4 * ( 7.0f * x2 - y2 );
 float new_y = 8.0f * P * x * y * ratio + Cy;
 float x2y2 = x2 * y2;
 float S1 = x4 * x4 + y4 * y4 + 70.0f * x4 * y4 - 28.0f * x2y2 * ( x4 + y4 );
 float new_x = ratio * S1 + Cx;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
