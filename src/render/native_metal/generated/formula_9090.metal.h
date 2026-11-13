// Generated from dIFS/PolyPyraIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_9090 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_POLYPYRAIFS = 9090;
 template < > FormulaVec3 execute_formula < FORMULA_POLYPYRAIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Height_fix = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float XZ_angle = params [ 5 ];
 const int apply_scale_add = int ( params [ 6 ] );
 const int PolyFold_Order = int ( params [ 7 ] );
 const float Cuts_fix = params [ 8 ];
 const float Initial_z_add = params [ 9 ];
 const float Star = params [ 10 ];
 const float INV_2PI = 0.159154943f;
 const float NEG_TWO_PI = - 6.283185307f;
 const float HALF_PI = 1.57079632675f;
 const float DEG2RAD = 0.017453293f;
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
 float raw_angle = atan2 ( x , y );
 float angle = HALF_PI - raw_angle;
 int n = int ( rint ( angle * INV_2PI * float ( PolyFold_Order ) ) );
 float sector = float ( n ) / float ( PolyFold_Order ) * NEG_TWO_PI;
 float cs = cos ( sector );
 float ss = sin ( sector );
 float radial = x * cs - y * ss;
 float perp = x * ss + y * cs;
 float projected = radial + abs ( perp * Star );
 float z_flipped = - ( z + Initial_z_add );
 float xz_rad = DEG2RAD * XZ_angle;
 float plane_dist = projected * sin ( xz_rad ) + z_flipped * cos ( xz_rad );
 float cap_dist = abs ( z - Cuts_fix ) - Height_fix;
 float shape_de = cap_dist >= plane_dist ? cap_dist : plane_dist;
 if ( apply_scale_add == 0 ) {
 shape_de = shape_de / Scale;

}
 const float iter_count = params [ ITER_COUNT_SLOT ];
 params [ MAX_FORMULA_PARAMS - 1 ] = fabs ( iter_count * params [ 11 ] + params [ 12 ] + float ( n ) * params [ 13 ] );
 de = shape_de;
 return pos;

}
 float PolyPyraIFS_shape_de ( FormulaVec3 p , FormulaParams params ) {
 const float Height_fix = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float XZ_angle = params [ 5 ];
 const int apply_scale_add = int ( params [ 6 ] );
 const int PolyFold_Order = int ( params [ 7 ] );
 const float Cuts_fix = params [ 8 ];
 const float Initial_z_add = params [ 9 ];
 const float Star = params [ 10 ];
 const float INV_2PI = 0.159154943f;
 const float NEG_TWO_PI = - 6.283185307f;
 const float HALF_PI = 1.57079632675f;
 const float DEG2RAD = 0.017453293f;
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float angle = HALF_PI - atan2 ( x , y );
 int n = int ( rint ( angle * INV_2PI * float ( PolyFold_Order ) ) );
 float sector = float ( n ) / float ( PolyFold_Order ) * NEG_TWO_PI;
 float cs = cos ( sector );
 float ss = sin ( sector );
 float radial = x * cs - y * ss;
 float perp = x * ss + y * cs;
 float projected = radial + abs ( perp * Star );
 float z_flipped = - ( z + Initial_z_add );
 float xz_rad = DEG2RAD * XZ_angle;
 float plane_dist = projected * sin ( xz_rad ) + z_flipped * cos ( xz_rad );
 float cap_dist = abs ( z - Cuts_fix ) - Height_fix;
 float shape_de = cap_dist >= plane_dist ? cap_dist : plane_dist;
 if ( apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 return shape_de;

}
 FormulaVec3 PolyPyraIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_POLYPYRAIFS > ( p , params , de );

}

}}
