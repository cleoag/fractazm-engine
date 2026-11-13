// Generated from dIFS/PiriformIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10196 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_PIRIFORMIFS = 10196;
 template < > FormulaVec3 execute_formula < FORMULA_PIRIFORMIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int apply = int ( params [ 6 ] ) , curve = int ( params [ 7 ] );
 const FormulaVec3 local ( p . x * params [ 1 ] + params [ 4 ] , p . y * params [ 1 ] + params [ 3 ] , p . z * params [ 1 ] + params [ 2 ] );
 if ( apply ) params [ DIFS_VARYSCALE_SLOT ] *= params [ 1 ];
 float x = local . x , z = local . z;
 if ( int ( params [ 9 ] ) ) {
 const float t = x;
 x = z;
 z = t;

}
 x = int ( params [ 10 ] ) ? fabs ( x ) : - x;
 const numeric :: X87Scope extendedScope;
 numeric :: X87Real u = ( numeric :: X87Real ) x * x;
 if ( curve == 0 ) u *= x;
 if ( curve != 2 ) u *= x;
 const numeric :: X87Real extra = u * x;
 const float storedU = float ( u );
 const numeric :: X87Real v1 = ( ( extra + ( numeric :: X87Real ) local . y * local . y ) + ( numeric :: X87Real ) z * z ) + params [ 5 ];
 const numeric :: X87Real v2 = ( numeric :: X87Real ) storedU + params [ 0 ];
 numeric :: X87Real distance = numeric :: x87Sqrt ( ( v1 + numeric :: x87Abs ( v1 ) ) * params [ 11 ] ) - numeric :: x87Sqrt ( ( v2 + numeric :: x87Abs ( v2 ) ) * params [ 11 ] );
 if ( ! ( params [ 8 ] <= 0 ) ) {
 const numeric :: X87Real limit = ( numeric :: X87Real ) fabs ( x ) - params [ 8 ];
 if ( limit >= distance ) distance = limit;

}
 if ( ! apply ) distance /= params [ 1 ];
 de = float ( distance );
 return apply ? local : p;

}

}}
