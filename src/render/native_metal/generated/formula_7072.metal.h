// Generated from 3D/MsltoeSym3.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7072 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MSLTOESYM3 = 7072;
 template < > FormulaVec3 execute_formula < FORMULA_MSLTOESYM3 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Y_mul = params [ 0 ];
 float x = p . x , y = p . y , z = p . z;
 float r2 = x * x + y * y + z * z;
 if ( abs ( y ) <= abs ( z ) ) {
 float t = y;
 y = z;
 z = t;

}
 if ( y >= z ) {
 x = - x;
 z = - z;

}
 float T = 1.0f - z * z / r2;
 float x2 = x * x;
 float y2 = y * y;
 float new_x = ( x2 - y2 ) * T;
 float S = sqrt ( y2 + x2 );
 float new_z = 2.0f * z * S;
 float new_y = 2.0f * y * T * x * Y_mul;
 return FormulaVec3 ( new_x , new_y , new_z );

}

}}
