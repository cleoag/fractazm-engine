// Generated from Ads/_recFold.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_3676 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA__RECFOLD = 3676;
 template < > FormulaVec3 execute_formula < FORMULA__RECFOLD > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 const E ax = abs ( E ( p . x ) ) , az = abs ( E ( p . z ) );
 E x = float ( ( az - ax ) - params [ 1 ] );
 E z = float ( ( az + ax ) - params [ 2 ] );
 const E folded = - abs ( x ) , difference = folded - z , sum = z + folded;
 x = float ( ( - sum - params [ 3 ] ) * params [ 5 ] );
 z = float ( ( difference - params [ 3 ] ) * params [ 5 ] );
 const int iterations = int ( params [ 4 ] );
 if ( x >= E ( 0 ) && iterations > 1 && iterations <= 1000 ) {
 const E fold = params [ 0 ];
 for ( int i = 1;
 i < iterations;
 ++ i ) x = abs ( fold - abs ( fold - x ) );

}
 return FormulaVec3 ( float ( x ) , p . y , float ( z ) );

}

}}
