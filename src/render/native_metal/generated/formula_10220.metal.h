// Generated from dIFS/columnsIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10220 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_COLUMNSIFS = 10220;
 template < > FormulaVec3 execute_formula < FORMULA_COLUMNSIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const int apply = int ( params [ 12 ] ) , iterations = int ( params [ 8 ] );
 const FormulaVec3 local ( p . x * params [ 3 ] + params [ 6 ] , p . y * params [ 3 ] + params [ 5 ] , p . z * params [ 3 ] + params [ 4 ] );
 if ( apply ) params [ DIFS_VARYSCALE_SLOT ] *= params [ 3 ];
 const numeric :: X87Scope extendedScope;
 const float a = float ( ( numeric :: X87Real ) fabs ( local . x ) - fabs ( local . y ) - params [ 9 ] );
 const float c = float ( ( numeric :: X87Real ) fabs ( local . x ) + fabs ( local . y ) - params [ 10 ] );
 numeric :: X87Real d1 = float ( ( - ( numeric :: X87Real ) c + fabs ( a ) - params [ 11 ] ) * params [ 13 ] );
 numeric :: X87Real d2 = float ( ( - ( numeric :: X87Real ) fabs ( a ) - c - params [ 11 ] ) * params [ 13 ] );
 const uint count = uint ( iterations ) - 1u;
 if ( d1 > 0 && count > 0 && count <= 999 ) for ( uint i = 0;
 i < count;
 ++ i ) d1 = numeric :: x87Abs ( params [ 1 ] - numeric :: x87Abs ( params [ 1 ] - d1 ) );
 const bool barrelNegative = metal :: signbit ( params [ 7 ] );
 if ( ! barrelNegative ) d2 = params [ 7 ] - numeric :: x87Abs ( params [ 7 ] - d2 );
 numeric :: X87Real distance;
 if ( ! metal :: signbit ( params [ 0 ] ) ) {
 distance = numeric :: x87Sqrt ( d1 * d1 + d2 * d2 ) - params [ 0 ];
 if ( barrelNegative ) distance -= ( numeric :: X87Real ) local . z * local . z * ( ( numeric :: X87Real ) params [ 7 ] + params [ 1 ] );

}
 else {
 d1 = numeric :: x87Abs ( d1 );
 d2 = numeric :: x87Abs ( d2 );
 distance = ( d2 >= d1 ? d2 : d1 ) + params [ 0 ];
 if ( barrelNegative ) distance += ( numeric :: X87Real ) local . z * ( ( numeric :: X87Real ) params [ 7 ] + params [ 1 ] );

}
 const numeric :: X87Real height = ( numeric :: X87Real ) fabs ( local . z ) - params [ 2 ];
 if ( height >= distance ) distance = height;
 if ( ! apply ) distance /= params [ 3 ];
 de = float ( distance );
 return apply ? local : p;

}

}}
