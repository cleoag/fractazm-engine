// Generated from dIFS/foldcut8-IFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10072 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_FOLDCUT8IFS = 10072;
 template < > FormulaVec3 execute_formula < FORMULA_FOLDCUT8IFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float Size = params [ 0 ];
 const float Scale = params [ 1 ];
 const float Z_add = params [ 2 ];
 const float Y_add = params [ 3 ];
 const float X_add = params [ 4 ];
 const int Apply_scale_add = int ( params [ 5 ] );
 const float R = params [ 6 ];
 const float Normal_1 = params [ 7 ];
 const float Normal_2 = params [ 8 ];
 const float Normal_3 = params [ 9 ];
 float x = p . x * Scale + X_add;
 float y = p . y * Scale + Y_add;
 float z = p . z * Scale + Z_add;
 if ( Apply_scale_add != 0 ) {
 params [ DIFS_VARYSCALE_SLOT ] *= Scale;

}
 const numeric :: X87Scope extendedScope;
 float ax = abs ( x );
 float ay = abs ( y );
 float az = abs ( z );
 float v0 = az , v1 = ay , v2 = ax;
 if ( v0 < v1 ) {
 float t = v0;
 v0 = v1;
 v1 = t;

}
 if ( v0 < v2 ) {
 float t = v0;
 v0 = v2;
 v2 = t;

}
 if ( v1 < v2 ) {
 float t = v1;
 v1 = v2;
 v2 = t;

}
 numeric :: X87Real weighted = ( static_cast < numeric :: X87Real > ( v1 ) * Normal_3 + static_cast < numeric :: X87Real > ( v0 ) * Normal_1 ) + static_cast < numeric :: X87Real > ( v2 ) * Normal_2;
 numeric :: X87Real plane_de = weighted - Size;
 numeric :: X87Real r = sqrt ( ( static_cast < numeric :: X87Real > ( x ) * x + static_cast < numeric :: X87Real > ( y ) * y ) + static_cast < numeric :: X87Real > ( z ) * z );
 float abs_R = abs ( R );
 numeric :: X87Real sphere_de = r - abs_R;
 numeric :: X87Real shape_de;
 if ( ! metal :: signbit ( R ) ) {
 shape_de = ! ( sphere_de > plane_de ) ? sphere_de : plane_de;

}
 else {
 shape_de = ! ( sphere_de > plane_de ) ? plane_de : sphere_de;

}
 if ( Apply_scale_add == 0 ) {
 shape_de /= Scale;

}
 de = float ( shape_de );
 return Apply_scale_add ? FormulaVec3 ( x , y , z ) : p;

}
 FormulaVec3 foldcut8IFS_formula ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 return execute_formula < FORMULA_FOLDCUT8IFS > ( p , params , de );

}

}}
