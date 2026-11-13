// Generated from 3D/Benesi3Pow2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7085 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BENESI3POW2 = 7085;
 template < > FormulaVec3 execute_formula < FORMULA_BENESI3POW2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Cx = params [ 0 ];
 const float Cy = params [ 1 ];
 const float Cz = params [ 2 ];
 float x = p . x , y = p . y , z = p . z;
 float y2 = y * y;
 float z2 = z * z;
 float x2 = x * x;
 float rho2 = y2 + z2;
 float rho = sqrt ( rho2 );
 float base_x = x2 - rho2;
 if ( rho < x && ! signbit ( Cx ) ) {
 base_x = - base_x;

}
 float new_x = base_x + Cx;
 float factor;
 if ( rho == 0.0f && ( y != 0.0f || z != 0.0f ) ) {
 factor = 0.0f;

}
 else {
 factor = - 2.0f * fabs ( x ) / rho;

}
 float new_y = factor * ( y2 - z2 ) + Cy;
 float new_z = factor * y * z * 2.0f + Cz;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
