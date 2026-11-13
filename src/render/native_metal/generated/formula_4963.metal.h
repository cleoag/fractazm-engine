// Generated from dIFS/boxIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_4963 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_BOXIFS = 4963;
 template < > FormulaVec3 execute_formula_dIFS < FORMULA_BOXIFS > ( FormulaVec3 p , FormulaParams params , thread float & varyScale , thread float & rout ) {
 const float Z_halfwidth = params [ 0 ];
 const float Y_halfwidth = params [ 1 ];
 const float X_halfwidth = params [ 2 ];
 const float Scale = params [ 3 ];
 const float Z_add = params [ 4 ];
 const float Y_add = params [ 5 ];
 const float X_add = params [ 6 ];
 const float Trapez = params [ 7 ];
 const float Twist = params [ 8 ];
 const int Apply_scale_add = int ( params [ 9 ] );
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 varyScale *= Scale;
 params [ DIFS_COMMIT_XYZ_SLOT ] = 1.0f;

}
 else {
 params [ DIFS_COMMIT_XYZ_SLOT ] = 0.0f;

}
 float x_tw = x , y_tw = y;
 if ( native_std :: bit_cast < uint > ( float ( Twist ) ) != 0 ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const auto sc = extendedSinCos ( E ( float ( Twist ) ) * z );
 x_tw = float ( sc . sin * E ( x ) + E ( y ) * sc . cos );
 y_tw = float ( E ( x ) * sc . cos - E ( y ) * sc . sin );

}
 float z_trapez = z * Trapez;
 float dx = abs ( x_tw ) - X_halfwidth - z_trapez;
 float dy = abs ( y_tw ) - Y_halfwidth;
 float dz = abs ( z ) - Z_halfwidth;
 float shape_de = dz > dx ? dz : dx;
 shape_de = shape_de > dy ? shape_de : dy;
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 rout = shape_de;
 {
 int otrap_opt = int ( params [ 10 ] ) & 3;
 float otrap_scale = static_cast < float > ( params [ 12 ] );
 float otrap_offset = static_cast < float > ( params [ 11 ] );
 float source;
 switch ( otrap_opt ) {
 default : case 0 : source = params [ MAX_FORMULA_PARAMS - 2 ];
 break;
 case 1 : source = x;
 break;
 case 2 : source = y;
 break;
 case 3 : source = z;
 break;

}
 if ( ! Apply_scale_add && otrap_opt != 0 ) source = otrap_opt == 1 ? p . x : otrap_opt == 2 ? p . y : p . z;
 numeric :: ExtendedScope scope;
 params [ MAX_FORMULA_PARAMS - 1 ] = float ( numeric :: Extended80 ( source ) * otrap_scale + otrap_offset );

}
 return FormulaVec3 ( x , y , z );

}
 template < > FormulaVec3 execute_formula < FORMULA_BOXIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float vs = 1.0f , rout = 0.0f;
 auto r = execute_formula_dIFS < FORMULA_BOXIFS > ( p , params , vs , rout );
 de = rout;
 return r;

}
 FormulaVec3 boxIFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_BOXIFS > ( p , params , de );

}

}}
