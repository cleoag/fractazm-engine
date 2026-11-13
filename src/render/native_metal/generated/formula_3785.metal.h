// Generated from dIFS/helixIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3785 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_HELIXIFS = 3785;
 template < > FormulaVec3 execute_formula < FORMULA_HELIXIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Helix_stretch = params [ 0 ];
 const float Diameter_1 = params [ 1 ];
 const float Diameter_2 = params [ 2 ];
 const float Scale = params [ 3 ];
 const float Z_add = params [ 4 ];
 const float Y_add = params [ 5 ];
 const float X_add = params [ 6 ];
 const float String_radius = params [ 7 ];
 const int apply_scale_add = int ( params [ 8 ] );
 const int Coord = int ( params [ 9 ] );
 const int Helix_count = int ( params [ 10 ] );
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 FormulaVec3 pos;
 if ( apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;
 pos = FormulaVec3 ( x , y , z );

}
 else {
 pos = p;

}
 float axis_coord;
 float rad1 , rad2;
 if ( Coord == 0 ) {
 axis_coord = x;
 rad1 = y;
 rad2 = z;

}
 else if ( Coord == 1 ) {
 axis_coord = y;
 rad1 = x;
 rad2 = z;

}
 else {
 axis_coord = z;
 rad1 = y;
 rad2 = x;

}
 float angle_step;
 int count = ( Helix_count - 1 ) & 0xF;
 if ( count <= 1 ) {
 angle_step = M_PI;

}
 else {
 angle_step = 2.0f * M_PI / float ( count + 1 );

}
 float helix_angle = axis_coord * Helix_stretch;
 float min_dist = 10000.0f;
 for ( int k = count;
 k >= 0;
 k -- ) {
 float c = cos ( helix_angle );
 float s = sin ( helix_angle );
 float center1 = c * Diameter_2;
 float center2 = s * Diameter_1;
 float d1 = rad1 - center1;
 float d2 = rad2 - center2;
 float dist = sqrt ( d1 * d1 + d2 * d2 );
 min_dist = min ( min_dist , dist );
 helix_angle += angle_step;

}
 float shape_de = min_dist - String_radius;
 if ( apply_scale_add == 0 ) {
 shape_de = shape_de / Scale;

}
 de = shape_de;
 return pos;

}
 float helixIFS_shape_de ( FormulaVec3 p , FormulaParams params ) {
 const float Helix_stretch = params [ 0 ];
 const float Diameter_1 = params [ 1 ];
 const float Diameter_2 = params [ 2 ];
 const float Scale = params [ 3 ];
 const float X_add = params [ 6 ];
 const float Y_add = params [ 5 ];
 const float Z_add = params [ 4 ];
 const float String_radius = params [ 7 ];
 const int apply_scale_add = int ( params [ 8 ] );
 const int Coord = int ( params [ 9 ] );
 const int Helix_count = int ( params [ 10 ] );
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float axis_coord , rad1 , rad2;
 if ( Coord == 0 ) {
 axis_coord = x;
 rad1 = y;
 rad2 = z;

}
 else if ( Coord == 1 ) {
 axis_coord = y;
 rad1 = x;
 rad2 = z;

}
 else {
 axis_coord = z;
 rad1 = y;
 rad2 = x;

}
 int count = ( Helix_count - 1 ) & 0xF;
 float angle_step = ( count <= 1 ) ? M_PI : 2.0f * M_PI / float ( count + 1 );
 float helix_angle = axis_coord * Helix_stretch;
 float min_dist = 10000.0f;
 for ( int k = count;
 k >= 0;
 k -- ) {
 float c = cos ( helix_angle );
 float s = sin ( helix_angle );
 float d1 = rad1 - c * Diameter_2;
 float d2 = rad2 - s * Diameter_1;
 min_dist = min ( min_dist , sqrt ( d1 * d1 + d2 * d2 ) );
 helix_angle += angle_step;

}
 float shape_de = min_dist - String_radius;
 if ( apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 return shape_de;

}
 FormulaVec3 helixIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_HELIXIFS > ( p , params , de );

}

}}
