// Generated from Ads/_BPolygonToCircle.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7042 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BPOLYGONTOCIRCLE = 7042;
 inline numeric :: Extended80 extendedPolygonSector ( numeric :: Extended80 angle , float formula_half , float step , thread FormulaWorkBudget & budget ) {
 while ( angle > numeric :: Extended80 ( formula_half ) ) {
 if ( ! budget . beforeStep ( ) ) return NAN;
 angle -= step;

}
 return angle;

}
 template < > FormulaVec3 execute_formula < FORMULA__BPOLYGONTOCIRCLE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using namespace numeric;
 ExtendedScope scope;
 FormulaWorkBudget budget ( params , "_BPolygonToCircle" );
 const E sector = E ( params [ 1 ] ) / params [ 0 ];
 const E angle = extendedPolygonSector ( abs ( extendedAtan2 ( E ( p . z ) , E ( p . y ) ) ) , float ( sector ) , float ( sector + sector ) , budget );
 const E factor = E ( 1 ) / extendedSinCos ( angle ) . cos;
 return FormulaVec3 ( p . x , float ( factor * p . y ) , float ( factor * p . z ) );

}

}}
