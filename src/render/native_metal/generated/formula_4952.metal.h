// Generated from dIFS/TorusEllipIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4952 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TORUSELLIPIFS = 4952;
 template < > FormulaVec3 execute_formula < FORMULA_TORUSELLIPIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Radius_1 = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Radius_2 = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const int Boxy_XY = int ( params [ 7 ] );
 const int Boxy_Z = int ( params [ 8 ] );
 const float Elliptic = params [ 9 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 float r_xy;
 if ( Boxy_XY == 0 ) {
 r_xy = sqrt ( x * x + y * y );

}
 else {
 r_xy = max ( abs ( x ) , abs ( y ) );

}
 float ring_dist = r_xy - Radius_1;
 float tube_dist;
 if ( Boxy_Z == 0 ) {
 tube_dist = sqrt ( ring_dist * ring_dist + z * z );

}
 else {
 tube_dist = max ( abs ( ring_dist ) , abs ( z ) );

}
 float shape_de = ( tube_dist - Radius_2 ) + x * Elliptic;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( x , y , z );

}
 FormulaVec3 TorusEllipIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_TORUSELLIPIFS > ( p , params , de );

}

}}
