// Generated from 3D/PseudoXDB.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7025 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_PSEUDOXDB = 7025;
 template < > FormulaVec3 execute_formula < FORMULA_PSEUDOXDB > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Zmul = params [ 0 ];
 float x = p . x , y = p . y , z = p . z;
 float az = abs ( z );
 float x2 = x * x;
 float y2 = y * y;
 float R = sqrt ( x2 + y2 );
 float K = 2.0f * az / R;
 float new_x = K * ( x2 - y2 );
 float new_y = 2.0f * K * x * y;
 float new_z = ( az * az - x2 - y2 ) * Zmul;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
