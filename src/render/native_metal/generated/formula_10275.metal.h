// Generated from 3D/_IFS_tess.inc; ARM branch, Native FP32.
// Qualification: NOT RUN. Do not count source emission as GPU support.
#pragma once
namespace mb3d_native { namespace formula_10275 {
template<uint ID> FormulaVec3 execute_formula(FormulaVec3, FormulaParams, thread float&);
template<uint ID> FormulaVec3 execute_formula_with_direct_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_with_sphere_map(FormulaVec3, FormulaParams, thread float&, bool);
template<uint ID> FormulaVec3 execute_formula_dIFS(FormulaVec3 p, FormulaParams a, thread float& v, thread float& d) { a[60]=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
template<uint ID> FormulaVec3 execute_formula_dIFS_transform(FormulaVec3 p, FormulaParams a, thread float& v) { a[60]=v; float d=v; auto r=execute_formula<ID>(p,a,d); v=a[60]; return r; }
constant constexpr unsigned int FORMULA_IFSTESS = 10275;
 template < > FormulaVec3 execute_formula < FORMULA_IFSTESS > ( FormulaVec3 p , FormulaParams params , thread float & de ) {
 FormulaWorkBudget work ( params , "_IFS_tess" );
 using namespace numeric;
 using E = Extended80;
 ExtendedScope scope;
 p . x = float ( E ( p . x ) + params [ 0 ] );
 p . y = float ( E ( p . y ) + params [ 1 ] );
 p . z = float ( E ( p . z ) + params [ 2 ] );
 E x = p . y , y = p . z , saved = p . x;
 const E atom = params [ 5 ] , disp = params [ 4 ] , norm = params [ 10 ];
 const auto sc = extendedSinCos ( E ( params [ 6 ] ) * params [ 11 ] );
 const E ca = float ( sc . cos ) , sa = float ( sc . sin );
 const int variation = int ( params [ 8 ] );
 struct NativeClosure0 {
 thread decltype ( p ) & p;
 thread decltype ( params ) & params;
 thread decltype ( saved ) & saved;
 auto operator ( ) ( ) const {
 return FormulaVec3 ( float ( saved * params [ 3 ] - params [ 0 ] ) , float ( E ( p . y ) * params [ 3 ] - params [ 1 ] ) , float ( E ( p . z ) * params [ 3 ] - params [ 2 ] ) );

}

}
;
 const auto rescale = NativeClosure0 {
 p , params , saved
}
;
 if ( ! ( atom > abs ( saved ) ) || ( ( atom > abs ( x ) ) && ( atom > abs ( y ) ) ) ) return rescale ( );
 unsigned int depth = 0;
 for (;
;
 ) {
 if ( ! work . beforeStep ( ) ) return NAN;
 if ( variation >= 4 ) {
 if ( ( atom > abs ( x ) ) && ( atom > abs ( y ) ) ) {
 x = extendedNaN ( );
 y = - y;

}
 else {
 E k = abs ( x );
 if ( k != E ( 0 ) ) k = float ( k / x );
 const E xt = float ( - ( k * disp ) + x ) , yt = float ( - ( disp * params [ 9 ] ) + y );
 if ( E ( 0 ) > k ) {
 x = float ( xt * ca + yt * sa );
 y = float ( - ( xt * sa ) + yt * ca );

}
 else {
 x = float ( xt * ca + ( - ( yt * sa ) ) );
 y = float ( xt * sa + yt * ca );

}

}

}
 else {
 bool negative = false;
 if ( y > x ) {
 y = float ( y + ( - ( disp * norm ) ) );
 negative = true;

}
 else x = float ( x + ( - ( disp * norm ) ) );
 const E xt = float ( - ( x * sa ) + ( - ( y * ca ) ) ) , yt = float ( x * ca + ( - ( y * sa ) ) );
 x = xt;
 y = yt;
 if ( negative ) {
 if ( variation == 1 ) {
 x = - x;
 y = - y;

}
 else if ( variation == 2 ) {
 const E t = x;
 x = y;
 y = t;

}
 else if ( variation == 3 ) {
 const E t = x;
 x = - y;
 y = - t;

}

}

}
 x = float ( x / norm );
 y = float ( y / norm );
 if ( ( atom > abs ( x ) ) && ( atom > abs ( y ) ) ) {
 p . y = float ( x );
 p . z = float ( y );
 saved = float ( saved / norm );
 return rescale ( );

}
 const auto constantDepth = uint ( params . bits ( 12 ) >> 32 );
 if ( depth >= constantDepth || depth >= static_cast < unsigned int > ( int ( params [ 7 ] ) ) ) break;
 ++ depth;
 saved = float ( saved / norm );

}
 return FormulaVec3 ( float ( E ( params [ 12 ] ) * p . x ) , float ( E ( params [ 12 ] ) * p . y ) , float ( E ( params [ 12 ] ) * p . z ) );

}

}}
