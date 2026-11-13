// Generated from dIFS/YinYangIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10218 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_YINYANGIFS = 10218;
 template < > FormulaVec3 execute_formula < FORMULA_YINYANGIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Radius1 = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const float Radius2 = params [ 5 ];
 const int Apply_scale_add = int ( params [ 6 ] );
 const float RadiusSpheres = params [ 7 ];
 const float M0 = params [ 8 ];
 const float M1 = params [ 9 ];
 const float M2 = params [ 10 ];
 const float M3 = params [ 11 ];
 const float M4 = params [ 12 ];
 const float M5 = params [ 13 ];
 const float M6 = params [ 14 ];
 const float M7 = params [ 15 ];
 const float M8 = params [ 16 ];
 const float neg_one = params [ 17 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 float rx = x * M0 + y * M1 + z * M2;
 float ry = x * M3 + y * M4 + z * M5;
 float rz = x * M6 + y * M7 + z * M8;
 float r_xy = sqrt ( rx * rx + ry * ry );
 float ring1 = r_xy - 2.0f * Radius1;
 float torus1_de = sqrt ( ring1 * ring1 + rz * rz ) - Radius2;
 float sphere_de = sqrt ( ( abs ( rx ) + Radius1 * neg_one ) * ( abs ( rx ) + Radius1 * neg_one ) + ry * ry + rz * rz ) - RadiusSpheres;
 float current_de = min ( torus1_de , sphere_de );
 float adjusted_rx;
 if ( ry < 0.0f ) {
 adjusted_rx = rx + Radius1;

}
 else {
 adjusted_rx = rx - Radius1;

}
 float r_xy2 = sqrt ( adjusted_rx * adjusted_rx + ry * ry );
 float ring2 = r_xy2 - Radius1;
 float torus2_de = sqrt ( ring2 * ring2 + rz * rz ) - Radius2;
 float shape_de = min ( current_de , torus2_de );
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = shape_de;
 return FormulaVec3 ( x , y , z );

}
 FormulaVec3 YinYangIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_YINYANGIFS > ( p , params , de );

}

}}
