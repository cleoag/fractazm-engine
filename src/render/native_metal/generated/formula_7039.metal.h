// Generated from 3Da/_AmazingBox.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7039 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__AMAZINGBOX = 7039;
 template < > FormulaVec3 execute_formula < FORMULA__AMAZINGBOX > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: X87Scope extendedScope;
 const numeric :: X87Real x = numeric :: x87Abs ( ( numeric :: X87Real ) p . x + 1 ) - ( numeric :: x87Abs ( ( numeric :: X87Real ) p . x - 1 ) + p . x );
 const numeric :: X87Real y = numeric :: x87Abs ( ( numeric :: X87Real ) p . y + 1 ) - ( numeric :: x87Abs ( ( numeric :: X87Real ) p . y - 1 ) + p . y );
 const numeric :: X87Real z = numeric :: x87Abs ( ( numeric :: X87Real ) p . z + 1 ) - ( numeric :: x87Abs ( ( numeric :: X87Real ) p . z - 1 ) + p . z );
 const numeric :: X87Real radius = ( z * z + y * y ) + x * x;
 const numeric :: X87Real scale = ! ( radius >= params [ 1 ] ) ? params [ 0 ] : ( radius > 1 ? params [ 2 ] : params [ 2 ] / radius );
 de = float ( ( numeric :: X87Real ) de * scale );
 return FormulaVec3 ( float ( x * scale ) , float ( y * scale ) , float ( z * scale ) );

}

}}
