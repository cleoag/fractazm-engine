// Generated from dIFS/cubetubeIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10302 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_CUBETUBEIFS = 10302;
 template < > FormulaVec3 execute_formula < FORMULA_CUBETUBEIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int apply = int ( params [ 11 ] );
 const FormulaVec3 local ( p . x * params [ 3 ] + params [ 6 ] , p . y * params [ 3 ] + params [ 5 ] , p . z * params [ 3 ] + params [ 4 ] );
 if ( apply != 0 ) params [ DIFS_VARYSCALE_SLOT ] *= params [ 3 ];
 const numeric :: X87Scope extendedScope;
 numeric :: X87Real a = fabs ( local . x ) , b = fabs ( local . z ) , c = fabs ( local . y ) , t;
 if ( ! ( a > b ) ) {
 t = a;
 a = b;
 b = t;

}
 if ( ! ( a > c ) ) {
 t = a;
 a = c;
 c = t;

}
 t = a;
 a = b;
 b = t;
 if ( ! ( a > c ) ) {
 t = a;
 a = c;
 c = t;

}
 float x = float ( c ) , y = float ( b ) , z = float ( a );
 float first = float ( ( numeric :: X87Real ) x * params [ 7 ] + ( numeric :: X87Real ) z * params [ 8 ] );
 x = float ( ( numeric :: X87Real ) x * params [ 8 ] - ( numeric :: X87Real ) z * params [ 7 ] );
 z = first;
 first = float ( ( numeric :: X87Real ) x * params [ 9 ] + ( numeric :: X87Real ) y * params [ 10 ] );
 x = float ( ( numeric :: X87Real ) x * params [ 10 ] - ( numeric :: X87Real ) y * params [ 9 ] );
 y = first;
 const numeric :: X87Real dy = ( numeric :: X87Real ) y - params [ 1 ] , dz = ( numeric :: X87Real ) z - params [ 2 ];
 y = float ( dy );
 z = float ( dz );
 const numeric :: X87Real tube = numeric :: x87Sqrt ( dy * dy + dz * dz ) - params [ 0 ];
 const numeric :: X87Real dx = numeric :: x87Abs ( ( numeric :: X87Real ) x ) - params [ 12 ];
 const numeric :: X87Real sphere = numeric :: x87Sqrt ( ( dx * dx + ( numeric :: X87Real ) y * y ) + ( numeric :: X87Real ) z * z ) - params [ 13 ];
 const bool selectSphere = ! ( sphere > tube );
 params [ MAX_FORMULA_PARAMS - 1 ] = params [ selectSphere ? 15 : 14 ];
 numeric :: X87Real distance = selectSphere ? sphere : tube;
 if ( apply == 0 ) distance /= params [ 3 ];
 de = float ( distance );
 return apply != 0 ? local : p;

}

}}
