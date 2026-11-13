// Generated from Ads/_BStellahedron_F_Sphere.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_173 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__BSTELLAHEDRON_F_SPHERE = 173;
 inline numeric :: Extended80 extendedPolygonSector ( numeric :: Extended80 angle , float formula_half , float step , thread FormulaWorkBudget & budget ) {
 while ( angle > numeric :: Extended80 ( formula_half ) ) {
 if ( ! budget . beforeStep ( ) ) return NAN;
 angle -= step;

}
 return angle;

}
 template < > FormulaVec3 execute_formula < FORMULA__BSTELLAHEDRON_F_SPHERE > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using E = numeric :: Extended80;
 using namespace numeric;
 ExtendedScope scope;
 FormulaWorkBudget budget ( params , "_BStellahedron_F_Sphere" );
 const E a = E ( params [ 4 ] ) / params [ 0 ] , b = E ( params [ 4 ] ) / params [ 1 ];
 const float half1 = float ( a ) , step1 = float ( a + a ) , half2 = float ( b ) , step2 = float ( b + b );
 struct NativeClosure0 {
 thread decltype ( params ) & params;
 auto operator ( ) ( float formula_half , float offset ) const {
 const E base = E ( formula_half ) * params [ 5 ];
 const float denominator = float ( extendedSinCos ( base - offset ) . cos );
 return float ( extendedSinCos ( base ) . cos / denominator );

}

}
;
 auto ratio = NativeClosure0 {
 params
}
;
 const float ratio1 = ratio ( half1 , params [ 2 ] ) , ratio2 = ratio ( half2 , params [ 3 ] );
 E angle = extendedPolygonSector ( abs ( extendedAtan2 ( E ( p . z ) , E ( p . y ) ) ) , half1 , step1 , budget );
 const E factor = extendedSinCos ( abs ( angle ) - params [ 2 ] ) . cos * ratio1;
 const E sy = factor * p . y , sz = E ( float ( factor ) ) * p . z;
 const float savedY = float ( sy ) , savedZ = float ( sz );
 angle = extendedPolygonSector ( abs ( extendedAtan2 ( sqrt ( sy * sy + sz * sz ) , E ( p . x ) ) ) , half2 , step2 , budget );
 const E factor2 = extendedSinCos ( abs ( angle ) - params [ 3 ] ) . cos * ratio2;
 return FormulaVec3 ( float ( factor2 * p . x ) , float ( factor2 * savedY ) , float ( factor2 * savedZ ) );

}

}}
