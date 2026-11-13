// Generated from dIFSa/tilingbrickIFS.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10246 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_TILINGBRICKIFS = 10246;
 template < > FormulaVec3 execute_formula < FORMULA_TILINGBRICKIFS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: X87Scope extendedScope;
 const numeric :: X87Real x = ( numeric :: X87Real ) p . x + params [ 0 ];
 const numeric :: X87Real y = ( numeric :: X87Real ) p . y + params [ 1 ];
 const numeric :: X87Real z = ( numeric :: X87Real ) p . z + params [ 2 ];
 const float rx = float ( x * params [ 3 ] + y * params [ 4 ] + z * params [ 5 ] );
 const float ry = float ( x * params [ 6 ] + y * params [ 7 ] + z * params [ 8 ] );
 const float rz = float ( x * params [ 9 ] + y * params [ 10 ] + z * params [ 11 ] );
 const numeric :: X87Real rounded = numeric :: x87Round ( ( numeric :: X87Real ) ry / params [ 13 ] );
 const int iy = rounded >= - numeric :: X87Real ( 2147483648.f ) && rounded <= numeric :: X87Real ( 2147483647.f ) ? int ( rounded ) : INT32_MIN;
 const numeric :: X87Real fy = ( numeric :: X87Real ) ry - ( numeric :: X87Real ) iy * params [ 13 ];
 const numeric :: X87Real shifted = ( numeric :: X87Real ) rx + ( ( iy & 1 ) ? numeric :: X87Real ( 0.f ) : ( numeric :: X87Real ) params [ 14 ] );
 const numeric :: X87Real fx = shifted - numeric :: x87Round ( shifted / params [ 12 ] ) * params [ 12 ];
 return FormulaVec3 ( float ( fx ) , float ( fy ) , rz );

}

}}
