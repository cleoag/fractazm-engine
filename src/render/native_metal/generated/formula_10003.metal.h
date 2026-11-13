// Generated from Ads/_JuliaSets.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10003 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__JULIASETS = 10003;
 template < > FormulaVec3 execute_formula < FORMULA__JULIASETS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 float state [ ] = {
 params [ 2 ] , params [ 3 ] , params [ 4 ] , 0.f , 0.f , 0.f
}
;
 const numeric :: X87Scope extendedScope;
 const numeric :: X87Real inverse = numeric :: X87Real ( 1.f ) / params [ 0 ];
 for ( int i = 0;
 i < 3;
 ++ i ) {
 const numeric :: X87Real scaled = state [ i ] * inverse , tile = numeric :: x87Round ( scaled );
 state [ i ] = float ( ( scaled - tile ) * params [ 0 ] );
 state [ i + 3 ] = float ( tile * params [ 1 ] );

}
 params [ WRITE_J1_SLOT ] = state [ 3 ];
 params [ WRITE_J2_SLOT ] = state [ 4 ];
 params [ WRITE_J3_SLOT ] = state [ 5 ];
 return FormulaVec3 ( state [ 0 ] , state [ 1 ] , state [ 2 ] );

}

}}
