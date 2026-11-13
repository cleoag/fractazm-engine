// Generated from dIFS/PenN-gonIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_1446 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_PENN_GONIFS = 1446;
 template < > FormulaVec3 execute_formula < FORMULA_PENN_GONIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Radius_1 = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Radius_2 = params [ 5 ];
 const int apply_scale_add = int ( params [ 6 ] );
 const int Poly_Order = int ( params [ 7 ] );
 const int Mobius = int ( params [ 8 ] );
 const float Atan_add = params [ 9 ];
 const float INV_2PI = params [ 10 ];
 const float TWO_PI = params [ 11 ];
 const float HALF_PI = params [ 12 ];
 const float QUARTER = params [ 13 ];
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
 float mobius_angle = raw_angle * QUARTER * float ( Mobius ) + Atan_add;
 float cos_mob = cos ( mobius_angle );
 float sin_mob = sin ( mobius_angle );
 float polyfold_angle = HALF_PI - raw_angle;
 float sector = rint ( polyfold_angle * INV_2PI * float ( Poly_Order ) );
 sector = sector / float ( Poly_Order ) * TWO_PI;
 float cs = cos ( sector );
 float ss = sin ( sector );
 float d_radial = abs ( x * cs + y * ss ) - Radius_1;
 float dist1 = abs ( d_radial * sin_mob - z * cos_mob );
 float dist2 = abs ( d_radial * cos_mob + z * sin_mob );
 float shape_de = max ( dist1 , dist2 ) - Radius_2;
 if ( apply_scale_add == 0 ) {
 shape_de = shape_de / Scale;

}
 de = shape_de;
 return pos;

}
 float PenNgonIFS_shape_de ( FormulaVec3 p , FormulaParams params ) {
 const float Radius_1 = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Radius_2 = params [ 5 ];
 const int apply_scale_add = int ( params [ 6 ] );
 const int Poly_Order = int ( params [ 7 ] );
 const int Mobius = int ( params [ 8 ] );
 const float Atan_add = params [ 9 ];
 const float INV_2PI = params [ 10 ];
 const float TWO_PI = params [ 11 ];
 const float HALF_PI = params [ 12 ];
 const float QUARTER = params [ 13 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 float raw_angle = atan2 ( x , y );
 float mobius_angle = raw_angle * QUARTER * float ( Mobius ) + Atan_add;
 float cos_mob = cos ( mobius_angle );
 float sin_mob = sin ( mobius_angle );
 float polyfold_angle = HALF_PI - raw_angle;
 float sector = rint ( polyfold_angle * INV_2PI * float ( Poly_Order ) );
 sector = sector / float ( Poly_Order ) * TWO_PI;
 float cs = cos ( sector );
 float ss = sin ( sector );
 float d_radial = abs ( x * cs + y * ss ) - Radius_1;
 float dist1 = abs ( d_radial * sin_mob - z * cos_mob );
 float dist2 = abs ( d_radial * cos_mob + z * sin_mob );
 float shape_de = max ( dist1 , dist2 ) - Radius_2;
 if ( apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 return shape_de;

}
 FormulaVec3 PenN_gonIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_PENN_GONIFS > ( p , params , de );

}

}}
