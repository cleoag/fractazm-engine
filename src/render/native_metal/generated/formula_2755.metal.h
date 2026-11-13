// Generated from 4D/Mandel4DBiC.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_2755 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_MANDEL4DBIC = 2755;
 template < > FormulaVec3 execute_formula < FORMULA_MANDEL4DBIC > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const numeric :: X87Scope extendedScope;
 const numeric :: X87Real x = p . x , y = p . y , z = p . z , w = de;
 const numeric :: X87Real xy = x * y , zw = z * w;
 de = float ( ( zw + zw + params [ 4 ] ) + params [ 0 ] );
 return FormulaVec3 ( float ( x * x - y * y + params [ 1 ] ) , float ( xy + xy + params [ 2 ] ) , float ( z * z - w * w + params [ 3 ] ) );

}

}}
