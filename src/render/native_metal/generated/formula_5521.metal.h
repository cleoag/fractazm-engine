// Generated from dIFS/cylinderIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_5521 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_CYLINDERIFS = 5521;
 template < > FormulaVec3 execute_formula < FORMULA_CYLINDERIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Radius = params [ 0 ];
 const float Halfwidth = params [ 1 ];
 const float Scale = params [ 2 ];
 const float Z_add = params [ 3 ];
 const float Y_add = params [ 4 ];
 const float X_add = params [ 5 ];
 const float Barrel = params [ 6 ];
 const float Tip = params [ 7 ];
 const int apply_scale_add = int ( params [ 8 ] );
 const float Inside_radius = params [ 12 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 FormulaVec3 pos;
 if ( apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 pos = FormulaVec3 ( x , y , z );

}
 else {
 pos = p;

}
 float ax = abs ( x );
 float radial = sqrt ( y * y + z * z );
 float cap_dist = ax - Halfwidth;
 float clamped_x = min ( ax , Halfwidth );
 float barrel_mod = clamped_x * clamped_x * Barrel;
 float tip_mod = x * Tip;
 float outer_dist = ( radial - Radius ) + barrel_mod + tip_mod;
 float inner_dist = ( Inside_radius - radial ) - barrel_mod - tip_mod;
 float shape_de = max ( max ( outer_dist , inner_dist ) , cap_dist );
 if ( apply_scale_add == 0 ) {
 shape_de = shape_de / Scale;

}
 de = shape_de;
 float source = params [ MAX_FORMULA_PARAMS - 2 ];
 int otrap_opt = int ( params [ 9 ] ) & 3;
 if ( otrap_opt == 1 ) source = pos . x;
 else if ( otrap_opt == 2 ) source = pos . y;
 else if ( otrap_opt == 3 ) source = pos . z;
 params [ MAX_FORMULA_PARAMS - 1 ] = source * params [ 11 ] + params [ 10 ];
 return pos;

}
 float cylinderIFS_shape_de ( FormulaVec3 p , FormulaParams params ) {
 const float Radius = params [ 0 ];
 const float Halfwidth = params [ 1 ];
 const float Scale = params [ 2 ];
 const float Z_add = params [ 3 ];
 const float Y_add = params [ 4 ];
 const float X_add = params [ 5 ];
 const float Barrel = params [ 6 ];
 const float Tip = params [ 7 ];
 const int apply_scale_add = int ( params [ 8 ] );
 const float Inside_radius = params [ 12 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float ax = abs ( x );
 float radial = sqrt ( y * y + z * z );
 float cap_dist = ax - Halfwidth;
 float clamped_x = min ( ax , Halfwidth );
 float barrel_mod = clamped_x * clamped_x * Barrel;
 float tip_mod = x * Tip;
 float outer_dist = ( radial - Radius ) + barrel_mod + tip_mod;
 float inner_dist = ( Inside_radius - radial ) - barrel_mod - tip_mod;
 float shape_de = max ( max ( outer_dist , inner_dist ) , cap_dist );
 if ( apply_scale_add == 0 ) {
 shape_de = shape_de / Scale;

}
 return shape_de;

}
 FormulaVec3 cylinderIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Radius = params [ 0 ];
 const float Halfwidth = params [ 1 ];
 const float Scale = params [ 2 ];
 const float Z_add = params [ 3 ];
 const float Y_add = params [ 4 ];
 const float X_add = params [ 5 ];
 const float Barrel = params [ 6 ];
 const float Tip = params [ 7 ];
 const int apply_scale_add = int ( params [ 8 ] );
 const float Inside_radius = params [ 12 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 FormulaVec3 pos;
 if ( apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;
 pos = FormulaVec3 ( x , y , z );

}
 else {
 pos = p;

}
 float ax = abs ( x );
 float radial = sqrt ( y * y + z * z );
 float cap_dist = ax - Halfwidth;
 float clamped_x = min ( ax , Halfwidth );
 float barrel_mod = clamped_x * clamped_x * Barrel;
 float tip_mod = x * Tip;
 float outer_dist = ( radial - Radius ) + barrel_mod + tip_mod;
 float inner_dist = ( Inside_radius - radial ) - barrel_mod - tip_mod;
 float shape_de = max ( max ( outer_dist , inner_dist ) , cap_dist );
 if ( apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return pos;

}

}}
