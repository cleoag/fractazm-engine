// Generated from Ads/_SphereFolding1.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_6030 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__SPHEREFOLDING1 = 6030;
 template < > FormulaVec3 execute_formula < FORMULA__SPHEREFOLDING1 > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 const float radius = params [ FORMULA_ROUT_SLOT ];
 const numeric :: X87Scope extendedScope;
 numeric :: X87Real numerator;
 if ( radius >= params [ 0 ] ) numerator = ( numeric :: X87Real ) params [ 1 ] - radius;
 else if ( ! ( radius >= params [ 2 ] ) ) numerator = ( numeric :: X87Real ) params [ 3 ] - radius;
 else return p;
 const numeric :: X87Real scale = numerator / radius;
 return FormulaVec3 ( float ( ( numeric :: X87Real ) p . x * scale ) , float ( ( numeric :: X87Real ) p . y * scale ) , float ( ( numeric :: X87Real ) p . z * scale ) );

}

}}
