// Generated from Ads/_ngon.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7081 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__NGON = 7081;
 template < > FormulaVec3 execute_formula < FORMULA__NGON > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Order = params [ 0 ];
 const float Shift_deg = params [ 1 ];
 const float Shift_x = params [ 2 ];
 const float Shift_y = params [ 3 ];
 const float Skew_deg = params [ 4 ];
 const int No_yfold = native_int32_parameter ( params , 5 );
 const float TWOPI = 6.283185307f;
 const float DEG2RAD = 0.017453293f;
 float xs = p . x + Shift_x;
 float ys = p . y + Shift_y;
 float angle = atan2 ( ys , xs );
 float combined = angle + ( Skew_deg + Shift_deg ) * DEG2RAD;
 float sector_value = combined / TWOPI * Order;
 int sector_i = 0;
 float sector = native_fistp_sector ( sector_value , sector_i );
 float rot = - sector / Order * TWOPI + Shift_deg * DEG2RAD;
 float c = cos ( rot );
 float s = sin ( rot );
 float rot_x = ys * s - xs * c;
 float y_out;
 if ( No_yfold != 0 ) {
 y_out = sector_value;

}
 else {
 y_out = sector_value - sector;
 if ( ( sector_i & 1 ) == 0 ) {
 y_out = - y_out;

}

}
 float new_y = y_out / Order * TWOPI - Shift_y;
 float new_x = rot_x - Shift_x;
 return FormulaVec3 ( new_x , new_y , p . z );

}

}}
