// Generated from Ads/_FoldingVar.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7077 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__FOLDINGVAR = 7077;
 template < > FormulaVec3 execute_formula < FORMULA__FOLDINGVAR > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E x = p . x , y = p . y , z = p . z;
 const E fx = ( ( x * params [ 1 ] + y * params [ 2 ] ) + z * params [ 3 ] ) + params [ 0 ];
 const E fy = ( ( x * params [ 5 ] + y * params [ 6 ] ) + z * params [ 7 ] ) + params [ 4 ];
 const E fz = ( x * params [ 9 ] + y * params [ 10 ] ) + ( z * params [ 11 ] + params [ 8 ] );
 return FormulaVec3 ( float ( ( abs ( fx + x ) - abs ( x - fx ) ) - x ) , float ( ( abs ( fy + y ) - abs ( y - fy ) ) - y ) , float ( ( abs ( fz + z ) - abs ( z - fz ) ) - z ) );

}

}}
