// Generated from dIFS/roomclrIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10065 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_ROOMCLRIFS = 10065;
 template < > FormulaVec3 execute_formula < FORMULA_ROOMCLRIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float RoomThick = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Pinch = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float X_halfwidth = params [ 7 ];
 const float Y_halfwidth = params [ 8 ];
 const float Z_halfwidth = params [ 9 ];
 const float OTClr_L = params [ 10 ];
 const float OTClr_R = params [ 11 ];
 const float OTClr = params [ 12 ];
 float state_x = p . x;
 float state_y = p . y;
 float state_z = p . z;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 state_x = x;
 state_y = y;
 state_z = z;

}
 float otrap;
 if ( z < 0.0f ) {
 otrap = OTClr_L;

}
 else {
 otrap = OTClr_R;

}
 float dz = fabs ( z ) - Z_halfwidth;
 float dy = fabs ( y ) - Y_halfwidth;
 float de_best;
 if ( dy >= dz ) {
 de_best = dy;
 otrap = OTClr;

}
 else {
 de_best = dz;

}
 float dx = fabs ( x ) - X_halfwidth;
 if ( dx >= de_best ) {
 de_best = dx;
 otrap = OTClr;

}
 params [ 63 ] = otrap;
 float r = native_formula_length3 ( x , y , z );
 float shape_raw = de_best + Pinch * r;
 float shape_de = fabs ( shape_raw ) - RoomThick;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( state_x , state_y , state_z );

}
 FormulaVec3 roomclrIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_ROOMCLRIFS > ( p , params , de );

}

}}
