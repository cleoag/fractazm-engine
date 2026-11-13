// Generated from Ads/_BPolyhedronToSphere.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_7095 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BPOLYHEDRONTOSPHERE = 7095;
 inline numeric :: Extended80 extendedPolygonSector ( numeric :: Extended80 angle , float formula_half , float step , thread FormulaWorkBudget & budget ) {
 while ( angle > numeric :: Extended80 ( formula_half ) ) {
 if ( ! budget . beforeStep ( ) ) return NAN;
 angle -= step;

}
 return angle;

}
 template < > FormulaVec3 execute_formula < FORMULA__BPOLYHEDRONTOSPHERE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using namespace numeric;
 ExtendedScope scope;
 FormulaWorkBudget budget ( params , "_BPolyhedronToSphere" );
 const E a = E ( params [ 2 ] ) / params [ 0 ] , b = E ( params [ 2 ] ) / params [ 1 ];
 const E x = p . x , y = p . y , z = p . z;
 E angle = extendedPolygonSector ( abs ( extendedAtan2 ( sqrt ( z * z + y * y ) , x ) ) , float ( b ) , float ( b + b ) , budget );
 const E factor = E ( 1 ) / extendedSinCos ( angle ) . cos;
 const float ox = float ( factor * x ) , sy = float ( factor * y ) , sz = float ( factor * z );
 angle = extendedPolygonSector ( abs ( extendedAtan2 ( factor * z , E ( sy ) ) ) , float ( a ) , float ( a + a ) , budget );
 const E yz = E ( 1 ) / extendedSinCos ( angle ) . cos;
 return FormulaVec3 ( ox , float ( yz * sy ) , float ( yz * sz ) );

}

}}
