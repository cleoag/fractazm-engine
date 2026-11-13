// Generated from Ads/_TranslateC4d.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10001 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__TRANSLATEC4D = 10001;
 template < > FormulaVec3 execute_formula < FORMULA__TRANSLATEC4D > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 ( void ) de;
 const float x_add = params [ 0 ];
 const float y_add = params [ 1 ];
 const float z_add = params [ 2 ];
 const float w_add = params [ 3 ];
 const float cx_add = params [ 4 ];
 const float cy_add = params [ 5 ];
 const float cz_add = params [ 6 ];
 const float cw_add = params [ 7 ];
 const float cx_in = params [ 8 ];
 const float cy_in = params [ 9 ];
 const float cz_in = params [ 10 ];
 const float cw_in = params [ 11 ];
 const float cx_out = cx_in + cx_add;
 const float cy_out = cy_in + cy_add;
 const float cz_out = cz_in + cz_add;
 const float cw_out = cw_in + cw_add;
 params [ WRITE_J1_SLOT ] = cx_out;
 params [ WRITE_J2_SLOT ] = cy_out;
 params [ WRITE_J3_SLOT ] = cz_out;
 params [ WRITE_J4_SLOT ] = cw_out;
 params [ FORMULA_STATE_W_SLOT ] = params [ FORMULA_STATE_W_SLOT ] + w_add;
 return FormulaVec3 ( p . x + x_add , p . y + y_add , p . z + z_add );

}

}}
