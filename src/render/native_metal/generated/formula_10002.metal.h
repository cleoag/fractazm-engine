// Generated from Ads/_juliax2.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10002 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__JULIAX2 = 10002;
 template < > FormulaVec3 execute_formula < FORMULA__JULIAX2 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Separation = params [ 0 ];
 const float X_seed_L = params [ 1 ];
 const float Y_seed_L = params [ 2 ];
 const float Z_seed_L = params [ 3 ];
 const float X_seed_R = params [ 4 ];
 const float Y_seed_R = params [ 5 ];
 const float Z_seed_R = params [ 6 ];
 FormulaVec3 result = p;
 if ( ! ( p . x >= 0.0f ) ) {
 result . x = p . x + Separation;
 params [ WRITE_J1_SLOT ] = X_seed_L;
 params [ WRITE_J2_SLOT ] = Y_seed_L;
 params [ WRITE_J3_SLOT ] = Z_seed_L;

}
 else {
 result . x = p . x - Separation;
 params [ WRITE_J1_SLOT ] = X_seed_R;
 params [ WRITE_J2_SLOT ] = Y_seed_R;
 params [ WRITE_J3_SLOT ] = Z_seed_R;

}
 return result;

}

}}
